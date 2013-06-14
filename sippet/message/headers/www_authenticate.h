// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
#define SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_

#include "sippet/message/headers/bits/auth_setters.h"

namespace sippet {

class Challenge :
  public has_realm<Challenge>,
  public has_domain<Challenge>,
  public has_nonce<Challenge>,
  public has_opaque<Challenge>,
  public has_stale<Challenge>,
  public has_algorithm<Challenge>,
  public has_qop_options<Challenge>,
  public has_auth_params {
public:
  Challenge() {}
  Challenge(Scheme s) : has_auth_params(s) {}
  Challenge(const std::string &scheme) : has_auth_params(scheme) {}
  Challenge(const Challenge &other) : has_auth_params(other) {}
  ~Challenge() {}
};

class WwwAuthenticate :
  public Header,
  public Challenge {
private:
  DISALLOW_ASSIGN(WwwAuthenticate);
  WwwAuthenticate(const WwwAuthenticate &other)
    : Header(other), Challenge(other) {}
  virtual WwwAuthenticate *DoClone() const {
    return new WwwAuthenticate(*this);
  }
public:
  WwwAuthenticate() : Header(Header::HDR_WWW_AUTHENTICATE) {}
  WwwAuthenticate(Scheme s)
    : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(s) {}
  WwwAuthenticate(const std::string &scheme)
    : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(scheme) {}

  scoped_ptr<WwwAuthenticate> Clone() const {
    return scoped_ptr<WwwAuthenticate>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("WWW-Authenticate");
    Challenge::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
