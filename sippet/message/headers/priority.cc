// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/priority.h"

namespace sippet {

Priority::Priority()
  : Header(Header::HDR_PRIORITY) {
}

Priority::Priority(Level l)
  : Header(Header::HDR_PRIORITY) {
  set_value(l);
}

Priority::Priority(const single_value::value_type &priority)
  : Header(Header::HDR_PRIORITY), single_value(priority) {
}

Priority::Priority(const Priority &other)
  : Header(other), single_value(other) {
}

Priority::~Priority() {
}

Priority *Priority::DoClone() const {
  return new Priority(*this);
}

void Priority::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

}  // namespace sippet
