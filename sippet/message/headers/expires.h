// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_EXPIRES_H_
#define SIPPET_MESSAGE_HEADERS_EXPIRES_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Expires :
  public Header,
  public single_value<unsigned> {
 private:
  DISALLOW_ASSIGN(Expires);
  Expires(const Expires &other);
  virtual Expires *DoClone() const OVERRIDE;

 public:
  Expires();
  Expires(const single_value::value_type &seconds);

  scoped_ptr<Expires> Clone() const {
    return scoped_ptr<Expires>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_EXPIRES_H_
