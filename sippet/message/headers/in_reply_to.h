// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_IN_REPLY_TO_H_
#define SIPPET_MESSAGE_HEADERS_IN_REPLY_TO_H_

#include <string>

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class InReplyTo :
  public Header,
  public has_multiple<std::string> {
 private:
  DISALLOW_ASSIGN(InReplyTo);
  InReplyTo(const InReplyTo &other);
  InReplyTo *DoClone() const override;

 public:
  InReplyTo();
  ~InReplyTo() override;

  std::unique_ptr<InReplyTo> Clone() const {
    return std::unique_ptr<InReplyTo>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_IN_REPLY_TO_H_
