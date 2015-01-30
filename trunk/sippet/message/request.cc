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
  : Message(true, Outgoing), method_(method), request_uri_(request_uri),
    version_(version), time_stamp_(base::Time::Now()),
    id_(base::GenerateGUID()) {}

Request::Request(const Method &method,
                 const GURL &request_uri,
                 Direction direction,
                 const Version &version)
  : Message(true, direction), method_(method), request_uri_(request_uri),
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

std::string Request::GetDialogId() const {
  std::string from_tag;
  std::string to_tag;
  std::string call_id(get<CallId>()->value());
  if (get<From>()->HasTag())
    from_tag = get<From>()->tag();
  if (get<To>()->HasTag())
    to_tag = get<To>()->tag();
  std::ostringstream oss;
  oss << call_id << ":";
  if (direction() == Outgoing) {
    oss << from_tag << ":" << to_tag;
  } else {
    oss << to_tag << ":" << from_tag;
  }
  return oss.str();
}

scoped_refptr<Request> Request::CloneRequest() const {
  scoped_refptr<Request> result(
    new Request(method(), request_uri(), version()));
  for (Message::const_iterator i = begin(), ie = end(); i != ie; ++i) {
    result->push_back(i->Clone().Pass());
  }
  if (has_content())
    result->set_content(content());
  Message::iterator j = result->find_first<Cseq>();
  if (result->end() != j) {
    Cseq *cseq = dyn_cast<Cseq>(j);
    cseq->set_sequence(cseq->sequence() + 1);
  }
  return result;
}

scoped_refptr<Response> Request::CreateResponse(
    int response_code,
    const std::string &reason_phrase) {
  scoped_refptr<Response> response(
      CreateResponseInternal(response_code, reason_phrase));
  To *to = response->get<To>();
  if (response_code > 100 && to && !to->HasTag())
    to->set_tag(CreateTag());
  return response;
}

scoped_refptr<Response> Request::CreateResponse(StatusCode code) {
  return CreateResponse(static_cast<int>(code), GetReasonPhrase(code));
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
  CloneTo<Via>(ack.get());
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  ack->push_back(max_forwards.Pass());
  CloneTo<From>(ack.get());
  scoped_ptr<To> to(Clone<To>().Pass());
  if (to && remote_tag.length() > 0) to->set_tag(remote_tag);
  ack->push_back(to.Pass());
  CloneTo<CallId>(ack.get());
  scoped_ptr<Cseq> cseq(Clone<Cseq>().Pass());
  if (cseq) cseq->set_method(Method::ACK);
  ack->push_back(cseq.Pass());
  CloneTo<Route>(ack.get());
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
  CloneTo<Via>(cancel.get());
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  cancel->push_back(max_forwards.Pass());
  CloneTo<From>(cancel.get());
  CloneTo<To>(cancel.get());
  CloneTo<CallId>(cancel.get());
  scoped_ptr<Cseq> cseq(Clone<Cseq>());
  if (cseq) cseq->set_method(Method::CANCEL);
  cancel->push_back(cseq.Pass());
  CloneTo<Route>(cancel.get());
  return net::OK;
}

scoped_refptr<Response> Request::CreateResponseInternal(
    int response_code,
    const std::string &reason_phrase) {
  if (Message::Incoming != direction()) {
    DVLOG(1) << "Trying to create a response from an outgoing request";
    return 0;
  }
  scoped_refptr<Response> response(
      new Response(response_code, reason_phrase, Message::Outgoing));
  CloneTo<Via>(response.get());
  CloneTo<From>(response.get());
  CloneTo<To>(response.get());
  CloneTo<CallId>(response.get());
  CloneTo<Cseq>(response.get());
  if (response_code == 100) {
    scoped_ptr<Timestamp> timestamp(Clone<Timestamp>());
    if (timestamp) {
      double delay = (base::Time::Now() - time_stamp_).InSecondsF();
      timestamp->set_delay(delay);
      response->push_back(timestamp.Pass());
    }
  }
  CloneTo<RecordRoute>(response.get());
  response->set_refer_to(this);
  return response;
}

scoped_refptr<Response> Request::CreateResponse(
    int response_code,
    const std::string &reason_phrase,
    const std::string &remote_tag) {
  scoped_refptr<Response> response(
      CreateResponseInternal(response_code, reason_phrase));
  To *to = response->get<To>();
  if (to) to->set_tag(remote_tag);
  return response;
}

} // End of sippet namespace
