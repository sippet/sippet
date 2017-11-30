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
  MediaRange();
  MediaRange(const MediaRange &other);
  MediaRange(const std::string &type, const std::string &subtype);
  ~MediaRange();

  MediaRange &operator=(const MediaRange &other);

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
  Accept(const Accept &other);
  Accept *DoClone() const override;

 public:
  Accept();
  ~Accept() override;

  std::unique_ptr<Accept> Clone() const {
    return std::unique_ptr<Accept>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_H_
