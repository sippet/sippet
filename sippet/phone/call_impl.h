// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_IMPL_H_
#define SIPPET_PHONE_CALL_IMPL_H_

#include <map>

#include "sippet/uri/uri.h"
#include "sippet/phone/call.h"
#include "sippet/message/request.h"
#include "sippet/ua/dialog.h"

#include "talk/app/webrtc/peerconnectioninterface.h"

namespace sippet {
namespace phone {

class PhoneImpl;

class CallImpl :
  public Call,
  public webrtc::PeerConnectionObserver {
 private:
  DISALLOW_COPY_AND_ASSIGN(CallImpl);
 public:
  CallDirection direction() const override;
  CallState state() const override;
  GURL uri() const override;
  std::string name() const override;
  base::Time creation_time() const override;
  base::Time start_time() const override;
  base::Time end_time() const override;
  base::TimeDelta duration() const override;
  bool PickUp(const net::CompletionCallback& on_completed) override;
  bool Reject() override;
  bool HangUp(const net::CompletionCallback& on_completed) override;
  void SendDtmf(const std::string& digits) override;

 private:
  friend class PhoneImpl;
  friend class base::RefCountedThreadSafe<Call>;

  CallDirection direction_;
  CallState state_;
  SipURI uri_;
  PhoneImpl *phone_;
  scoped_refptr<Request> last_request_;
  scoped_refptr<Dialog> dialog_;
  net::CompletionCallback on_completed_;
  net::CompletionCallback on_hangup_completed_;

  base::Time creation_time_;
  base::Time start_time_;
  base::Time end_time_;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DtmfSenderInterface> dtmf_sender_;
  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;

  CallImpl(const SipURI& uri, PhoneImpl* phone,
      const net::CompletionCallback& on_completed);
  CallImpl(const scoped_refptr<Request> &invite, PhoneImpl* phone);
  ~CallImpl() override;

  bool InitializePeerConnection(
        webrtc::PeerConnectionFactoryInterface *peer_connection_factory);
  void DeletePeerConnection();
  void CreateOffer();
  void OnCreateOfferCompleted(const std::string& offer);
  void HandleSessionDescriptionAnswer(const scoped_refptr<Response> &incoming_response);
  void SendAck(const scoped_refptr<Response> &incoming_response);
  void SendCancel();
  void SendBye();
  void HandleCallingOrRingingResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog);
  void HandleHungupResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog);

  //
  // PeerConnectionObserver implementation.
  //
  void OnAddStream(webrtc::MediaStreamInterface* stream) override {}
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override {}
  void OnDataChannel(webrtc::DataChannelInterface* data_channel) override {}
  void OnRenegotiationNeeded() override {/* TODO*/}
  void OnIceCandidate(
        const webrtc::IceCandidateInterface* candidate) override {}
  void OnIceComplete() override;

  //
  // CreateSessionDescriptionObserver callbacks.
  //
  void OnCreateSessionSuccess(webrtc::SessionDescriptionInterface* desc);
  void OnCreateSessionFailure(const std::string& error);

  //
  // SetSessionDescriptionObserver callbacks.
  //
  void OnSetLocalSessionSuccess();
  void OnSetLocalSessionFailure(const std::string& error);
  void OnSetRemoteSessionSuccess();
  void OnSetRemoteSessionFailure(const std::string& error);

  //
  // Signalling thread callbacks
  //
  void OnMakeCall(
        webrtc::PeerConnectionFactoryInterface *peer_connection_factory);
  void OnPickUp();
  void OnReject();
  void OnHangup();
  void OnSendDtmf(const std::string& digits);
  void OnDestroy();

  //
  // Phone callbacks
  //
  const scoped_refptr<Request> &last_request() const {
    return last_request_;
  }
  const scoped_refptr<Dialog> &dialog() const {
    return dialog_;
  }
  void OnIncomingRequest(
      const scoped_refptr<Request> &incoming_request,
      const scoped_refptr<Dialog> &dialog);
  void OnIncomingResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog);
  void OnTimedOut(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Dialog> &dialog);
  void OnTransportError(
      const scoped_refptr<Request> &request, int error,
      const scoped_refptr<Dialog> &dialog);
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_CALL_IMPL_H_
