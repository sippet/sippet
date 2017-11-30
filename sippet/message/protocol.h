// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_PROTOCOL_H_
#define SIPPET_MESSAGE_PROTOCOL_H_

#include "sippet/message/atom.h"
#include "base/strings/string_util.h"

namespace sippet {

namespace details {
struct Protocol {
  enum Type {
#define SIP_PROTOCOL(protocol) protocol,
#include "sippet/message/protocol_list.h"
#undef SIP_PROTOCOL
    Unknown
  };
};
} // End of details namespace

template<>
struct AtomTraits<details::Protocol> {
  typedef details::Protocol::Type type;
  static const type unknown_type = details::Protocol::Unknown;
  static const char *string_of(type t);
  static type coerce(const char *str);
};

typedef Atom<details::Protocol> Protocol;
typedef AtomLess<details::Protocol> ProtocolLess;

} // End of sippet namespace

#endif // SIPPET_MESSAGE_PROTOCOL_H_
