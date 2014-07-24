// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/reply_to.h"

namespace sippet {

ReplyTo::ReplyTo()
  : Header(Header::HDR_REPLY_TO) {
}

ReplyTo::ReplyTo(const GURL &address, const std::string &displayName)
  : Header(Header::HDR_REPLY_TO), ContactBase(address, displayName) {
}

ReplyTo::ReplyTo(const ReplyTo &other)
  : Header(other), ContactBase(other) {
}

ReplyTo *ReplyTo::DoClone() const {
  return new ReplyTo(*this);
}

void ReplyTo::print(raw_ostream &os) const {
  Header::print(os);
  ContactBase::print(os);
}

} // End of sippet namespace
