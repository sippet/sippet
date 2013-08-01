// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/request.h"
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

scoped_refptr<Response> Request::MakeResponse(int response_code,
                                              const std::string &to_tag) {
  scoped_refptr<Response> response(new Response(response_code));
  for (iterator via = find_first<Via>(); via != end();
       via = find_next<Via>(via)) {
    response->push_back(via->Clone().PassAs<Header>());
  }
  From* from = get<From>();
  response->push_back(from->Clone().PassAs<Header>());
  To* to = get<To>();
  if (to->HasTag())
    response->push_back(to->Clone().PassAs<Header>());
  else {
    scoped_ptr<To> newTo(to->Clone());
    newTo->set_tag(to_tag);
    response->push_back(newTo.PassAs<Header>());
  }
  CallId* callid = get<CallId>();
  response->push_back(callid->Clone().PassAs<Header>());
  Cseq* cseq = get<Cseq>();
  response->push_back(cseq->Clone().PassAs<Header>());
  if (response_code/100 == 1) {
    Timestamp *timestamp = get<Timestamp>();
    if (timestamp != NULL) {
      scoped_ptr<Timestamp> newTimestamp(timestamp->Clone());
      double delay = (base::Time::Now() - time_stamp_).InSecondsF();
      newTimestamp->set_delay(delay);
      response->push_back(newTimestamp.PassAs<Header>());
    }
  }
  response->set_refer_to(id_);
  return response;
}

} // End of sippet namespace
