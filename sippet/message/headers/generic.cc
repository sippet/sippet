// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/generic.h"

#include <string>

namespace sippet {

Generic::Generic()
  : Header(Header::HDR_GENERIC) {
}

Generic::Generic(const Generic &other)
  : Header(other), header_name_(other.header_name_),
    header_value_(other.header_value_) {
}

Generic::~Generic() {
}

Generic *Generic::DoClone() const {
  return new Generic(*this);
}

Generic::Generic(const std::string &header_name,
                 const std::string &header_value)
  : Header(Header::HDR_GENERIC), header_name_(header_name),
    header_value_(header_value) {
}

void Generic::print(raw_ostream &os) const {
  Header::print(os);
  os << header_value_;
}

}  // namespace sippet
