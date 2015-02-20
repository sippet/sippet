// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call_impl.h"
#include "sippet/phone/phone_impl.h"

#include "base/callback.h"
#include "net/base/net_errors.h"
#include "re2/re2.h"
#include "sippet/phone/phone.h"
#include "sippet/message/status_code.h"

namespace {

class ProxyCreateSessionDescriptionObserver :
  public webrtc::CreateSessionDescriptionObserver {
public:
  static ProxyCreateSessionDescriptionObserver* Create(
      const base::Callback<void(webrtc::SessionDescriptionInterface*)> &on_success =
          base::Callback<void(webrtc::SessionDescriptionInterface*)>(),
      const base::Callback<void(const std::string&)> &on_failure =
          base::Callback<void(const std::string&)>()) {
    return
      new rtc::RefCountedObject<ProxyCreateSessionDescriptionObserver>(
          on_success, on_failure);
  }

  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    on_success_.Run(desc);
  }

  void OnFailure(const std::string& error) override {
    on_failure_.Run(error);
  }

protected:
  ProxyCreateSessionDescriptionObserver(
      const base::Callback<void(webrtc::SessionDescriptionInterface*)> &on_success,
      const base::Callback<void(const std::string&)> &on_failure) :
          on_success_(on_success), on_failure_(on_failure) {
  }
  ~ProxyCreateSessionDescriptionObserver() override {}

  base::Callback<void(webrtc::SessionDescriptionInterface*)> on_success_;
  base::Callback<void(const std::string&)> on_failure_;
};

class ProxySetSessionDescriptionObserver :
  public webrtc::SetSessionDescriptionObserver {
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
        const base::Callback<void(const std::string&)> &on_failure) :
            on_success_(on_success), on_failure_(on_failure) {
  }
  ~ProxySetSessionDescriptionObserver() override {}

  base::Callback<void()> on_success_;
  base::Callback<void(const std::string&)> on_failure_;
};

} // empty namespace

namespace sippet {
namespace phone {

CallImpl::CallImpl(const SipURI& uri, PhoneImpl* phone) :
  type_(kTypeOutgoing), state_(kStateCalling), uri_(uri), phone_(phone) {
}

CallImpl::CallImpl(const scoped_refptr<Request> &invite,
                   PhoneImpl* phone) :
  type_(kTypeIncoming), state_(kStateRinging), uri_(invite->request_uri()),
  last_request_(invite), phone_(phone) {
}

CallImpl::~CallImpl() {
}

CallImpl::Type CallImpl::type() const {
  return type_;
}

CallImpl::State CallImpl::state() const {
  return state_;
}

GURL CallImpl::uri() const {
  return GURL(uri_.spec());
}

std::string CallImpl::name() const {
  return uri_.username();
}

base::Time CallImpl::creation_time() const {
  return creation_time_;
}

base::Time CallImpl::start_time() const {
  return start_time_;
}

base::Time CallImpl::end_time() const {
  return end_time_;
}

base::TimeDelta CallImpl::duration() const {
  return end_time_ - start_time_;
}

bool CallImpl::Answer(int code) {
  if (!last_request_ || Request::Incoming != last_request_->direction()) {
    DVLOG(1) << "Impossible to answer an outgoing call";
    return false;
  }
  if (kStateRinging != state_) {
    DVLOG(1) << "Invalid state to answer call";
    return false;
  }
  phone_->signalling_message_loop()->PostTask(FROM_HERE,
    base::Bind(&CallImpl::OnAnswer, base::Unretained(this), code));
  return true;
}

bool CallImpl::HangUp() {
  if (!last_request_) {
    DVLOG(1) << "Impossible to hangup an uninitiated call";
    return false;
  }
  if (kStateHungUp == state_) {
    DVLOG(1) << "Cannot hangup an already hungup call";
    return false;
  }
  if (kStateError == state_) {
    DVLOG(1) << "Cannot hangup a call in error state";
    return false;
  }
  phone_->signalling_message_loop()->PostTask(FROM_HERE,
    base::Bind(&CallImpl::OnHangup, base::Unretained(this)));
  return true;
}

void CallImpl::SendDtmf(const std::string& digits) {
  if (!last_request_) {
    DVLOG(1) << "Impossible to send digit to an uninitiated call";
    return;
  }
  if (kStateHungUp == state_) {
    DVLOG(1) << "Cannot send digit to a hungup call";
    return;
  }
  if (kStateError == state_) {
    DVLOG(1) << "Cannot send digit to a call in error state";
    return;
  }
  phone_->signalling_message_loop()->PostTask(FROM_HERE,
    base::Bind(&CallImpl::OnSendDtmf, base::Unretained(this), digits));
}

bool CallImpl::InitializePeerConnection(
      webrtc::PeerConnectionFactoryInterface *peer_connection_factory) {
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  //webrtc::PeerConnectionInterface::IceServer server;
  //server.uri = "stun:stun.l.google.com:19302";
  //config.servers.push_back(server);

  peer_connection_ = peer_connection_factory->CreatePeerConnection(config,
    nullptr, nullptr, nullptr, this);
  if (!peer_connection_.get()) {
    LOG(ERROR) << "Error: CreatePeerConnection failed\n";
    DeletePeerConnection();
    return false;
  }

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
    peer_connection_factory->CreateAudioTrack(
    "audio", peer_connection_factory->CreateAudioSource(nullptr)));

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
    peer_connection_factory->CreateLocalMediaStream("stream");

