// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_REQUEST_H_
#define SIPPET_MESSAGE_REQUEST_H_

#include "sippet/message/message.h"
#include "sippet/message/method.h"
#include "sippet/message/version.h"
#include "googleurl/src/gurl.h"

namespace sippet {

class Request :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Request);

private:
  virtual ~Request() {}

public:
  Request(const Method &method,
          const GURL &request_uri,
          const Version &version = Version(2,0)) 
    : Message(true), method_(method), request_uri_(request_uri), version_(version) {}

  Method method() const { return method_; }
  void set_method(const Method &method) {
    method_ = method;
  }

  GURL request_uri() const { return request_uri_; }
  void set_request_uri(const GURL &request_uri) {
    request_uri_ = request_uri;
  }

  Version version() const { return version_; }
  void set_version(const Version &version) {
    version_ = version;
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    os << method_.str() << " "
       << request_uri_.spec() << " "
       << "SIP/" << version_.major_value()
       << "." << version_.minor_value()
       << "\r\n";
    Message::print(os);
  }

private:
  Method method_;
  GURL request_uri_;
  Version version_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_REQUEST_H_
