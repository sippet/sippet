// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/warning.h"

#include <string>

namespace sippet {

WarnParam::WarnParam()
  : warn_code_(0) {
}

WarnParam::WarnParam(const WarnParam &other)
  : warn_code_(other.warn_code_), warn_agent_(other.warn_agent_),
    warn_text_(other.warn_text_) {
}

WarnParam::WarnParam(unsigned warn_code,
                     const std::string &warn_agent,
                     const std::string &warn_text)
  : warn_code_(warn_code), warn_agent_(warn_agent),
    warn_text_(warn_text) {
}

WarnParam::~WarnParam() {
}

WarnParam &WarnParam::operator=(const WarnParam &other) {
  warn_code_ = other.warn_code_;
  warn_agent_ = other.warn_agent_;
  warn_text_ = other.warn_text_;
  return *this;
}

void WarnParam::print(raw_ostream &os) const {
  os << warn_code_ << " " << warn_agent_ << " \"" << warn_text_ << "\"";
}

Warning::Warning()
  : Header(Header::HDR_WARNING) {
}

Warning::Warning(const WarnParam &param)
  : Header(Header::HDR_WARNING) {
  push_back(param);
}

Warning::Warning(const Warning &other)
  : Header(other), has_multiple(other) {
}

Warning::~Warning() {
}

Warning *Warning::DoClone() const {
  return new Warning(*this);
}

void Warning::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
