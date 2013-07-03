// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_canon_internal.h"

namespace sippet {
namespace uri_canon {

namespace {

static unsigned char kUserInfoChar[] =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "0123456789"
  "&=+$,;?/"
  "-_.!~*'()";

bool IsUserInfoChar(unsigned char ch) {
  static unsigned char *begin = kUserInfoChar;
  static unsigned char *end = kUserInfoChar + arraysize(kUserInfoChar);
  return std::find(begin, end, ch) != end;
}

template<typename CHAR, typename UCHAR>
void DoAppendUserInfoString(const CHAR* source, int length,
                            CanonOutput* output) {
  for (int i = 0; i < length; i++) {
    if (static_cast<UCHAR>(source[i]) >= 0x80) {
      // ReadChar will fill the code point with kUnicodeReplacementCharacter
      // when the input is invalid, which is what we want.
      unsigned code_point;
      ReadUTFChar(source, &i, length, &code_point);
      AppendUTF8EscapedValue(code_point, output);
    } else {
      // Just append the 7-bit character, possibly escaping it.
      unsigned char uch = static_cast<unsigned char>(source[i]);
      if (!IsUserInfoChar(uch))
        AppendEscapedChar(uch, output);
      else
        output->push_back(uch);
    }
  }
}

} // End of empty namespace

void AppendUserInfoString(const char* source, int length,
                          CanonOutput* output) {
  DoAppendUserInfoString<char, unsigned char>(source, length, output);
}

void AppendUserInfoString(const char16* source, int length,
                          CanonOutput* output) {
  DoAppendUserInfoString<char16, char16>(source, length, output);
}

} // End of uri_canon namespace
} // End of sippet namespace
