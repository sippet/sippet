// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_METHOD_H_
#define SIPPET_MESSAGE_METHOD_H_

#include "sippet/message/atom.h"
#include "base/strings/string_util.h"

namespace sippet {

namespace details {
struct Method {
  enum Type {
#define SIP_METHOD(method) method,
#include "sippet/message/method_list.h"
#undef SIP_METHOD
    Unknown
  };
};
} // End of details namespace

template<>
struct AtomTraits<details::Method> {
  typedef details::Method::Type type;
  static const type unknown_type = details::Method::Unknown;
  static const char *string_of(type t);
  static type coerce(const char *str);
};

typedef Atom<details::Method> Method;

} // End of sippet namespace

#endif // SIPPET_MESSAGE_METHOD_H_
