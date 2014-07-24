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
  ContentEncoding(const ContentEncoding &other);
  virtual ContentEncoding *DoClone() const OVERRIDE;

 public:
  ContentEncoding();
  ContentEncoding(const std::string &encoding);

  scoped_ptr<ContentEncoding> Clone() const {
    return scoped_ptr<ContentEncoding>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_ENCODING_H_
