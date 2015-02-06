// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_REQUIRE_H_
#define SIPPET_MESSAGE_HEADERS_REQUIRE_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Require :
  public Header,
  public has_multiple<std::string> {
 private:
  DISALLOW_ASSIGN(Require);
  Require(const Require &other);
  Require *DoClone() const override;

 public:
  Require();
  Require(const std::string &value);
  ~Require() override;

  scoped_ptr<Require> Clone() const {
    return scoped_ptr<Require>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_REQUIRE_H_
