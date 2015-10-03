// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/user_agent.h"

namespace sippet {

UserAgent::UserAgent()
  : Header(Header::HDR_USER_AGENT) {
}

UserAgent::UserAgent(const single_value::value_type &subject)
  : Header(Header::HDR_USER_AGENT), single_value(subject) {
}

UserAgent::UserAgent(const UserAgent &other)
  : Header(other), single_value(other) {
}

UserAgent::~UserAgent() {
}

UserAgent *UserAgent::DoClone() const {
  return new UserAgent(*this);
}

void UserAgent::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

}  // namespace sippet
