// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
#define SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/auth_setters.h"
#include "sippet/message/headers/bits/has_auth_params.h"
#include "sippet/base/raw_ostream.h"

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
  Challenge();
  Challenge(Scheme s);
  Challenge(const std::string &scheme);
  Challenge(const Challenge &other);
  ~Challenge();

  std::string ToString();
};

class WwwAuthenticate :
  public Header,
  public Challenge {
 private:
  DISALLOW_ASSIGN(WwwAuthenticate);
  WwwAuthenticate(const WwwAuthenticate &other);
  WwwAuthenticate *DoClone() const override;

 public:
  WwwAuthenticate();
  WwwAuthenticate(Scheme s);
  WwwAuthenticate(const std::string &scheme);
  ~WwwAuthenticate() override;

  scoped_ptr<WwwAuthenticate> Clone() const {
    return scoped_ptr<WwwAuthenticate>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
