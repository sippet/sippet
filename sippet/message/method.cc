// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/method.h"

#include <cstring>
#include <algorithm>

#include "base/strings/string_util.h"

namespace sippet {

namespace {
  const char *names[] = {
#define SIP_METHOD(method) \
  #method,
  #include "sippet/message/method_list.h"
#undef SIP_METHOD
    "",  // for Unknown
  };

  bool string_less(const char *a, const char *b) {
    return base::strcasecmp(a, b) < 0;
  }
}  // namespace

const char *AtomTraits<details::Method>::string_of(type t) {
  return names[static_cast<int>(t)];
}

AtomTraits<details::Method>::type
AtomTraits<details::Method>::coerce(const char *str) {
  details::Method::Type type = details::Method::Unknown;
  const char **first = names;
  const char **last = names + arraysize(names) - 1;  // don't include the last
  const char **found = std::lower_bound(first, last, str, string_less);
  if (found != last
      && base::strcasecmp(*found, str) == 0) {
    type = static_cast<details::Method::Type>(found - first);
  }
  return type;
}

}  // namespace sippet
