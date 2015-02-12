// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "base/timer/timer.h"
#include "base/i18n/icu_string_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/bind.h"
#include "net/base/net_errors.h"
#include "sippet/examples/program_main/program_main.h"

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "webrtc/base/ssladapter.h"

#include "jingle/glue/thread_wrapper.h"

#include "re2/re2.h"

#if defined(OSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#endif  // defined(OS_MACOSX)

static void PrintUsage() {
  std::cout << "sippet_examples_login"
            << " --username=user"
            << " --password=pass"
            << " --dial=phone-number"
            << " \\" << std::endl
            << "    [--tcp|--udp|--tls|--ws|--wss]\n";
}

struct Settings {
  sippet::NetworkLayer *network_layer;
  sippet::ua::UserAgent *user_agent;
  std::string username;
  std::string password;
  std::string registrar_uri;
  std::string server;
  std::string phone_number;
};

class MediaConstraints
  : public webrtc::MediaConstraintsInterface {
  using webrtc::MediaConstraintsInterface::Constraint;
  using webrtc::MediaConstraintsInterface::Constraints;

 public:
  void AddMandatory(const std::string& key, bool value) {
    using namespace webrtc;
    mandatory_.push_back(Constraint(key,
        value ?
          MediaConstraintsInterface::kValueTrue :
          MediaConstraintsInterface::kValueFalse));
  }
  void AddOptional(const std::string& key, bool value) {
    using namespace webrtc;
    optional_.push_back(Constraint(key,
        value ?
          MediaConstraintsInterface::kValueTrue :
          MediaConstraintsInterface::kValueFalse));
  }

  const Constraints& GetMandatory() const override {
    return mandatory_;
  }
  const Constraints& GetOptional() const override {
    return optional_;
  }

 private:
  Constraints mandatory_;
  Constraints optional_;
};

class ProxySetSessionDescriptionObserver
  : public webrtc::SetSessionDescriptionObserver {
public:
  static ProxySetSessionDescriptionObserver* Create(
    const base::Callback<void()> &on_success = base::Callback<void()>(),
    const base::Callback<void(const std::string&)> &on_failure =
        base::Callback<void(const std::string&)>()) {
    return
        new rtc::RefCountedObject<ProxySetSessionDescriptionObserver>(
          on_success, on_failure);
  }
  void OnSuccess() override {
    on_success_.Run();
  }
  void OnFailure(const std::string& error) override {
    on_failure_.Run(error);
  }

protected:
  ProxySetSessionDescriptionObserver(
    const base::Callback<void()> &on_success,
    const base::Callback<void(const std::string&)> &on_failure)
    : on_success_(on_success), on_failure_(on_failure) {
  }
  ~ProxySetSessionDescriptionObserver() override {}

  base::Callback<void()> on_success_;
  base::Callback<void(const std::string&)> on_failure_;
};

