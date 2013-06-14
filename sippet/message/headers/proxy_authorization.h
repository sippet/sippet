// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_PROXY_AUTHORIZATION_H_
#define SIPPET_MESSAGE_HEADERS_PROXY_AUTHORIZATION_H_

#include "sippet/message/headers/authorization.h"

namespace sippet {

class ProxyAuthorization :
  public Header,
  public Credentials {
private:
  DISALLOW_ASSIGN(ProxyAuthorization);
  ProxyAuthorization(const ProxyAuthorization &other)
    : Header(other), Credentials(other) {}
  virtual ProxyAuthorization *DoClone() const {
    return new ProxyAuthorization(*this);
  }
public:
  ProxyAuthorization() : Header(Header::HDR_PROXY_AUTHORIZATION) {}
  ProxyAuthorization(Scheme s)
    : Header(Header::HDR_PROXY_AUTHORIZATION), Credentials(s) {}
  ProxyAuthorization(const std::string &scheme)
    : Header(Header::HDR_PROXY_AUTHORIZATION), Credentials(scheme) {}

  scoped_ptr<ProxyAuthorization> Clone() const {
    return scoped_ptr<ProxyAuthorization>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Proxy-Authorization");
    Credentials::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PROXY_AUTHORIZATION_H_
