// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_RETRY_AFTER_H_
#define SIPPET_MESSAGE_HEADERS_RETRY_AFTER_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class RetryAfter :
  public Header,
  public single_value<unsigned>,
  public has_parameters {
private:
  DISALLOW_ASSIGN(RetryAfter);
  RetryAfter(const RetryAfter &other)
    : Header(other), single_value(other), has_parameters(other) {}
  virtual RetryAfter *DoClone() const OVERRIDE {
    return new RetryAfter(*this);
  }
public:
  RetryAfter() : Header(Header::HDR_RETRY_AFTER) {}
  RetryAfter(single_value::value_type seconds)
    : Header(Header::HDR_RETRY_AFTER), single_value(seconds) {}

  scoped_ptr<RetryAfter> Clone() const {
    return scoped_ptr<RetryAfter>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    single_value::print(os);
    has_parameters::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_RETRY_AFTER_H_
