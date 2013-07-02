// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/status.h"

namespace sippet {

namespace {
  const char *reason_phrases[] = {
#define X(class_name, code, reason_phrase) \
  reason_phrase,
  #include "sippet/message/known_statuses.h"
#undef X
    "", // for Unknown
  };
}

const char *Status::string_of(Type t) {
  return reason_phrases[static_cast<int>(t)];
}

} // End of sippet namespace
