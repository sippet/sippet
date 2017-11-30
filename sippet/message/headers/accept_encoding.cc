// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/accept_encoding.h"
#include "base/strings/string_util.h"

namespace sippet {

Encoding::Encoding() {
}

Encoding::Encoding(const Encoding &other)
  : single_value(other), has_parameters(other) {
}

Encoding::Encoding(const single_value::value_type &value)
  : single_value(base::ToLowerASCII(value)) {
}

Encoding &Encoding::operator=(const Encoding &other) {
  single_value::operator=(other);
  has_parameters::operator=(other);
  return *this;
}

Encoding::~Encoding() {
}

void Encoding::print(raw_ostream &os) const {
  os << value();
  has_parameters::print(os);
}

AcceptEncoding::AcceptEncoding()
  : Header(Header::HDR_ACCEPT_ENCODING) {
}

AcceptEncoding::AcceptEncoding(const AcceptEncoding &other)
  : Header(other), has_multiple(other) {
}

AcceptEncoding::~AcceptEncoding() {
}

AcceptEncoding *AcceptEncoding::DoClone() const {
  return new AcceptEncoding(*this);
}

void AcceptEncoding::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
