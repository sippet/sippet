// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/to.h"

#include <string>

namespace sippet {

To::To()
  : Header(Header::HDR_TO) {
}

To::To(const GURL &address, const std::string &displayName)
  : Header(Header::HDR_TO), ContactBase(address, displayName) {
}

To::To(const To &other)
  : Header(other), ContactBase(other) {
}

To::~To() {
}

To *To::DoClone() const {
  return new To(*this);
}

void To::print(raw_ostream &os) const {
  Header::print(os);
  ContactBase::print(os);
}

}  // namespace sippet
