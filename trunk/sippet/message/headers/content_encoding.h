// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_ENCODING_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_ENCODING_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class ContentEncoding :
  public Header,
  public has_multiple<single_value<std::string> > {
private:
  DISALLOW_ASSIGN(ContentEncoding);
  ContentEncoding(const ContentEncoding &other)
    : Header(other), has_multiple(other) {}
  virtual ContentEncoding *DoClone() const OVERRIDE {
    return new ContentEncoding(*this);
  }
public:
  ContentEncoding() : Header(Header::HDR_CONTENT_ENCODING) {}
  ContentEncoding(const std::string &encoding)
    : Header(Header::HDR_CONTENT_ENCODING) {
    push_back(encoding);
  }

  scoped_ptr<ContentEncoding> Clone() const {
    return scoped_ptr<ContentEncoding>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_ENCODING_H_
