// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_IMPL_H_
#define SIPPET_PHONE_CALL_IMPL_H_

#include "sippet/uri/uri.h"
#include "sippet/phone/call.h"
#include "sippet/message/request.h"

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
  Type type() const override { return type_; }
  State state() const override { return state_; }
  GURL uri() const override { return GURL(uri_.spec()); }
  std::string name() const override { return uri_.username(); }
  base::Time creation_time() const override { return creation_time_; }
  base::Time start_time() const override { return start_time_; }
  base::Time end_time() const override { return end_time_; }

  base::TimeDelta duration() const override {
    return end_time_ - start_time_;
  }

  bool Answer(int code = 200) override;
  bool HangUp() override;
  void SendDtmf(const std::string& digits) override;

 private:
  friend class PhoneImpl;
  friend class base::RefCountedThreadSafe<Call>;

  Type type_;
  State state_;
  SipURI uri_;
  PhoneImpl *phone_;
  scoped_refptr<Request> last_request_;
  scoped_refptr<Dialog> dialog_;
  
  base::Time creation_time_;
  base::Time start_time_;
  base::Time end_time_;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DtmfSenderInterface> dtmf_sender_;
  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;

  CallImpl(const SipURI& uri, PhoneImpl* phone);
  CallImpl(const scoped_refptr<Request> &invite, PhoneImpl* phone);
  virtual ~CallImpl();

  bool InitializePeerConnection(
        webrtc::PeerConnectionFactoryInterface *peer_connection_factory);
  void DeletePeerConnection();
  void CreateOffer();
  void OnCreateOfferCompleted(const std::string& offer);
  void HandleSessionAnswer(const scoped_refptr<Response> &incoming_response);

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
  void OnAnswer(int code);
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
