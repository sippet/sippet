// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/*
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

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
