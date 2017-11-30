// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_TO_H_
#define SIPPET_MESSAGE_HEADERS_TO_H_

#include "sippet/message/headers/contact.h"

namespace sippet {

class To :
  public Header,
  public ContactBase,
  public has_tag<To> {
 private:
  DISALLOW_ASSIGN(To);
  To(const To &other);
  To *DoClone() const override;

 public:
  To();
  To(const GURL &address, const std::string &displayName="");
  ~To() override;

  std::unique_ptr<To> Clone() const {
    return std::unique_ptr<To>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_TO_H_
