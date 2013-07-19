// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ORGANIZATION_H_
#define SIPPET_MESSAGE_HEADERS_ORGANIZATION_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Organization :
  public Header,
  public single_value<std::string> {
private:
  DISALLOW_ASSIGN(Organization);
  Organization(const Organization &other) : Header(other), single_value(other) {}
  virtual Organization *DoClone() const OVERRIDE {
    return new Organization(*this);
  }
public:
  Organization() : Header(Header::HDR_ORGANIZATION) {}
  Organization(const single_value::value_type &organization)
    : Header(Header::HDR_ORGANIZATION), single_value(organization) {}

  scoped_ptr<Organization> Clone() const {
    return scoped_ptr<Organization>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ORGANIZATION_H_
