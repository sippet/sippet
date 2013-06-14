// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ACCEPT_H_
#define SIPPET_MESSAGE_HEADERS_ACCEPT_H_

#include "sippet/message/headers/content_type.h"
#include "sippet/message/headers/bits/has_multiple.h"

namespace sippet {

class MediaRange :
  public MediaType {
public:
  MediaRange() {}
  MediaRange(const MediaRange &other)
    : MediaType(other) {}
  MediaRange(const std::string &type, const std::string &subtype)
    : MediaType(type, subtype) {}
  ~MediaRange() {}

  MediaRange &operator=(const MediaRange &other) {
    MediaType::operator=(other);
    return *this;
  }

  bool AllowsAll() { return type() == "*" && AllowsAllSubtypes(); }
  bool AllowsAllSubtypes() { return subtype() == "*"; }
};

inline
raw_ostream &operator << (raw_ostream &os, const MediaRange &m) {
  m.print(os);
  return os;
}

class Accept :
  public Header,
  public has_multiple<MediaRange> {
private:
  DISALLOW_ASSIGN(Accept);
  Accept(const Accept &other) : Header(other), has_multiple(other) {}
  virtual Accept *DoClone() const {
    return new Accept(*this);
  }
public:
  Accept() : Header(Header::HDR_ACCEPT) {}

  scoped_ptr<Accept> Clone() const {
    return scoped_ptr<Accept>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Accept");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_H_
