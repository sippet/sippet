// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/max_forwards.h"

namespace sippet {

MaxForwards::MaxForwards()
  : Header(Header::HDR_MAX_FORWARDS) {
}

MaxForwards::MaxForwards(const single_value::value_type &n)
  : Header(Header::HDR_MAX_FORWARDS), single_value(n) {
}

MaxForwards::MaxForwards(const MaxForwards &other)
  : Header(other), single_value(other) {
}

MaxForwards::~MaxForwards() {
}

MaxForwards *MaxForwards::DoClone() const {
  return new MaxForwards(*this);
}

void MaxForwards::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

} // End of sippet namespace
