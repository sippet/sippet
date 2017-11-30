// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_SUPPORTED_H_
#define SIPPET_MESSAGE_HEADERS_SUPPORTED_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Supported :
  public Header,
  public has_multiple<std::string> {
 private:
  DISALLOW_ASSIGN(Supported);
  Supported(const Supported &other);
  Supported *DoClone() const override;

 public:
  Supported();
  Supported(const std::string &value);
  ~Supported() override;

  std::unique_ptr<Supported> Clone() const {
    return std::unique_ptr<Supported>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_SUPPORTED_H_
