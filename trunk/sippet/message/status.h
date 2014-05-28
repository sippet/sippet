// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_STATUS_H_
#define SIPPET_MESSAGE_STATUS_H_

#include "sippet/message/atom.h"
#include "base/strings/string_util.h"

namespace sippet {

struct Status {
  enum Type {
#define X(class_name, code, reason_phrase) class_name = code,
#include "sippet/message/known_statuses.h"
#undef X
    Unknown
  };
  static const char *string_of(Type t);
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_STATUS_H_
