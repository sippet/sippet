// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/min_expires.h"

namespace sippet {

MinExpires::MinExpires()
  : Header(Header::HDR_MIN_EXPIRES) {
}

MinExpires::MinExpires(const single_value::value_type &seconds)
  : Header(Header::HDR_MIN_EXPIRES), single_value(seconds) {
}

MinExpires::MinExpires(const MinExpires &other)
  : Header(other), single_value(other) {
}

MinExpires::~MinExpires() {
}

MinExpires *MinExpires::DoClone() const {
  return new MinExpires(*this);
}

void MinExpires::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

}  // namespace sippet
