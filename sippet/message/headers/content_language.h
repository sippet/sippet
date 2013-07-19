// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_LANGUAGE_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_LANGUAGE_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class ContentLanguage :
  public Header,
  public has_multiple<std::string> {
private:
  DISALLOW_ASSIGN(ContentLanguage);
  ContentLanguage(const ContentLanguage &other)
    : Header(other), has_multiple(other) {}
  virtual ContentLanguage *DoClone() const OVERRIDE {
    return new ContentLanguage(*this);
  }
public:
  ContentLanguage()
    : Header(Header::HDR_CONTENT_LANGUAGE) {}
  ContentLanguage(const std::string &language)
    : Header(Header::HDR_CONTENT_LANGUAGE) {
    push_back(language);
  }

  scoped_ptr<ContentLanguage> Clone() const {
    return scoped_ptr<ContentLanguage>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_LANGUAGE_H_
