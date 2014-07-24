// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/require.h"

namespace sippet {

Require::Require()
  : Header(Header::HDR_REQUIRE) {
}

Require::Require(const std::string &value)
  : Header(Header::HDR_REQUIRE) {
  push_back(value);
}

Require::Require(const Require &other)
  : Header(other), has_multiple(other) {
}

Require::~Require() {
}

Require *Require::DoClone() const {
  return new Require(*this);
}

void Require::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
