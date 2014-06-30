// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_SEQUENCES_H_
#define SIPPET_BASE_SEQUENCES_H_

#include "crypto/random.h"

namespace sippet {

// Create a non-zero 16-bit random integer.
inline
unsigned Create16BitRandomInteger() {
  char bytes[2];
  crypto::RandBytes(bytes, sizeof(bytes));
  return static_cast<unsigned>(bytes[0]) |
      (static_cast<unsigned>(bytes[1]) << 8);
}

} // End of sippet namespace

#endif // SIPPET_BASE_SEQUENCES_H_
