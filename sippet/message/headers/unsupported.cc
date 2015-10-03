// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/unsupported.h"

namespace sippet {

Unsupported::Unsupported()
  : Header(Header::HDR_UNSUPPORTED) {
}

Unsupported::Unsupported(const std::string &value)
  : Header(Header::HDR_UNSUPPORTED) {
  push_back(value);
}

Unsupported::Unsupported(const Unsupported &other)
  : Header(other), has_multiple(other) {
}

Unsupported::~Unsupported() {
}

Unsupported *Unsupported::DoClone() const {
  return new Unsupported(*this);
}

void Unsupported::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
