// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/supported.h"

namespace sippet {

Supported::Supported()
  : Header(Header::HDR_SUPPORTED) {
}

Supported::Supported(const std::string &value)
  : Header(Header::HDR_SUPPORTED) {
  push_back(value);
}

Supported::Supported(const Supported &other)
  : Header(other), has_multiple(other) {
}

Supported::~Supported() {
}

Supported *Supported::DoClone() const {
  return new Supported(*this);
}

void Supported::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
