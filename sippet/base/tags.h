// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_TAGS_H_
#define SIPPET_BASE_TAGS_H_

#include <string>

namespace sippet {

// Create a random string at least of that indicated size of bits
std::string CreateRandomString(int bits);

// This is the magic cookie "z9hG4bK" defined in RFC 3261
static const char kMagicCookie[] = "z9hG4bK";

// Create an unique local branch (72-bit random string, 7+12 characters long).
inline std::string CreateBranch() {
  return kMagicCookie + CreateRandomString(72);
}

// Create a local tag (48-bit random string, 8 characters long).
inline std::string CreateTag() {
  return CreateRandomString(48);
}

// Create an unique Call-ID (120-bit random string, 20 characters long).
inline std::string CreateCallId() {
  return CreateRandomString(120);
}

} // End of sippet namespace

#endif // SIPPET_BASE_TAGS_H_
