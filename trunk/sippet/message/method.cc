// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/method.h"

#include <cstring>
#include <algorithm>

#include "base/string_util.h"

namespace sippet {

namespace {
  const char *names[] = {
#define X(method) \
  #method,
  #include "sippet/message/known_methods.h"
#undef X
    "", // for Unknown
  };

  bool string_less(const char *a, const char *b) {
    return base::strcasecmp(a, b) < 0;
  }
}

const char *AtomTraits<Method>::string_of(type t) {
  return names[static_cast<int>(t)];
}

AtomTraits<Method>::type
AtomTraits<Method>::coerce(const char *str) {
  Method::Type type = Method::Unknown;
  const char **first = names;
  const char **last = names + ARRAYSIZE(names) - 1; // don't include the last
  const char **found = std::lower_bound(first, last, str, string_less);
  if (found != last
      && base::strcasecmp(*found, str) == 0) {
    type = static_cast<Method::Type>(found - first);
  }
  return type;
}

} // End of sippet namespace
