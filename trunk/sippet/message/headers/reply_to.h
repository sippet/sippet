// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_REPLY_TO_H_
#define SIPPET_MESSAGE_HEADERS_REPLY_TO_H_

#include "sippet/message/headers/contact.h"

namespace sippet {

class ReplyTo :
  public Header,
  public ContactBase {
 private:
  DISALLOW_ASSIGN(ReplyTo);
  ReplyTo(const ReplyTo &other);
  virtual ReplyTo *DoClone() const OVERRIDE;

 public:
  ReplyTo();
  ReplyTo(const GURL &address, const std::string &displayName="");

  scoped_ptr<ReplyTo> Clone() const {
    return scoped_ptr<ReplyTo>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_REPLY_TO_H_
