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
  MinExpires(const MinExpires &other);
  MinExpires *DoClone() const override;

 public:
  MinExpires();
  MinExpires(const single_value::value_type &seconds);
  ~MinExpires() override;

  scoped_ptr<MinExpires> Clone() const {
    return scoped_ptr<MinExpires>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_MIN_EXPIRES_H_
