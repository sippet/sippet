// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/content_disposition.h"

namespace sippet {

ContentDisposition::ContentDisposition()
  : Header(Header::HDR_CONTENT_DISPOSITION) {
}

ContentDisposition::ContentDisposition(Type t)
  : Header(Header::HDR_CONTENT_DISPOSITION) {
  set_value(t);
}

ContentDisposition::ContentDisposition(const single_value::value_type &value)
  : Header(Header::HDR_CONTENT_DISPOSITION), single_value(value) {
}

ContentDisposition::ContentDisposition(const ContentDisposition &other)
  : Header(other), single_value(other), has_parameters(other) {
}

ContentDisposition::~ContentDisposition() {
}

ContentDisposition *ContentDisposition::DoClone() const {
  return new ContentDisposition(*this);
}

void ContentDisposition::print(raw_ostream &os) const {
  Header::print(os);
  os << value();
  has_parameters::print(os);
}

}  // namespace sippet
