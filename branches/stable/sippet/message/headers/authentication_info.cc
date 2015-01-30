// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/authentication_info.h"

namespace sippet {

AuthenticationInfo::AuthenticationInfo()
  : Header(Header::HDR_AUTHENTICATION_INFO) {
}

AuthenticationInfo::AuthenticationInfo(const AuthenticationInfo &other)
  : Header(other), has_auth_params(other) {
}

AuthenticationInfo::~AuthenticationInfo() {
}

AuthenticationInfo *AuthenticationInfo::DoClone() const {
  return new AuthenticationInfo(*this);
}

void AuthenticationInfo::print(raw_ostream &os) const {
  Header::print(os);
  has_auth_params::print(os);
}

} // End of sippet namespace
