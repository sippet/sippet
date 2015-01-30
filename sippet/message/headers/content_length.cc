// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/content_length.h"

namespace sippet {

ContentLength::ContentLength()
  : Header(Header::HDR_CONTENT_LENGTH) {
}

ContentLength::ContentLength(const single_value::value_type &length)
  : Header(Header::HDR_CONTENT_LENGTH), single_value(length) {
}

ContentLength::ContentLength(const ContentLength &other)
  : Header(other), single_value(other) {
}

ContentLength::~ContentLength() {
}

ContentLength *ContentLength::DoClone() const {
  return new ContentLength(*this);
}

void ContentLength::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

} // End of sippet namespace
