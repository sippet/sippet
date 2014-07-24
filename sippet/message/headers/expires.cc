// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/expires.h"

namespace sippet {

Expires::Expires()
  : Header(Header::HDR_EXPIRES) {
}

Expires::Expires(const single_value::value_type &seconds)
  : Header(Header::HDR_EXPIRES), single_value(seconds) {
}

Expires::Expires(const Expires &other)
  : Header(other), single_value(other) {
}

Expires::~Expires() {
}

Expires *Expires::DoClone() const {
  return new Expires(*this);
}

void Expires::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

} // End of sippet namespace
