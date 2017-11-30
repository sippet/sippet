// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/www_authenticate.h"

#include <string>

namespace sippet {

Challenge::Challenge() {
}

Challenge::Challenge(Scheme s)
  : has_auth_params(s) {
}

Challenge::Challenge(const std::string &scheme)
  : has_auth_params(scheme) {
}

Challenge::Challenge(const Challenge &other)
  : has_auth_params(other) {
}

Challenge::~Challenge() {
}

std::string Challenge::ToString() {
  std::string result;
  raw_string_ostream os(result);
  has_auth_params::print(os);
  return os.str();
}

WwwAuthenticate::WwwAuthenticate()
  : Header(Header::HDR_WWW_AUTHENTICATE) {
}

WwwAuthenticate::WwwAuthenticate(Scheme s)
  : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(s) {
}

WwwAuthenticate::WwwAuthenticate(const std::string &scheme)
  : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(scheme) {
}

WwwAuthenticate::WwwAuthenticate(const WwwAuthenticate &other)
  : Header(other), Challenge(other) {
}

WwwAuthenticate::~WwwAuthenticate() {
}

WwwAuthenticate *WwwAuthenticate::DoClone() const {
  return new WwwAuthenticate(*this);
}

void WwwAuthenticate::print(raw_ostream &os) const {
  Header::print(os);
  Challenge::print(os);
}

}  // namespace sippet
