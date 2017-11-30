// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/subject.h"

namespace sippet {

Subject::Subject()
  : Header(Header::HDR_SUBJECT) {
}

Subject::Subject(const single_value::value_type &subject)
  : Header(Header::HDR_SUBJECT), single_value(subject) {
}

Subject::Subject(const Subject &other)
  : Header(other), single_value(other) {
}

Subject::~Subject() {
}

Subject *Subject::DoClone() const {
  return new Subject(*this);
}

void Subject::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

}  // namespace sippet
