// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/reason.h"

namespace sippet {

const char* Reason::kProtoSIP = "SIP";
const char* Reason::kQ850 = "Q.850";

Reason::Reason()
  : Header(Header::HDR_REASON) {
}

Reason::Reason(const single_value::value_type &value)
  : Header(Header::HDR_REASON), single_value(value) {
}

Reason::Reason(StatusCode status_code, const std::string& text)
  : Header(Header::HDR_REASON) {
  set_value(kProtoSIP);
  set_cause(static_cast<int>(status_code));
  if (text.size() > 0)
    set_text(text);
  else
    set_text(GetReasonPhrase(status_code));
}

Reason::Reason(const std::string& protocol, int cause,
    const std::string& text)
  : Header(Header::HDR_REASON) {
  set_value(protocol);
  set_cause(cause);
  set_text(text);
}

Reason::Reason(const Reason &other)
  : Header(other), single_value(other), has_parameters(other) {
}

Reason::~Reason() {
}

Reason *Reason::DoClone() const {
  return new Reason(*this);
}

void Reason::print(raw_ostream &os) const {
  Header::print(os);
  os << value();
  has_parameters::print(os);
}

} // End of sippet namespace
