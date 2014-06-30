// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_TAGS_H_
#define SIPPET_BASE_TAGS_H_

#include "crypto/random.h"
#include "base/strings/string_number_conversions.h"

namespace sippet {

// Create a 32-bit random string
inline
std::string Create32BitRandomString() {
  char tag[4];
  crypto::RandBytes(tag, sizeof(tag));
  return base::HexEncode(tag, sizeof(tag));
}

} // End of sippet namespace

#endif // SIPPET_BASE_TAGS_H_