class UserAgentHandler
  : public sippet::ua::UserAgent::Delegate,
    public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver {
 public:
  UserAgentHandler(const Settings& settings)
    : network_layer_(settings.network_layer),
      user_agent_(settings.user_agent),
      username_(settings.username),
      registrar_uri_(settings.registrar_uri),
      server_(settings.server),
      phone_number_(settings.phone_number),
      next_state_(STATE_NONE),
      message_loop_(base::MessageLoop::current()),
      on_sdp_success_(
          base::Bind(&UserAgentHandler::OnSessionDescriptionSuccess,
          base::Unretained(this))),
      on_sdp_failure_(
          base::Bind(&UserAgentHandler::OnSessionDescriptionFailure,
          base::Unretained(this))) {
  }

  ~UserAgentHandler() override {}

  bool InitializePeerConnection() {
    ASSERT(peer_connection_factory_.get() == nullptr);
    ASSERT(peer_connection_.get() == nullptr);

    // To allow sending to the signaling/worker threads.
    jingle_glue::JingleThreadWrapper::EnsureForCurrentMessageLoop();
    jingle_glue::JingleThreadWrapper::current()->set_send_allowed(true);

    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();
    if (!peer_connection_factory_.get()) {
      std::cout << "Error: Failed to initialize PeerConnectionFactory\n";
      DeletePeerConnection();
      return false;
    }

    webrtc::PeerConnectionFactoryInterface::Options options;
    options.disable_encryption = true;
    options.disable_sctp_data_channels = true;
    peer_connection_factory_->SetOptions(options);

    webrtc::PeerConnectionInterface::IceServers servers;
    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = "stun:stun.l.google.com:19302";
    //servers.push_back(server);

    peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers,
      &constraints_, nullptr, nullptr, this);
    if (!peer_connection_.get()) {
      std::cout << "Error: CreatePeerConnection failed\n";
      DeletePeerConnection();
      return false;
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        peer_connection_factory_->CreateAudioTrack(
            "audio", peer_connection_factory_->CreateAudioSource(nullptr)));

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
      peer_connection_factory_->CreateLocalMediaStream("stream");

    stream->AddTrack(audio_track);
    if (!peer_connection_->AddStream(stream)) {
      LOG(ERROR) << "Adding stream to PeerConnection failed";
    }
    active_streams_.insert(std::make_pair(stream->label(), stream));

    return peer_connection_.get() != nullptr;
  }

  void DeletePeerConnection() {
    peer_connection_ = nullptr;
    active_streams_.clear();
    peer_connection_factory_ = nullptr;
  }

  void Start() {
    next_state_ = STATE_REGISTER;
    OnIOComplete(net::OK);
  }

 protected:
  //
  // UserAgent::Delegate implementation
  //
  void OnChannelConnected(const sippet::EndPoint &destination,
                          int err) override {
    std::cout << "Channel " << destination.ToString()
              << " connected, status = " << err << "\n";
  }

  void OnChannelClosed(const sippet::EndPoint &destination) override {
    std::cout << "Channel " << destination.ToString()
              << " closed.\n";

    base::MessageLoop::current()->Quit();
  }

  void OnIncomingRequest(
      const scoped_refptr<sippet::Request> &incoming_request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "-- Incoming request "
              << incoming_request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";
  }

  void OnIncomingResponse(
      const scoped_refptr<sippet::Response> &incoming_response,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "-- Incoming response "
              << incoming_response->response_code()
              << " "
              << incoming_response->reason_phrase();
    if (incoming_response->refer_to())
      std::cout << " for " << incoming_response->refer_to()->method().str();
    std::cout << "\n";

    if (dialog) {
      std::cout << "-- Using dialog " << dialog->id() << "\n";
      sippet::Message::iterator i = incoming_response->find_first<sippet::Cseq>();
      if (incoming_response->end() != i
          && sippet::Method::INVITE == sippet::dyn_cast<sippet::Cseq>(i)->method()
          && incoming_response->response_code() / 100 == 2) {
        if ((dialog_ = dialog)) { // save dialog for later
          webrtc::SessionDescriptionInterface *desc =
              webrtc::CreateSessionDescription(
                  webrtc::SessionDescriptionInterface::kAnswer,
                  incoming_response->content());
          if (!desc) {
            std::cout << "-- Error in response SDP\n";
          } else if (peer_connection_) {
            peer_connection_->SetRemoteDescription(
                ProxySetSessionDescriptionObserver::Create(on_sdp_success_,
                    on_sdp_failure_), desc);
          }
          if (nullptr != incoming_response->refer_to()
              && last_invite_ != incoming_response->refer_to())
            last_invite_ = incoming_response->refer_to();
          scoped_refptr<sippet::Request> ack = dialog_->CreateAck(last_invite_);
          user_agent_->Send(ack, base::Bind(&RequestSent,
            sippet::Method::ACK));
        }
      }
    }
    if (STATE_REGISTER_COMPLETE == next_state_
        || STATE_INVITE_COMPLETE == next_state_
        || STATE_BYE_COMPLETE == next_state_) {
      switch (incoming_response->response_code() / 100) {
      case 1:
        break;
      case 2:
        OnIOComplete(net::OK);
        break;
      default:
        OnIOComplete(net::ERR_ABORTED);
        break;
      }
    }
  }

  void OnTimedOut(
      const scoped_refptr<sippet::Request> &request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Timed out sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  void OnTransportError(
      const scoped_refptr<sippet::Request> &request, int error,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Transport error sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  //
  // PeerConnectionObserver implementation.
  //
  void OnStateChange(
      webrtc::PeerConnectionObserver::StateType state_changed) override {
    std::cout << "-- PeerConnection::OnStateChange : " << state_changed << "\n";
  }

  void OnAddStream(webrtc::MediaStreamInterface* stream) override {
    std::cout << "-- PeerConnection::OnAddStream " << stream->label();
  }

  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override {
    std::cout << "-- PeerConnection::OnRemoveStream " << stream->label();
  }

  void OnDataChannel(webrtc::DataChannelInterface* data_channel) override {
    std::cout << "-- PeerConnection::OnDataChannel " << data_channel->label();
  }

  void OnRenegotiationNeeded() override {}
  
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
    std::cout << "-- PeerConnection::OnIceCandidate " << candidate->sdp_mline_index() << "\n";

    std::string sdp;
    if (!candidate->ToString(&sdp)) {
      LOG(ERROR) << "Failed to serialize candidate";
      return;
    }

    std::cout << "-- SDP:\n" << sdp;
  }

  void OnIceComplete() override {
    std::cout << "-- PeerConnection::OnIceComplete\n";

    const webrtc::SessionDescriptionInterface* desc =
      peer_connection_->local_description();
    desc->ToString(&offer_);

    static std::pair<const char*, const char*> replacements[] {
      std::make_pair("a=group:[^\r\n]*\r?\n", ""),
        std::make_pair("a=msid-[^\r\n]*\r?\n", ""),
        std::make_pair("RTP/AVPF", "RTP/AVP"),
        std::make_pair("a=ice-[^\r\n]*\r?\n", ""),
        std::make_pair("a=mid:[^\r\n]*\r?\n", ""),
        std::make_pair("a=rtcp-mux[^\r\n]*\r?\n", ""),
        std::make_pair("a=extmap:[^\r\n]*\r?\n", ""),
        std::make_pair("a=ssrc:[^\r\n]*\r?\n", ""),
        std::make_pair("a=candidate:[^\r\n]*\r?\n", ""),
    };

    for (int i = 0; i < arraysize(replacements); ++i) {
      std::pair<const char*, const char*> &elem = replacements[i];
      RE2::GlobalReplace(&offer_, elem.first, elem.second);
    }

    message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&UserAgentHandler::OnIOComplete,
      base::Unretained(this), net::OK));
  }

  //
  // SetSessionDescriptionObserver callbacks.
  //
  void OnSessionDescriptionSuccess() {
    std::cout << "SetSessionDescriptionObserver::OnSuccess\n";
  }
  void OnSessionDescriptionFailure(const std::string& error) {
    std::cout << "SetSessionDescriptionObserver::OnFailure\n";

    if (next_state_ == STATE_CREATE_OFFER_COMPLETE) {
      message_loop_->PostTask(
        FROM_HERE,
        base::Bind(&UserAgentHandler::OnIOComplete,
        base::Unretained(this), net::ERR_ABORTED));
    }
  }

  //
  // CreateSessionDescriptionObserver implementation.
  //
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    std::cout << "CreateSessionDescriptionObserver::OnSuccess\n";
    peer_connection_->SetLocalDescription(
        ProxySetSessionDescriptionObserver::Create(on_sdp_success_,
            on_sdp_failure_), desc);
  }
  
  void OnFailure(const std::string& error) override {
    std::cout << "CreateSessionDescriptionObserver::OnFailure\n";
  }

 private:
  sippet::NetworkLayer *network_layer_;
  sippet::ua::UserAgent *user_agent_;
  base::MessageLoop *message_loop_;
  std::string username_;
  std::string registrar_uri_;
  std::string server_;
  std::string phone_number_;
  
  std::string offer_;
  scoped_refptr<sippet::Dialog> dialog_;
  scoped_refptr<sippet::Request> last_invite_;
  base::OneShotTimer<UserAgentHandler> call_timeout_;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;
  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;
  
  MediaConstraints constraints_;
  base::Callback<void()> on_sdp_success_;
  base::Callback<void(const std::string&)> on_sdp_failure_;

  enum State {
    STATE_REGISTER,
    STATE_REGISTER_COMPLETE,
    STATE_CREATE_OFFER,
    STATE_CREATE_OFFER_COMPLETE,
    STATE_INVITE,
    STATE_INVITE_COMPLETE,
    STATE_CALL_TIMER,
    STATE_CALL_TIMER_COMPLETE,
    STATE_BYE,
    STATE_BYE_COMPLETE,
    STATE_UNREGISTER,
    STATE_UNREGISTER_COMPLETE,
    STATE_NONE
  };
  
  State next_state_;
  
  static void RequestSent(const sippet::Method& method, int error) {
    std::cout << ">> " << method.str() << " completed";
    if (error != net::OK) {
      std::cout << ", error = " << error
                << " (" << net::ErrorToString(error) << ")";
    }
    std::cout << "\n";
  }

  void OnIOComplete(int result) {
    DCHECK_NE(STATE_NONE, next_state_);
    int rv = DoLoop(result);
    if (rv != net::ERR_IO_PENDING) {
      DeletePeerConnection();
      base::MessageLoop::current()->Quit();
    }
  }

  int DoLoop(int last_io_result) {
    DCHECK_NE(next_state_, STATE_NONE);
    int rv = last_io_result;
    do {
      State state = next_state_;
      next_state_ = STATE_NONE;
      switch (state) {
      case STATE_REGISTER:
        DCHECK_EQ(net::OK, rv);
        rv = DoRegister();
        break;
      case STATE_REGISTER_COMPLETE:
        rv = DoRegisterComplete(rv);
        break;
      case STATE_CREATE_OFFER:
        DCHECK_EQ(net::OK, rv);
        rv = DoCreateOffer();
        break;
      case STATE_CREATE_OFFER_COMPLETE:
        rv = DoCreateOfferComplete(rv);
        break;
      case STATE_INVITE:
        DCHECK_EQ(net::OK, rv);
        rv = DoInvite();
        break;
      case STATE_INVITE_COMPLETE:
        rv = DoInviteComplete(rv);
        break;
      case STATE_CALL_TIMER:
        DCHECK_EQ(net::OK, rv);
        rv = DoCallTimer();
        break;
      case STATE_CALL_TIMER_COMPLETE:
        DCHECK_EQ(net::OK, rv);
        rv = DoCallTimerComplete();
        break;
      case STATE_BYE:
        DCHECK_EQ(net::OK, rv);
        rv = DoBye();
        break;
      case STATE_BYE_COMPLETE:
        rv = DoByeComplete(rv);
        break;
      case STATE_UNREGISTER:
        DCHECK_EQ(net::OK, rv);
        rv = DoUnregister();
        break;
      case STATE_UNREGISTER_COMPLETE:
        rv = DoUnregisterComplete(rv);
        break;
      default:
        NOTREACHED() << "bad state";
        rv = net::ERR_UNEXPECTED;
        break;
      }
    } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);
    return rv;
  }

  int DoRegister() {
    next_state_ = STATE_REGISTER_COMPLETE;

    std::string from("sip:" + username_ + "@" + server_);
    std::string to("sip:" + username_ + "@" + server_);

    scoped_refptr<sippet::Request> request =
      user_agent_->CreateRequest(
      sippet::Method::REGISTER,
      GURL(registrar_uri_),
      GURL(from),
      GURL(to));

    int status = user_agent_->Send(request, base::Bind(&RequestSent,
        request->method()));
    if (net::OK != status && net::ERR_IO_PENDING != status)
      return status;
    return net::ERR_IO_PENDING; // Wait for SIP response
  }

  int DoRegisterComplete(int result) {
    if (net::OK != result)
      return result;

    next_state_ = STATE_CREATE_OFFER;
    return net::OK;
  }

  int DoCreateOffer() {
    next_state_ = STATE_CREATE_OFFER_COMPLETE;
    peer_connection_->CreateOffer(this, &constraints_);
    return net::ERR_IO_PENDING;
  }

  int DoCreateOfferComplete(int result) {
    if (result == net::OK) {
      next_state_ = STATE_INVITE;
    } else {
      next_state_ = STATE_NONE;
    }
    return net::OK;
  }

  int DoInvite() {
    next_state_ = STATE_INVITE_COMPLETE;

    std::string request_uri("sip:" + phone_number_ + "@" + server_);
    std::string from("sip:" + username_ + "@" + server_);
    std::string to("sip:" + phone_number_ + "@" + server_);

    last_invite_ =
      user_agent_->CreateRequest(
      sippet::Method::INVITE,
      GURL(request_uri),
      GURL(from),
      GURL(to));

    scoped_ptr<sippet::ContentType> content_type(
      new sippet::ContentType("application", "sdp"));
    last_invite_->push_back(content_type.Pass());
    last_invite_->set_content(offer_);

    int status = user_agent_->Send(last_invite_, base::Bind(&RequestSent,
      last_invite_->method()));
    if (net::OK != status && net::ERR_IO_PENDING != status)
      return status;
    return net::ERR_IO_PENDING;
  }

  int DoInviteComplete(int result) {
    if (net::OK != result)
      return result;

    next_state_ = STATE_CALL_TIMER;
    return net::OK;
  }

  int DoCallTimer() {
    next_state_ = STATE_CALL_TIMER_COMPLETE;
    call_timeout_.Start(FROM_HERE,
        base::TimeDelta::FromSeconds(3600),
        base::Bind(&UserAgentHandler::OnIOComplete,
            base::Unretained(this), net::OK));
    return net::ERR_IO_PENDING;
  }

  int DoCallTimerComplete() {
    next_state_ = STATE_BYE;
    return net::OK;
  }

  int DoBye() {
    ASSERT(dialog_ != nullptr);
    next_state_ = STATE_BYE_COMPLETE;

    scoped_refptr<sippet::Request> bye(
      dialog_->CreateRequest(sippet::Method::BYE));

    int status = user_agent_->Send(bye, base::Bind(&RequestSent,
        bye->method()));
    if (net::OK != status && net::ERR_IO_PENDING != status)
      return status;
    return net::ERR_IO_PENDING;
  }

  int DoByeComplete(int result) {
    next_state_ = STATE_UNREGISTER;
    return net::OK;
  }

  int DoUnregister() {
    next_state_ = STATE_UNREGISTER_COMPLETE;

    std::string from("sip:" + username_ + "@" + server_);
    std::string to("sip:" + username_ + "@" + server_);
    scoped_refptr<sippet::Request> request =
      user_agent_->CreateRequest(
      sippet::Method::REGISTER,
      GURL(registrar_uri_),
      GURL(from),
      GURL(to));

    // Modifies the Contact header to include an expires=0 at the end
    // of existing parameters
    sippet::Contact* contact = request->get<sippet::Contact>();
    contact->front().set_expires(0);

    int status = user_agent_->Send(request, base::Bind(&RequestSent,
      request->method()));
    if (net::OK != status && net::ERR_IO_PENDING != status)
      return status;
    return net::ERR_IO_PENDING; // Wait for SIP response
  }

  int DoUnregisterComplete(int result) {
    next_state_ = STATE_NONE;
    return net::OK;
  }
};

