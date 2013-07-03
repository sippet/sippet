// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_canon.h"
#include "googleurl/src/url_canon_internal.h"

namespace sippet {
namespace uri_canon {

// Character type handling -----------------------------------------------------

// Bits that identify different character types. These types identify different
// bits that are set for each 8-bit character in the kSharedCharTypeTable.
enum SharedCharTypes {
  // Valid in the username/password field.
  CHAR_USERINFO = 1,

  // Valid in parameters field.
  CHAR_PARAMETERS = 2,

  // Valid in headers field.
  CHAR_HEADERS = 4,
};

// This table contains the flags in SharedCharTypes for each 8-bit character.
// Some canonicalization functions have their own specialized lookup table.
// For those with simple requirements, we have collected the flags in one
// place so there are fewer lookup tables to load into the CPU cache.
//
// Using an unsigned char type has a small but measurable performance benefit
// over using a 32-bit number.
extern const unsigned char kSharedCharTypeTable[0x100];

// More readable wrappers around the character type lookup table.
inline bool IsCharOfType(unsigned char c, SharedCharTypes type) {
  return !!(kSharedCharTypeTable[c] & type);
}
inline bool IsQueryChar(unsigned char c) {
  return url_canon::IsQueryChar(c);
}
inline bool IsIPv4Char(unsigned char c) {
  return url_canon::IsIPv4Char(c);
}
inline bool IsHexChar(unsigned char c) {
  return url_canon::IsHexChar(c);
}
inline bool IsComponentChar(unsigned char c) {
  return url_canon::IsComponentChar(c);
}
inline bool IsUserInfoChar(unsigned char c) {
  return IsCharOfType(c, CHAR_USERINFO);
}
inline bool IsParametersChar(unsigned char c) {
  return IsCharOfType(c, CHAR_PARAMETERS);
}
inline bool IsHeadersChar(unsigned char c) {
  return IsCharOfType(c, CHAR_HEADERS);
}

// Appends the given string to the output, escaping characters that do not
// match the given |type| in SharedCharTypes.
void AppendStringOfType(const char* source, int length,
                        SharedCharTypes type,
                        CanonOutput* output);
void AppendStringOfType(const char16* source, int length,
                        SharedCharTypes type,
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
