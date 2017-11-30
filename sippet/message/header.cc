// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/header.h"

#include <cstring>
#include <algorithm>

#include "base/strings/string_util.h"
#include "sippet/message/headers/generic.h"

namespace sippet {

namespace {
  static const char *names[] = {
#define X(class_name, compact_form, header_name, enum_name, format) \
    #header_name,
#include "sippet/message/header_list.h"
#undef X
  };

  static const char compact_forms[] = {
#define X(class_name, compact_form, header_name, enum_name, format) \
    compact_form,
#include "sippet/message/header_list.h"
#undef X
  };

  bool HeaderNameLess(const char *a, const char *b) {
    return base::CompareCaseInsensitiveASCII(a, b) < 0;
  }
}  // namespace

Header::Header(Type type)
  : type_(type) {
}

Header::Header(const Header &other)
  : type_(other.type_) {
}

Header::~Header() {
}

char Header::compact_form() const {
  return compact_forms[static_cast<int>(type_)];
}

void Header::print(raw_ostream &os) const {
  if (compact_form() != 0)
    os << static_cast<signed char>(compact_form());
  else
    os << names[static_cast<size_t>(type())];
  os << ": ";
}

const char *AtomTraits<Header::Type>::string_of(type t) {
  return names[static_cast<size_t>(t)];
}

AtomTraits<Header::Type>::type
AtomTraits<Header::Type>::coerce(const char *str) {
  Header::Type type = Header::HDR_GENERIC;
  if (strlen(str) == 1) {
    char h = tolower(str[0]);
    for (size_t i = 0; i < arraysize(compact_forms); ++i) {
      if (h == compact_forms[i]) {
        type = static_cast<Header::Type>(i);
        break;
      }
    }
  } else {
    // Perform a simple binary search
    const char **first = names;
    const char **last = names + arraysize(names);
    const char **found = std::lower_bound(first, last, str, HeaderNameLess);
    if (found != last
        && base::CompareCaseInsensitiveASCII(*found, str) == 0) {
      type = static_cast<Header::Type>(found - first);
    }
  }

  return type;
}

}  // namespace sippet
