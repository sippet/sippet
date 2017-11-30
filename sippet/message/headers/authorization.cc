// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/authorization.h"

#include <string>

namespace sippet {

Credentials::Credentials() {
}

Credentials::Credentials(Scheme s)
  : has_auth_params(s) {
}

Credentials::Credentials(const std::string &scheme)
  : has_auth_params(scheme) {
}

Credentials::Credentials(const Credentials &other)
  : has_auth_params(other) {
}

Credentials::~Credentials() {
}

Authorization::Authorization()
  : Header(Header::HDR_AUTHORIZATION) {
}

Authorization::Authorization(Scheme s)
  : Header(Header::HDR_AUTHORIZATION), Credentials(s) {
}

Authorization::Authorization(const std::string &scheme)
  : Header(Header::HDR_AUTHORIZATION), Credentials(scheme) {
}

Authorization::Authorization(const Authorization &other)
  : Header(other), Credentials(other) {
}

Authorization::~Authorization() {
}

Authorization *Authorization::DoClone() const {
  return new Authorization(*this);
}

void Authorization::print(raw_ostream &os) const {
  Header::print(os);
  Credentials::print(os);
}

}  // namespace sippet
