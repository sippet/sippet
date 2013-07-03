// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_
#define SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/auth_setters.h"
#include "sippet/message/headers/bits/has_auth_params.h"

namespace sippet {

class Credentials :
  public has_username<Credentials>,
  public has_realm<Credentials>,
  public has_nonce<Credentials>,
  public has_uri<Credentials>,
  public has_response<Credentials>,
  public has_algorithm<Credentials>,
  public has_cnonce<Credentials>,
  public has_opaque<Credentials>,
  public has_nc<Credentials>,
  public has_auth_params {
public:
  Credentials() {}
  Credentials(Scheme s) : has_auth_params(s) {}
  Credentials(const std::string &scheme) : has_auth_params(scheme) {}
  Credentials(const Credentials &other)
    : has_auth_params(other) {}
  ~Credentials() {}
};

class Authorization :
  public Header,
  public Credentials {
private:
  DISALLOW_ASSIGN(Authorization);
  Authorization(const Authorization &other)
    : Header(other), Credentials(other) {}
  virtual Authorization *DoClone() const {
    return new Authorization(*this);
  }
public:
  Authorization() : Header(Header::HDR_AUTHORIZATION) {}
  Authorization(Scheme s)
    : Header(Header::HDR_AUTHORIZATION), Credentials(s) {}
  Authorization(const std::string &scheme)
    : Header(Header::HDR_AUTHORIZATION), Credentials(scheme) {}

  scoped_ptr<Authorization> Clone() const {
    return scoped_ptr<Authorization>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Authorization");
    Credentials::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_