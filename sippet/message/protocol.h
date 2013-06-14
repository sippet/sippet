// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_PROTOCOL_H_
#define SIPPET_MESSAGE_PROTOCOL_H_

#include "sippet/message/atom.h"
#include "base/string_util.h"

namespace sippet {

struct Protocol {
  enum Type {
#define X(protocol) protocol,
#include "sippet/message/known_protocols.h"
#undef X
    Unknown
  };
};

template<>
struct AtomTraits<Protocol> {
  typedef Protocol::Type type;
  static const type unknown_type = Protocol::Unknown;
  static const char *string_of(type t);
  static type coerce(const char *str);
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_PROTOCOL_H_
