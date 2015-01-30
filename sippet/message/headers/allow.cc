// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/allow.h"

namespace sippet {

Allow::Allow()
  : Header(Header::HDR_ALLOW) {
}

Allow::Allow(const Allow &other)
  : Header(other), has_multiple(other) {
}

Allow::~Allow() {
}

Allow *Allow::DoClone() const {
  return new Allow(*this);
}

void Allow::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
