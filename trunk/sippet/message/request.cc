// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/request.h"
#include "sippet/base/tags.h"
#include "net/base/net_errors.h"
#include "base/guid.h"

namespace sippet {

Request::Request(const Method &method,
                 const GURL &request_uri,
                 const Version &version)
  : Message(true), method_(method), request_uri_(request_uri),
    version_(version), time_stamp_(base::Time::Now()),
    id_(base::GenerateGUID()) {}

Request::~Request() {}

Method Request::method() const {
  return method_;
}

void Request::set_method(const Method &method) {
  method_ = method;
}

GURL Request::request_uri() const {
  return request_uri_;
}

void Request::set_request_uri(const GURL &request_uri) {
  request_uri_ = request_uri;
}

Version Request::version() const {
  return version_;
}

void Request::set_version(const Version &version) {
  version_ = version;
}

void Request::print(raw_ostream &os) const {
  os << method_.str() << " "
     << request_uri_.spec() << " "
     << "SIP/" << version_.major_value()
     << "." << version_.minor_value()
     << "\r\n";
  Message::print(os);
}

scoped_refptr<Response> Request::CreateResponse(int response_code) {
  scoped_refptr<Response> response(new Response(response_code));
  copy_to<Via>(response);
  scoped_ptr<From> from(get<From>()->Clone());
  response->push_back(from.PassAs<Header>());
  scoped_ptr<To> to(get<To>()->Clone());
  if (to->HasTag())
    response->push_back(to.PassAs<Header>());
  else {
    to->set_tag(CreateTag());
    response->push_back(to.PassAs<Header>());
  }
  scoped_ptr<CallId> call_id(get<CallId>()->Clone());
  response->push_back(call_id.PassAs<Header>());
  scoped_ptr<Cseq> cseq(get<Cseq>()->Clone());
  response->push_back(cseq.PassAs<Header>());
  if (response_code == 100) {
    Timestamp *timestamp = get<Timestamp>();
    if (timestamp != NULL) {
      scoped_ptr<Timestamp> newTimestamp(timestamp->Clone());
      double delay = (base::Time::Now() - time_stamp_).InSecondsF();
      newTimestamp->set_delay(delay);
      response->push_back(newTimestamp.PassAs<Header>());
    }
  }
  copy_to<RecordRoute>(response);
  response->set_refer_to(id_);
  return response;
}

int Request::CreateAck(const std::string &remote_tag,
                       scoped_refptr<Request> &ack) {
  if (Method::INVITE != method()) {
    DVLOG(1) << "Cannot create an ACK from a non-INVITE request";
    return net::ERR_NOT_IMPLEMENTED;
  }
  if (end() == find_first<From>()
      || end() == find_first<To>()
      || end() == find_first<Cseq>()
      || end() == find_first<CallId>()) {
    DVLOG(1) << "Incomplete INVITE request, cannot create the ACK";
    return net::ERR_UNEXPECTED;
  }
  if (end() == find_first<Via>()) {
    DVLOG(1) << "INVITE request was not sent yet, cannot create the ACK";
    return net::ERR_UNEXPECTED;
  }
  ack = new Request(Method::ACK, request_uri());
  copy_to<Via>(ack);
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  ack->push_back(max_forwards.PassAs<Header>());
  scoped_ptr<From> from(get<From>()->Clone());
  ack->push_back(from.PassAs<Header>());
  scoped_ptr<To> to(get<To>()->Clone());
  to->set_tag(remote_tag);
  ack->push_back(to.PassAs<Header>());
  scoped_ptr<CallId> call_id(get<CallId>()->Clone());
  ack->push_back(call_id.PassAs<Header>());
  scoped_ptr<Cseq> cseq(new Cseq(get<Cseq>()->sequence(), Method::ACK));
  ack->push_back(cseq.PassAs<Header>());
  copy_to<Route>(ack);
  return net::OK;
}

int Request::CreateCancel(scoped_refptr<Request> &cancel) {
  if (Method::INVITE != method()) {
    DVLOG(1) << "Cannot create an ACK from a non-INVITE request";
    return net::ERR_NOT_IMPLEMENTED;
  }
  if (end() == find_first<From>()
      || end() == find_first<To>()
      || end() == find_first<Cseq>()
      || end() == find_first<CallId>()) {
    DVLOG(1) << "Incomplete INVITE request, cannot create the ACK";
    return net::ERR_UNEXPECTED;
  }
  if (end() == find_first<Via>()) {
    DVLOG(1) << "INVITE request was not sent yet, cannot create the ACK";
    return net::ERR_UNEXPECTED;
  }
  cancel = new Request(Method::CANCEL, request_uri());
  copy_to<Via>(cancel);
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  cancel->push_back(max_forwards.PassAs<Header>());
  scoped_ptr<From> from(get<From>()->Clone());
  cancel->push_back(from.PassAs<Header>());
  scoped_ptr<To> to(get<To>()->Clone());
  cancel->push_back(to.PassAs<Header>());
  scoped_ptr<CallId> call_id(get<CallId>()->Clone());
  cancel->push_back(call_id.PassAs<Header>());
  scoped_ptr<Cseq> cseq(new Cseq(get<Cseq>()->sequence(), Method::CANCEL));
  cancel->push_back(cseq.PassAs<Header>());
  copy_to<Route>(cancel);
  return net::OK;
}

std::string Request::CreateTag() {
  return Create32BitRandomString();
}

} // End of sippet namespace
