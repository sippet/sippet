// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ALLOW_H_
#define SIPPET_MESSAGE_HEADERS_ALLOW_H_

#include <string>

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/method.h"

namespace sippet {

class Allow :
  public Header,
  public has_multiple<Method> {
private:
  DISALLOW_ASSIGN(Allow);
  Allow(const Allow &other);
  Allow *DoClone() const override;

public:
  Allow();
  ~Allow() override;

  scoped_ptr<Allow> Clone() const {
    return scoped_ptr<Allow>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ALLOW_H_
