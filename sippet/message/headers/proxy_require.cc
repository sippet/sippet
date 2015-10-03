// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/proxy_require.h"

namespace sippet {

ProxyRequire::ProxyRequire()
  : Header(Header::HDR_PROXY_REQUIRE) {
}

ProxyRequire::ProxyRequire(const std::string &value)
  : Header(Header::HDR_PROXY_REQUIRE) {
  push_back(value);
}

ProxyRequire::ProxyRequire(const ProxyRequire &other)
  : Header(other), has_multiple(other) {
}

ProxyRequire::~ProxyRequire() {
}

ProxyRequire *ProxyRequire::DoClone() const {
  return new ProxyRequire(*this);
}

void ProxyRequire::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
