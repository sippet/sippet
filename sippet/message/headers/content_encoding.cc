// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/content_encoding.h"

namespace sippet {

ContentEncoding::ContentEncoding()
  : Header(Header::HDR_CONTENT_ENCODING) {
}

ContentEncoding::ContentEncoding(const std::string &encoding)
  : Header(Header::HDR_CONTENT_ENCODING) {
  push_back(encoding);
}

ContentEncoding::ContentEncoding(const ContentEncoding &other)
  : Header(other), has_multiple(other) {
}

ContentEncoding::~ContentEncoding() {
}

ContentEncoding *ContentEncoding::DoClone() const {
  return new ContentEncoding(*this);
}

void ContentEncoding::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
