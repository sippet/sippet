// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/accept.h"

namespace sippet {

MediaRange::MediaRange() {
}

MediaRange::MediaRange(const MediaRange &other)
  : MediaType(other) {
}

MediaRange::MediaRange(const std::string &type, const std::string &subtype)
  : MediaType(type, subtype) {
}

MediaRange::~MediaRange() {
}

MediaRange &MediaRange::operator=(const MediaRange &other) {
  MediaType::operator=(other);
  return *this;
}

Accept::Accept()
  : Header(Header::HDR_ACCEPT) {
}

Accept::Accept(const Accept &other)
  : Header(other), has_multiple(other) {
}

Accept::~Accept() {
}

Accept *Accept::DoClone() const {
  return new Accept(*this);
}

void Accept::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
