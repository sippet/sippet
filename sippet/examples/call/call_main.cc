// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "net/base/net_errors.h"
#include "base/i18n/icu_string_conversions.h"
#include "sippet/examples/program_main/program_main.h"

#include "talk/base/ssladapter.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/media/devices/devicemanager.h"

#if defined(OS_MACOSX)
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
      new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
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
  string16 username;
  string16 password;
  string16 registrar_uri;
  string16 server;
  string16 phone_number;
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

    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();

    if (!peer_connection_factory_.get()) {
      std::cout << "Error: Failed to initialize PeerConnectionFactory\n";
      DeletePeerConnection();
      return false;
    }

    webrtc::PeerConnectionInterface::IceServers servers;
    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = GetPeerConnectionString();
    //servers.push_back(server);
    peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers,
      NULL,
      NULL,
      this);
    if (!peer_connection_.get()) {
      std::cout << "Error: CreatePeerConnection failed\n";
      DeletePeerConnection();
      return false;
    }

    talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      peer_connection_factory_->CreateAudioTrack(
      "audio", peer_connection_factory_->CreateAudioSource(NULL)));

    talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream =
      peer_connection_factory_->CreateLocalMediaStream("stream");

    stream->AddTrack(audio_track);
    if (!peer_connection_->AddStream(stream, NULL)) {
      LOG(ERROR) << "Adding stream to PeerConnection failed";
    }
    typedef std::pair<std::string,
      talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
      MediaStreamPair;
    active_streams_.insert(MediaStreamPair(stream->label(), stream));

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
                                  int err) OVERRIDE {
    std::cout << "Channel " << destination.ToString()
              << " connected, status = " << err << "\n";
  }

  virtual void OnChannelClosed(const sippet::EndPoint &destination) OVERRIDE {
    std::cout << "Channel " << destination.ToString()
              << " closed.\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnIncomingRequest(
      const scoped_refptr<sippet::Request> &incoming_request,
      const scoped_refptr<sippet::Dialog> &dialog) OVERRIDE {
    std::cout << "Incoming request "
              << incoming_request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";
  }

  virtual void OnIncomingResponse(
      const scoped_refptr<sippet::Response> &incoming_response,
      const scoped_refptr<sippet::Dialog> &dialog) OVERRIDE {
    std::cout << "Incoming response "
              << incoming_response->response_code()
              << " "
              << incoming_response->reason_phrase()
              << "\n";
    if (dialog) {
      std::cout << "-- Using dialog " << dialog->id() << "\n";
      if (sippet::Method::INVITE == incoming_response->refer_to()->method()
          && incoming_response->response_code()/100 == 2) {
        dialog_ = dialog; // save dialog for later
        scoped_refptr<sippet::Request> ack = dialog_->CreateAck(last_invite_);
        user_agent_->Send(ack, base::Bind(&RequestSent,
          sippet::Method::ACK));
      }
    }
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

  virtual void OnTimedOut(
      const scoped_refptr<sippet::Request> &request,
      const scoped_refptr<sippet::Dialog> &dialog) OVERRIDE {
    std::cout << "Timed out sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnTransportError(
      const scoped_refptr<sippet::Request> &request, int error,
      const scoped_refptr<sippet::Dialog> &dialog) OVERRIDE {
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
      webrtc::PeerConnectionObserver::StateType state_changed) {}

  virtual void OnAddStream(webrtc::MediaStreamInterface* stream) {
    std::cout << "-- PeerConnection::OnAddStream " << stream->label();
  }

  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream) {
    std::cout << "-- PeerConnection::OnRemoveStream " << stream->label();
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
    peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
    desc->ToString(&offer_);
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
  string16 username_;
  string16 registrar_uri_;
  string16 server_;
  string16 phone_number_;
  
  std::string offer_;
  scoped_refptr<sippet::Dialog> dialog_;
  scoped_refptr<sippet::Request> last_invite_;

  talk_base::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;
  std::map<std::string, talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;

  enum State {
    STATE_REGISTER,
    STATE_REGISTER_COMPLETE,
    STATE_CREATE_OFFER,
    STATE_CREATE_OFFER_COMPLETE,
    STATE_INVITE,
    STATE_INVITE_COMPLETE,
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
    string16 from(L"sip:" + username_ + L"@" + server_);
    string16 to(L"sip:" + username_ + L"@" + server_);
    scoped_refptr<sippet::Request> request =
      user_agent_->CreateRequest(
      sippet::Method::REGISTER,
      GURL(registrar_uri_),
      GURL(from),
      GURL(to));
    int status = user_agent_->Send(request, base::Bind(&RequestSent,
        sippet::Method::REGISTER));
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

    string16 request_uri(L"sip:" + phone_number_ + L"@" + server_);
    string16 from(L"sip:" + username_ + L"@" + server_);
    string16 to(L"sip:" + phone_number_ + L"@" + server_);

    last_invite_ =
      user_agent_->CreateRequest(
      sippet::Method::INVITE,
      GURL(request_uri),
      GURL(from),
      GURL(to));

    scoped_ptr<sippet::ContentType> content_type(
      new sippet::ContentType("application", "sdp"));
    last_invite_->push_back(content_type.PassAs<sippet::Header>());
    last_invite_->set_content(offer_);

    int status = user_agent_->Send(last_invite_, base::Bind(&RequestSent,
      sippet::Method::REGISTER));
    if (net::OK != status && net::ERR_IO_PENDING != status)
      return status;
    return net::ERR_IO_PENDING;
  }

  int DoInviteComplete(int result) {
    if (net::OK != result)
      return result;

    next_state_ = STATE_NONE;
    return net::OK;
  }
};

int main(int argc, char **argv) {
#if defined(OS_MACOSX)
  // Needed so we don't leak objects when threads are created.
  base::mac::ScopedNSAutoreleasePool pool;
#endif

  talk_base::InitializeSSL();

  ProgramMain program_main(argc, argv);
  CommandLine* command_line = program_main.command_line();

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
    const char16 *registrar_uri_;
  } args[] = {
    { "udp", L"sip:%ls" },
    { "tcp", L"sip:%ls;transport=tcp" },
    { "tls", L"sips:%ls" },
    { "ws", L"sip:%ls;transport=ws" },
    { "wss", L"sips:%ls;transport=ws" },
  };

  for (int i = 0; i < ARRAYSIZE_UNSAFE(args); i++) {
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

  talk_base::scoped_refptr<UserAgentHandler> handler(
      new talk_base::RefCountedObject<UserAgentHandler>(settings));
  program_main.AppendHandler(handler.get());

  if (handler->InitializePeerConnection())
    handler->Start();

  program_main.Run();
  return 0;
}
