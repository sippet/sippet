// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_METHOD_H_
#define SIPPET_MESSAGE_METHOD_H_

#include "sippet/message/atom.h"
#include "base/string_util.h"

namespace sippet {

struct Method {
  enum Type {
#define X(method) method,
#include "sippet/message/known_methods.h"
#undef X
    Unknown
  };
};

template<>
struct AtomTraits<Method> {
  typedef Method::Type type;
  static const type unknown_type = Method::Unknown;
  static const char *string_of(type t);
  static type coerce(const char *str);
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_METHOD_H_
