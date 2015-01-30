// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/call_id.h"

namespace sippet {

CallId::CallId()
  : Header(Header::HDR_CALL_ID) {
}

CallId::CallId(const single_value::value_type &callid)
  : Header(Header::HDR_CALL_ID), single_value(callid) {
}

CallId::CallId(const CallId &other)
  : Header(other), single_value(other) {
}

CallId::~CallId() {
}

CallId *CallId::DoClone() const {
  return new CallId(*this);
}

void CallId::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

} // End of sippet namespace
