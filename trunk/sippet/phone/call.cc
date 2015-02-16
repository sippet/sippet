// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call.h"

#include "sippet/phone/phone.h"
#include "net/base/net_errors.h"

namespace sippet {
namespace phone {

Call::Call(const SipURI& uri, Phone* phone) :
  type_(kTypeOutgoing), state_(kStateCalling), uri_(uri), phone_(phone) {
}

Call::Call(const scoped_refptr<Request> &invite,
           Phone* phone) :
  type_(kTypeIncoming), state_(kStateRinging), uri_(invite->request_uri()),
  invite_(invite), phone_(phone) {
}

Call::~Call() {
}

std::string Call::name() const {
  return uri_.username();
}

bool Call::Answer(int code) {
  if (!invite_ || Request::Incoming != invite_->direction()) {
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
  if (!invite_) {
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

  invite_ =
    phone_->user_agent()->CreateRequest(
        sippet::Method::INVITE,
        GURL(request_uri),
        GURL(from),
        GURL(to));

  scoped_ptr<sippet::ContentType> content_type(
    new sippet::ContentType("application", "sdp"));
  invite_->push_back(content_type.Pass());
  invite_->set_content(""); // TODO


  int rv = phone_->user_agent()->Send(invite_,
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

}

} // namespace sippet
} // namespace phone