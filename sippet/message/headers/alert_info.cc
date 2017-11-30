// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/alert_info.h"

namespace sippet {

AlertParam::AlertParam() {
}

AlertParam::AlertParam(const AlertParam &other)
  : single_value(other), has_parameters(other) {
}

AlertParam::AlertParam(const single_value::value_type &type)
  : single_value(type) {
}

AlertParam::~AlertParam() {
}

AlertParam &AlertParam::operator=(const AlertParam &other) {
  single_value::operator=(other);
  has_parameters::operator=(other);
  return *this;
}

void AlertParam::print(raw_ostream &os) const {
  os << "<" << value().spec() << ">";
  has_parameters::print(os);
}

AlertInfo::AlertInfo()
  : Header(Header::HDR_ALERT_INFO) {
}

AlertInfo::AlertInfo(const AlertInfo &other)
  : Header(other), has_multiple(other) {
}

AlertInfo::~AlertInfo() {
}

AlertInfo *AlertInfo::DoClone() const {
  return new AlertInfo(*this);
}

void AlertInfo::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