  stream->AddTrack(audio_track);
  if (!peer_connection_->AddStream(stream)) {
    LOG(ERROR) << "Adding stream to PeerConnection failed";
  }
  active_streams_.insert(std::make_pair(stream->label(), stream));
  return true;
}

void CallImpl::DeletePeerConnection() {
  peer_connection_ = nullptr;
  active_streams_.clear();
}

void CallImpl::OnMakeCall(
  webrtc::PeerConnectionFactoryInterface *peer_connection_factory) {
  InitializePeerConnection(peer_connection_factory);
  // TODO: handle errors

  CreateOffer();

  // Wait for allocations
}

void CallImpl::CreateOffer() {
  peer_connection_->CreateOffer(
    ProxyCreateSessionDescriptionObserver::Create(
      base::Bind(&CallImpl::OnCreateSessionSuccess, base::Unretained(this)),
      base::Bind(&CallImpl::OnCreateSessionFailure, base::Unretained(this))
    ), nullptr);
}

void CallImpl::OnIceComplete() {
  std::string offer;
  const webrtc::SessionDescriptionInterface* desc =
    peer_connection_->local_description();
  desc->ToString(&offer);

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
    RE2::GlobalReplace(&offer, elem.first, elem.second);
  }

  phone_->signalling_message_loop()->PostTask(
    FROM_HERE,
    base::Bind(&CallImpl::OnCreateOfferCompleted,
    base::Unretained(this), offer));
}

void CallImpl::OnCreateSessionSuccess(webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetLocalDescription(
    ProxySetSessionDescriptionObserver::Create(
      base::Bind(&CallImpl::OnSetLocalSessionSuccess, base::Unretained(this)),
      base::Bind(&CallImpl::OnSetLocalSessionFailure, base::Unretained(this))
    ), desc);
}

void CallImpl::OnCreateSessionFailure(const std::string& error) {
  // TODO
}

void CallImpl::OnSetLocalSessionSuccess() {
  // TODO
}

void CallImpl::OnSetLocalSessionFailure(const std::string& error) {
  // TODO
}

void CallImpl::OnSetRemoteSessionSuccess() {
  // TODO
}

void CallImpl::OnSetRemoteSessionFailure(const std::string& error) {
  // TODO
}

