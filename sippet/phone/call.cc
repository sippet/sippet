// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call.h"

#include "sippet/phone/phone.h"
#include "sippet/message/status_code.h"
#include "net/base/net_errors.h"

namespace sippet {
namespace phone {

Call::Call(const SipURI& uri, Phone* phone) :
  type_(kTypeOutgoing), state_(kStateCalling), uri_(uri), phone_(phone) {
}

Call::Call(const scoped_refptr<Request> &invite,
           Phone* phone) :
  type_(kTypeIncoming), state_(kStateRinging), uri_(invite->request_uri()),
  last_request_(invite), phone_(phone) {
}

Call::~Call() {
}

bool Call::Answer(int code) {
  if (!last_request_ || Request::Incoming != last_request_->direction()) {
    DVLOG(1) << "Impossible to answer an outgoing call";
    return false;
  }
  if (kStateRinging != state_) {
    DVLOG(1) << "Invalid state to answer call";
    return false;
  }
  phone_->signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Call::OnAnswer, base::Unretained(this), code));
  return true;
}

bool Call::HangUp() {
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
  phone_->signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Call::OnHangup, base::Unretained(this)));
  return true;
}

void Call::OnMakeCall() {
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
  last_request_->set_content(""); // TODO

  int rv = phone_->user_agent()->Send(last_request_,
    base::Bind(&Phone::OnRequestSent, base::Unretained(phone_)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_->phone_observer()->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void Call::OnAnswer(int code) {
  // TODO
}

void Call::OnHangup() {
  last_request_ =
    dialog_->CreateRequest(Method::BYE);

  int rv = phone_->user_agent()->Send(last_request_,
    base::Bind(&Phone::OnRequestSent, base::Unretained(phone_)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_->phone_observer()->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void Call::OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) {
  DCHECK(dialog == dialog_);
  if (Method::BYE == incoming_request->method()) {
    state_ = kStateHungUp;
    phone_->phone_observer()->OnCallHungUp(this);
    phone_->RemoveCall(this);
  }
}

void Call::OnIncomingResponse(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  int response_code = incoming_response->response_code();
  switch (response_code / 100) {
    case 1:
      if (kStateCalling == state_) {
        switch (response_code / 10) {
          case 18: // 18x
            state_ = kStateRinging;
            dialog_ = dialog;
            phone_->phone_observer()->OnCallRinging(this);
            break;
        }
      }
      break;
    case 2:
      if (kStateCalling == state_
          || kStateRinging == state_) {
        state_ = kStateEstablished;
        DCHECK(!dialog_ || dialog_ == dialog);
        dialog_ = dialog;
        scoped_refptr<Request> ack = dialog->CreateAck(last_request_);
        phone_->user_agent()->Send(ack, base::Bind(
          &Phone::OnRequestSent, phone_));
        phone_->phone_observer()->OnCallEstablished(this);
      }
      break;
    default:
      if (kStateCalling == state_
          || kStateRinging == state_) {
        state_ = kStateError;
        phone_->phone_observer()->OnCallError(response_code,
            incoming_response->reason_phrase(), this);
        phone_->RemoveCall(this);
      }
      break;
  }
}

void Call::OnTimedOut(
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

void Call::OnTransportError(
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