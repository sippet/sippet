// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/in_reply_to.h"

namespace sippet {

InReplyTo::InReplyTo()
  : Header(Header::HDR_IN_REPLY_TO) {
}

InReplyTo::InReplyTo(const InReplyTo &other)
  : Header(other), has_multiple(other) {
}

InReplyTo::~InReplyTo() {
}

InReplyTo *InReplyTo::DoClone() const {
  return new InReplyTo(*this);
}

void InReplyTo::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
