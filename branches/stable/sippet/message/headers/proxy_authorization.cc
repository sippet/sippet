// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/proxy_authorization.h"

namespace sippet {

ProxyAuthorization::ProxyAuthorization()
  : Header(Header::HDR_PROXY_AUTHORIZATION) {
}

ProxyAuthorization::ProxyAuthorization(Scheme s)
  : Header(Header::HDR_PROXY_AUTHORIZATION), Credentials(s) {
}

ProxyAuthorization::ProxyAuthorization(const std::string &scheme)
  : Header(Header::HDR_PROXY_AUTHORIZATION), Credentials(scheme) {
}

ProxyAuthorization::ProxyAuthorization(const ProxyAuthorization &other)
  : Header(other), Credentials(other) {
}

ProxyAuthorization::~ProxyAuthorization() {
}

ProxyAuthorization *ProxyAuthorization::DoClone() const {
  return new ProxyAuthorization(*this);
}

void ProxyAuthorization::print(raw_ostream &os) const {
  Header::print(os);
  Credentials::print(os);
}

} // End of sippet namespace