int main(int argc, char **argv) {
#if defined(OSX)
  // Needed so we don't leak objects when threads are created.
  base::mac::ScopedNSAutoreleasePool pool;
#endif
  rtc::InitializeSSL();

  ProgramMain program_main(argc, argv);
  base::CommandLine* command_line = program_main.command_line();

  if (command_line->GetSwitches().empty() ||
      command_line->HasSwitch("help")) {
    PrintUsage();
    return -1;
  }

  Settings settings;

  if (command_line->HasSwitch("username")) {
    settings.username = command_line->GetSwitchValueASCII("username");
  } else {
    PrintUsage();
    return -1;
  }

  if (command_line->HasSwitch("password")) {
    settings.password = command_line->GetSwitchValueASCII("password");
  } else {
    PrintUsage();
    return -1;
  }

  if (command_line->HasSwitch("dial")) {
    settings.phone_number = command_line->GetSwitchValueASCII("dial");
  }
  else {
    PrintUsage();
    return -1;
  }

  settings.server = "localhost";
  if (command_line->HasSwitch("server")) {
    settings.server = command_line->GetSwitchValueASCII("server");
  }

  struct {
    const char *cmd_switch_;
    const char *registrar_uri_;
  } args[] = {
    { "udp", "sip:%s" },
    { "tcp", "sip:%s;transport=tcp" },
    { "tls", "sips:%s" },
    { "ws", "sip:%s;transport=ws" },
    { "wss", "sips:%s;transport=ws" },
  };

  for (int i = 0; i < arraysize(args); i++) {
    if (command_line->HasSwitch(args[i].cmd_switch_)) {
      settings.registrar_uri = base::StringPrintf(args[i].registrar_uri_,
          settings.server.c_str());
      break;
    }
  }
  if (settings.registrar_uri.empty()) {
    settings.registrar_uri = base::StringPrintf(args[0].registrar_uri_,
      settings.server.c_str()); // Defaults to UDP
  }

  program_main.set_username(base::ASCIIToUTF16(settings.username));
  program_main.set_password(base::ASCIIToUTF16(settings.password));
  if (!program_main.Init()) {
    return -1;
  }

  settings.network_layer = program_main.network_layer();
  settings.user_agent = program_main.user_agent();

  rtc::scoped_refptr<UserAgentHandler> handler(
      new rtc::RefCountedObject<UserAgentHandler>(settings));
  program_main.AppendHandler(handler.get());

  if (handler->InitializePeerConnection())
    handler->Start();

  program_main.Run();
  return 0;
}
