// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_canon.h"
#include "googleurl/src/url_canon_internal.h"

namespace sippet {
namespace uri_canon {

// Appends the given string to the output, escaping characters that do not
// match the userinfo part of the SIP-URI.
void AppendUserInfoString(const char* source, int length,
                          CanonOutput* output);
void AppendUserInfoString(const char16* source, int length,
                          CanonOutput* output);

// UTF-8 functions ------------------------------------------------------------

// Reads one character in UTF-8 starting at |*begin| in |str| and places
// the decoded value into |*code_point|. If the character is valid, we will
// return true. If invalid, we'll return false and put the
// kUnicodeReplacementCharacter into |*code_point|.
//
// |*begin| will be updated to point to the last character consumed so it
// can be incremented in a loop and will be ready for the next character.
// (for a single-byte ASCII character, it will not be changed).
inline
bool ReadUTFChar(const char* str, int* begin, int length,
                 unsigned* code_point_out) {
  return url_canon::ReadUTFChar(str, begin, length, code_point_out);
}

// UTF-16 functions -----------------------------------------------------------

// Reads one character in UTF-16 starting at |*begin| in |str| and places
// the decoded value into |*code_point|. If the character is valid, we will
// return true. If invalid, we'll return false and put the
// kUnicodeReplacementCharacter into |*code_point|.
//
// |*begin| will be updated to point to the last character consumed so it
// can be incremented in a loop and will be ready for the next character.
// (for a single-16-bit-word character, it will not be changed).
inline
bool ReadUTFChar(const char16* str, int* begin, int length,
                 unsigned* code_point) {
  return url_canon::ReadUTFChar(str, begin, length, code_point);
}

} // End of uri_canon namespace
} // End of sippet namespace
