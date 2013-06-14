// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_MIN_EXPIRES_H_
#define SIPPET_MESSAGE_HEADERS_MIN_EXPIRES_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class MinExpires :
  public Header,
  public single_value<unsigned> {
private:
  DISALLOW_ASSIGN(MinExpires);
  MinExpires(const MinExpires &other) : Header(other), single_value(other) {}
  virtual MinExpires *DoClone() const {
    return new MinExpires(*this);
  }
public:
  MinExpires() : Header(Header::HDR_MIN_EXPIRES) {}
  MinExpires(const single_value::value_type &seconds)
    : Header(Header::HDR_MIN_EXPIRES), single_value(seconds) {}

  scoped_ptr<MinExpires> Clone() const {
    return scoped_ptr<MinExpires>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Min-Expires");
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_MIN_EXPIRES_H_
