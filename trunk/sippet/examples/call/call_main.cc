// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "base/timer/timer.h"
#include "base/i18n/icu_string_conversions.h"
#include "base/strings/string_util.h"
#include "net/base/net_errors.h"
#include "sippet/examples/program_main/program_main.h"

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/media/devices/devicemanager.h"
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

class DummySetSessionDescriptionObserver
  : public webrtc::SetSessionDescriptionObserver {
public:
  static DummySetSessionDescriptionObserver* Create() {
    return
      new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() {
    std::cout << "SetSessionDescriptionObserver::OnSuccess\n";
  }
  virtual void OnFailure(const std::string& error) {
    std::cout << "SetSessionDescriptionObserver::OnFailure\n";
  }

protected:
  DummySetSessionDescriptionObserver() {}
  ~DummySetSessionDescriptionObserver() {}
};

struct Settings {
  sippet::NetworkLayer *network_layer;
  sippet::ua::UserAgent *user_agent;
  base::string16 username;
  base::string16 password;
  base::string16 registrar_uri;
  base::string16 server;
  base::string16 phone_number;
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
      message_loop_(base::MessageLoop::current()) {
  }

  virtual ~UserAgentHandler() {}

  bool InitializePeerConnection() {
    ASSERT(peer_connection_factory_.get() == NULL);
    ASSERT(peer_connection_.get() == NULL);

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
    peer_connection_factory_->SetOptions(options);

    webrtc::PeerConnectionInterface::IceServers servers;
    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = "stun:stun.l.google.com:19302";
    //servers.push_back(server);
    peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers,
      NULL, NULL, NULL, this);
    if (!peer_connection_.get()) {
      std::cout << "Error: CreatePeerConnection failed\n";
      DeletePeerConnection();
      return false;
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        peer_connection_factory_->CreateAudioTrack(
            "audio", peer_connection_factory_->CreateAudioSource(NULL)));

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
      peer_connection_factory_->CreateLocalMediaStream("stream");

    stream->AddTrack(audio_track);
    if (!peer_connection_->AddStream(stream)) {
      LOG(ERROR) << "Adding stream to PeerConnection failed";
    }
    active_streams_.insert(std::make_pair(stream->label(), stream));

    return peer_connection_.get() != NULL;
  }

  void DeletePeerConnection() {
    peer_connection_ = NULL;
    active_streams_.clear();
    peer_connection_factory_ = NULL;
  }

  void Start() {
    next_state_ = STATE_REGISTER;
    OnIOComplete(net::OK);
  }

 protected:
  //
  // UserAgent::Delegate implementation
  //
  virtual void OnChannelConnected(const sippet::EndPoint &destination,
                                  int err) override {
    std::cout << "Channel " << destination.ToString()
              << " connected, status = " << err << "\n";
  }

  virtual void OnChannelClosed(const sippet::EndPoint &destination) override {
    std::cout << "Channel " << destination.ToString()
              << " closed.\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnIncomingRequest(
      const scoped_refptr<sippet::Request> &incoming_request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "-- Incoming request "
              << incoming_request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";
  }

  virtual void OnIncomingResponse(
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
              DummySetSessionDescriptionObserver::Create(), desc);
          }
          if (last_invite_ != incoming_response->refer_to())
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

  virtual void OnTimedOut(
      const scoped_refptr<sippet::Request> &request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Timed out sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnTransportError(
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
  virtual void OnError() {
    std::cout << "-- PeerConnection::OnError\n";
    message_loop_->Quit();
  }

  virtual void OnStateChange(
      webrtc::PeerConnectionObserver::StateType state_changed) {
    std::cout << "-- PeerConnection::OnStateChange : " << state_changed << "\n";
  }

  virtual void OnAddStream(webrtc::MediaStreamInterface* stream) {
    std::cout << "-- PeerConnection::OnAddStream " << stream->label();
  }

  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream) {
    std::cout << "-- PeerConnection::OnRemoveStream " << stream->label();
  }

  virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel) {
    std::cout << "-- PeerConnection::OnDataChannel " << data_channel->label();
  }

  virtual void OnRenegotiationNeeded() {}
  
  virtual void OnIceChange() {}
  
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    std::cout << "-- PeerConnection::OnIceCandidate " << candidate->sdp_mline_index() << "\n";

    std::string sdp;
    if (!candidate->ToString(&sdp)) {
      LOG(ERROR) << "Failed to serialize candidate";
      return;
    }

    std::cout << "-- SDP:\n" << sdp;
  }

  //
  // CreateSessionDescriptionObserver implementation.
  //
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) {
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
    };

    for (int i = 0; i < arraysize(replacements); ++i) {
      std::pair<const char*, const char*> &elem = replacements[i];
      RE2::GlobalReplace(&offer_, elem.first, elem.second);
    }

    peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
    message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&UserAgentHandler::OnIOComplete,
          base::Unretained(this), net::OK));
  }
  
  virtual void OnFailure(const std::string& error) {
    message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&UserAgentHandler::OnIOComplete,
          base::Unretained(this), net::ERR_ABORTED));
  }

 private:
  sippet::NetworkLayer *network_layer_;
  sippet::ua::UserAgent *user_agent_;
  base::MessageLoop *message_loop_;
  base::string16 username_;
  base::string16 registrar_uri_;
  base::string16 server_;
  base::string16 phone_number_;
  
  std::string offer_;
  scoped_refptr<sippet::Dialog> dialog_;
  scoped_refptr<sippet::Request> last_invite_;
  base::OneShotTimer<UserAgentHandler> call_timeout_;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;
  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;

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

    base::string16 from(L"sip:" + username_ + L"@" + server_);
    base::string16 to(L"sip:" + username_ + L"@" + server_);

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
    peer_connection_->CreateOffer(this, NULL);
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

    base::string16 request_uri(L"sip:" + phone_number_ + L"@" + server_);
    base::string16 from(L"sip:" + username_ + L"@" + server_);
    base::string16 to(L"sip:" + phone_number_ + L"@" + server_);

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
        base::TimeDelta::FromSeconds(5),
        base::Bind(&UserAgentHandler::OnIOComplete,
            base::Unretained(this), net::OK));
    return net::ERR_IO_PENDING;
  }

  int DoCallTimerComplete() {
    next_state_ = STATE_BYE;
    return net::OK;
  }

  int DoBye() {
    ASSERT(dialog_ != NULL);
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

    base::string16 from(L"sip:" + username_ + L"@" + server_);
    base::string16 to(L"sip:" + username_ + L"@" + server_);
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
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("username"),
        NULL, base::OnStringConversionError::FAIL, &settings.username);
  } else {
    PrintUsage();
    return -1;
  }

  if (command_line->HasSwitch("password")) {
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("password"),
        NULL, base::OnStringConversionError::FAIL, &settings.password);
  } else {
    PrintUsage();
    return -1;
  }

  if (command_line->HasSwitch("dial")) {
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("dial"),
      NULL, base::OnStringConversionError::FAIL, &settings.phone_number);
  }
  else {
    PrintUsage();
    return -1;
  }

  settings.server = L"localhost";
  if (command_line->HasSwitch("server")) {
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("server"),
      NULL, base::OnStringConversionError::FAIL, &settings.server);
  }

  struct {
    const char *cmd_switch_;
    const base::char16 *registrar_uri_;
  } args[] = {
    { "udp", L"sip:%ls" },
    { "tcp", L"sip:%ls;transport=tcp" },
    { "tls", L"sips:%ls" },
    { "ws", L"sip:%ls;transport=ws" },
    { "wss", L"sips:%ls;transport=ws" },
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

  program_main.set_username(settings.username);
  program_main.set_password(settings.password);
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
