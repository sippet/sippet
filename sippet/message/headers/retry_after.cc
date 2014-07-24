// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/retry_after.h"

namespace sippet {

RetryAfter::RetryAfter()
  : Header(Header::HDR_RETRY_AFTER) {
}

RetryAfter::RetryAfter(single_value::value_type seconds)
  : Header(Header::HDR_RETRY_AFTER), single_value(seconds) {
}

RetryAfter::RetryAfter(const RetryAfter &other)
  : Header(other), single_value(other), has_parameters(other) {
}

RetryAfter::~RetryAfter() {
}

RetryAfter *RetryAfter::DoClone() const {
  return new RetryAfter(*this);
}

void RetryAfter::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
  has_parameters::print(os);
}

} // End of sippet namespace
