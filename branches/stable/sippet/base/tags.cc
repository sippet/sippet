// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/base/tags.h"

#include "crypto/random.h"
#include "base/strings/string_util.h"
#include "base/base64.h"

namespace sippet {

std::string CreateRandomString(int bits) {
  // This function uses Base64 for the final encoding, as it will generate a
  // shorter string than hex (just 33% of overhead).

  // First, round up the number of bits.
  int bytes = (bits + 7) >> 3;

  // Avoid Base64 padding at the end (must be multiple of 3).
  int base64_no_padding = ((bytes + 2) / 3) * 3;

  char *tag = new char[base64_no_padding];

  crypto::RandBytes(tag, base64_no_padding);
  std::string random_string;
  base::StringPiece part(tag, base64_no_padding);
  base::Base64Encode(part, &random_string);

  delete [] tag;

  // Slash is an invalid character for SIP tokens, so substitute it by dot.
  ReplaceChars(random_string, "/", ".", &random_string);

  return random_string;
}

} // End of sippet namespace

