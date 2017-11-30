// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_UNSUPPORTED_H_
#define SIPPET_MESSAGE_HEADERS_UNSUPPORTED_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Unsupported :
  public Header,
  public has_multiple<std::string> {
 private:
  DISALLOW_ASSIGN(Unsupported);
  Unsupported(const Unsupported &other);
  Unsupported *DoClone() const override;

 public:
  Unsupported();
  Unsupported(const std::string &value);
  ~Unsupported() override;

  std::unique_ptr<Unsupported> Clone() const {
    return std::unique_ptr<Unsupported>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_UNSUPPORTED_H_
