// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/bits/has_parameters.h"

namespace sippet {

has_parameters::has_parameters() {
}

has_parameters::~has_parameters() {
}

has_parameters::has_parameters(const has_parameters &other)
  : params_(other.params_) {
}

has_parameters &has_parameters::operator=(const has_parameters &other) {
  params_ = other.params_;
  return *this;
}

} // namespace sippet
