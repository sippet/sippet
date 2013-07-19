// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_PROXY_AUTHENTICATE_H_
#define SIPPET_MESSAGE_HEADERS_PROXY_AUTHENTICATE_H_

#include "sippet/message/headers/www_authenticate.h"

namespace sippet {

class ProxyAuthenticate :
  public Header,
  public Challenge {
private:
  DISALLOW_ASSIGN(ProxyAuthenticate);
  ProxyAuthenticate(const ProxyAuthenticate &other)
    : Header(other), Challenge(other) {}
  virtual ProxyAuthenticate *DoClone() const OVERRIDE {
    return new ProxyAuthenticate(*this);
  }
public:
  ProxyAuthenticate() : Header(Header::HDR_PROXY_AUTHENTICATE) {}
  ProxyAuthenticate(Scheme s)
    : Header(Header::HDR_PROXY_AUTHENTICATE), Challenge(s) {}
  ProxyAuthenticate(const std::string &scheme)
    : Header(Header::HDR_PROXY_AUTHENTICATE), Challenge(scheme) {}

  scoped_ptr<ProxyAuthenticate> Clone() const {
    return scoped_ptr<ProxyAuthenticate>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    Challenge::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PROXY_AUTHENTICATE_H_
