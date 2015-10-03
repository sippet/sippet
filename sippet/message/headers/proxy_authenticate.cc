// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/proxy_authenticate.h"

#include <string>

namespace sippet {

ProxyAuthenticate::ProxyAuthenticate()
  : Header(Header::HDR_PROXY_AUTHENTICATE) {
}

ProxyAuthenticate::ProxyAuthenticate(Scheme s)
  : Header(Header::HDR_PROXY_AUTHENTICATE), Challenge(s) {
}

ProxyAuthenticate::ProxyAuthenticate(const std::string &scheme)
  : Header(Header::HDR_PROXY_AUTHENTICATE), Challenge(scheme) {
}

ProxyAuthenticate::ProxyAuthenticate(const ProxyAuthenticate &other)
  : Header(other), Challenge(other) {
}

ProxyAuthenticate::~ProxyAuthenticate() {
}

ProxyAuthenticate *ProxyAuthenticate::DoClone() const {
  return new ProxyAuthenticate(*this);
}

void ProxyAuthenticate::print(raw_ostream &os) const {
  Header::print(os);
  Challenge::print(os);
}

}  // namespace sippet
