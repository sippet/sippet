// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/error_info.h"

namespace sippet {

ErrorUri::ErrorUri() {
}

ErrorUri::ErrorUri(const ErrorUri &other)
  : single_value(other), has_parameters(other) {
}

ErrorUri::ErrorUri(const single_value::value_type &type)
  : single_value(type) {
}

ErrorUri::~ErrorUri() {
}

ErrorUri &ErrorUri::operator=(const ErrorUri &other) {
  single_value::operator=(other);
  has_parameters::operator=(other);
  return *this;
}

void ErrorUri::print(raw_ostream &os) const {
  os << "<" << value().spec() << ">";
  has_parameters::print(os);
}

ErrorInfo::ErrorInfo()
  : Header(Header::HDR_ERROR_INFO) {
}

ErrorInfo::ErrorInfo(const ErrorInfo &other)
  : Header(other), has_multiple(other) {
}

ErrorInfo::~ErrorInfo() {
}

ErrorInfo *ErrorInfo::DoClone() const {
  return new ErrorInfo(*this);
}

void ErrorInfo::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
