// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/content_language.h"

namespace sippet {

ContentLanguage::ContentLanguage()
  : Header(Header::HDR_CONTENT_LANGUAGE) {
}

ContentLanguage::ContentLanguage(const std::string &language)
  : Header(Header::HDR_CONTENT_LANGUAGE) {
  push_back(language);
}

ContentLanguage::ContentLanguage(const ContentLanguage &other)
  : Header(other), has_multiple(other) {
}

ContentLanguage *ContentLanguage::DoClone() const {
  return new ContentLanguage(*this);
}

void ContentLanguage::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
