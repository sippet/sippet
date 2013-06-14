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
  To(const To &other)
    : Header(other), ContactBase(other) {}
  virtual To *DoClone() const {
    return new To(*this);
  }
public:
  To() : Header(Header::HDR_TO) {}
  To(const GURL &address, const std::string &displayName="")
    : Header(Header::HDR_TO), ContactBase(address, displayName) {}

  scoped_ptr<To> Clone() const {
    return scoped_ptr<To>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("To");
    ContactBase::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_TO_H_
