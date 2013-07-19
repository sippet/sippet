// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_MAX_FORWARDS_H_
#define SIPPET_MESSAGE_HEADERS_MAX_FORWARDS_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class MaxForwards :
  public Header,
  public single_value<unsigned> {
private:
  DISALLOW_ASSIGN(MaxForwards);
  MaxForwards(const MaxForwards &other) : Header(other), single_value(other) {}
  virtual MaxForwards *DoClone() const OVERRIDE {
    return new MaxForwards(*this);
  }
public:
  MaxForwards() : Header(Header::HDR_MAX_FORWARDS) {}
  MaxForwards(const single_value::value_type &n)
    : Header(Header::HDR_MAX_FORWARDS), single_value(n) {}

  scoped_ptr<MaxForwards> Clone() const {
    return scoped_ptr<MaxForwards>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_MAX_FORWARDS_H_
