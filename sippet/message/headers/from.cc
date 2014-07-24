// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/from.h"

namespace sippet {

From::From()
  : Header(Header::HDR_FROM) {
}

From::From(const GURL &address, const std::string &displayName)
  : Header(Header::HDR_FROM), ContactBase(address, displayName) {
}

From::From(const From &other)
  : Header(other), ContactBase(other) {
}

From::~From() {
}

From *From::DoClone() const {
  return new From(*this);
}

void From::print(raw_ostream &os) const {
  Header::print(os);
  ContactBase::print(os);
}

} // End of sippet namespace
