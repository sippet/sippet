// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/organization.h"

namespace sippet {

Organization::Organization()
  : Header(Header::HDR_ORGANIZATION) {
}

Organization::Organization(const single_value::value_type &organization)
  : Header(Header::HDR_ORGANIZATION), single_value(organization) {
}

Organization::Organization(const Organization &other)
  : Header(other), single_value(other) {
}

Organization::~Organization() {
}

Organization *Organization::DoClone() const {
  return new Organization(*this);
}

void Organization::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

}  // namespace sippet
