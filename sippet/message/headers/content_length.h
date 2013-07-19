// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_LENGTH_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_LENGTH_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class ContentLength :
  public Header,
  public single_value<unsigned> {
private:
  DISALLOW_ASSIGN(ContentLength);
  ContentLength(const ContentLength &other) : Header(other), single_value(other) {}
  virtual ContentLength *DoClone() const OVERRIDE {
    return new ContentLength(*this);
  }
public:
  ContentLength() : Header(Header::HDR_CONTENT_LENGTH) {}
  ContentLength(const single_value::value_type &length)
    : Header(Header::HDR_CONTENT_LENGTH), single_value(length) {}

  scoped_ptr<ContentLength> Clone() const {
    return scoped_ptr<ContentLength>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_LENGTH_H_
