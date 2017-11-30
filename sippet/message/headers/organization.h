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
  Organization(const Organization &other);
  Organization *DoClone() const override;

 public:
  Organization();
  Organization(const single_value::value_type &organization);
  ~Organization() override;

  std::unique_ptr<Organization> Clone() const {
    return std::unique_ptr<Organization>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ORGANIZATION_H_