void CallImpl::OnCreateOfferCompleted(const std::string& offer) {
  std::string request_uri(uri_.spec());
  std::string from("sip:" + phone_->username() + "@" + phone_->host());
  std::string to(uri_.spec());

  last_request_ =
    phone_->user_agent()->CreateRequest(
        Method::INVITE,
        GURL(request_uri),
        GURL(from),
        GURL(to));

  scoped_ptr<ContentType> content_type(
    new ContentType("application", "sdp"));
  last_request_->push_back(content_type.Pass());
  last_request_->set_content(offer);

  int rv = phone_->user_agent()->Send(last_request_,
    base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(phone_)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_->phone_observer()->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void CallImpl::HandleSessionDescriptionAnswer(
      const scoped_refptr<Response> &incoming_response) {
  ContentType *content_type = incoming_response->get<ContentType>();
  if (content_type != nullptr
      && "application" == content_type->MediaType::type()
      && "sdp" == content_type->subtype()) {
    webrtc::SessionDescriptionInterface *desc =
      webrtc::CreateSessionDescription(
          webrtc::SessionDescriptionInterface::kAnswer,
          incoming_response->content());
    if (desc) {
      peer_connection_->SetRemoteDescription(
        ProxySetSessionDescriptionObserver::Create(
          base::Bind(&CallImpl::OnSetRemoteSessionSuccess, base::Unretained(this)),
          base::Bind(&CallImpl::OnSetRemoteSessionFailure, base::Unretained(this))
        ), desc);
    }
  }
}

void CallImpl::SendAck(const scoped_refptr<Response> &incoming_response) {
  scoped_refptr<Request> ack = dialog_->CreateAck(
    incoming_response->refer_to());
  int rv = phone_->user_agent()->Send(ack, base::Bind(
    &PhoneImpl::OnRequestSent, phone_));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_->phone_observer()->OnNetworkError(rv);
    return;
  }

  // ACK is a final request, so there's no answer for it.
}

void CallImpl::SendCancel() {
  // Send a CANCEL in this case
  scoped_refptr<Request> cancel;
  int rv = last_request_->CreateCancel(cancel);
  if (net::OK != rv) {
    LOG(ERROR) << "Unexpected error while creating CANCEL: "
      << net::ErrorToString(rv);
  }
  else {
    int rv = phone_->user_agent()->Send(cancel,
      base::Bind(&PhoneImpl::OnRequestSent, phone_));
    if (net::OK != rv && net::ERR_IO_PENDING != rv) {
      phone_->phone_observer()->OnNetworkError(rv);
      return;
    }

    // Wait for SIP response now
  }
}

void CallImpl::SendBye() {
  last_request_ = dialog_->CreateRequest(Method::BYE);
  int rv = phone_->user_agent()->Send(last_request_,
    base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(phone_)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_->phone_observer()->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void CallImpl::HandleCallingOrRingingResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog) {
  DCHECK(kStateRinging == state_ || kStateCalling == state_);

  State next_state; // Determine the next state
  int response_code = incoming_response->response_code();
  if (response_code / 100 == 1) { // 1xx
    if (response_code / 10 == 18) { // 18x
      next_state = kStateRinging;
    } else {
      next_state = state_; // Keep on same state
    }
  } else if (response_code / 100 == 2) { // 2xx
    next_state = kStateEstablished;
  } else {
    next_state = kStateError;
  }

  // Send ACK for every 2xx
  if (next_state == kStateEstablished) {
    SendAck(incoming_response);
  }

  // Handle Session Description on ringing or established
  if (state_ != next_state) {
    if (kStateRinging == next_state
        || kStateEstablished == next_state) {
      HandleSessionDescriptionAnswer(incoming_response);
    }
  }

  // Now change state and process post changed state conditions
  state_ = next_state;
  if (kStateRinging == state_) {
    phone_->phone_observer()->OnCallRinging(this);
  } else if (kStateEstablished == state_) {
    phone_->phone_observer()->OnCallEstablished(this);
  } else if (kStateError == state_) {
    phone_->phone_observer()->OnCallError(response_code,
      incoming_response->reason_phrase(), this);
    phone_->RemoveCall(this);
  }
}

void CallImpl::HandleHungupResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog) {
  DCHECK(kStateHungUp == state_);
  int response_code = incoming_response->response_code();
  if (Method::INVITE == incoming_response->refer_to()->method()) {
    if (response_code / 100 == 2) { // 2xx
      // In that case, we sent CANCEL, but it didn't get through,
      // so we have to send a BYE (it's a normal race condition).
      SendBye();
    }
  } else if (Method::CANCEL == incoming_response->refer_to()->method()
             || Method::BYE == incoming_response->refer_to()->method()) {
    if (response_code / 100 == 2) { // 2xx for CANCEL or BYE
      // The CANCEL or BYE request concluded
      // successfully, time to release the call.
      phone_->RemoveCall(this);
    }
  }
}

void CallImpl::OnDestroy() {
  peer_connection_->Close();
  peer_connection_ = nullptr;
  active_streams_.clear();
}

void CallImpl::OnAnswer(int code) {
  // TODO
}

void CallImpl::OnHangup() {
  state_ = kStateHungUp;
  if (!dialog_) {
    // Wait until the server answers a 18x before sending CANCEL
  } else if (Dialog::STATE_EARLY == dialog_->state()) {
    SendCancel();
  } else {
    SendBye();
  }
}

void CallImpl::OnSendDtmf(const std::string& digits) {
  if (!dtmf_sender_) {
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
        active_streams_["stream"];
    dtmf_sender_ =
        peer_connection_->CreateDtmfSender(stream->FindAudioTrack("audio"));
  }
  if (!dtmf_sender_->CanInsertDtmf()) {
    LOG(INFO) << "Current call doesn't support DTMF";
    return;
  }
  if (!dtmf_sender_->InsertDtmf(digits, 150, 50)) {
    LOG(INFO) << "Error inserting DTMF";
  }
}

void CallImpl::OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) {
  DCHECK(dialog == dialog_);
  if (Method::BYE == incoming_request->method()) {
    OnDestroy();
    scoped_refptr<Response> response =
        dialog->CreateResponse(SIP_OK, incoming_request);
    phone_->user_agent()->Send(response,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(phone_)));
    state_ = kStateHungUp;
    phone_->phone_observer()->OnCallHungUp(this);
    phone_->RemoveCall(this);
  }
}

void CallImpl::OnIncomingResponse(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  dialog_ = dialog; // Save dialog
  if (kStateCalling == state_
      || kStateRinging == state_) {
    HandleCallingOrRingingResponse(incoming_response, dialog);
  } else if (kStateHungUp == state_) {
    HandleHungupResponse(incoming_response, dialog);
  }
}

void CallImpl::OnTimedOut(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) {
  if (kStateCalling == state_
      || kStateRinging == state_) {
    state_ = kStateError;
    phone_->phone_observer()->OnCallError(SIP_REQUEST_TIMEOUT,
      GetReasonPhrase(SIP_REQUEST_TIMEOUT), this);
    phone_->RemoveCall(this);
  }
}

void CallImpl::OnTransportError(
    const scoped_refptr<Request> &request, int error,
    const scoped_refptr<Dialog> &dialog) {
  if (kStateCalling == state_
      || kStateRinging == state_) {
    state_ = kStateError;
    phone_->phone_observer()->OnCallError(SIP_SERVICE_UNAVAILABLE,
      GetReasonPhrase(SIP_SERVICE_UNAVAILABLE), this);
    phone_->RemoveCall(this);
  }
}

} // namespace sippet
} // namespace phone
