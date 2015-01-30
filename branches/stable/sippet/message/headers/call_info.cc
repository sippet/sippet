// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/call_info.h"

namespace sippet {

Info::Info() {
}

Info::Info(const Info &other)
  : has_parameters(other), single_value(other) {
}

Info::Info(const single_value::value_type &type)
  : single_value(type) {
}

Info::~Info() {
}

Info &Info::operator=(const Info &other) {
  single_value::operator=(other);
  has_parameters::operator=(other);
  return *this;
}

void Info::print(raw_ostream &os) const {
  os << "<" << value().spec() << ">";
  has_parameters::print(os);
}

CallInfo::CallInfo()
  : Header(Header::HDR_CALL_INFO) {
}

CallInfo::CallInfo(const CallInfo &other)
  : Header(other), has_multiple(other) {
}

CallInfo::~CallInfo() {
}

CallInfo *CallInfo::DoClone() const {
  return new CallInfo(*this);
}

void CallInfo::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
