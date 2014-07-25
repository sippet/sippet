// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"
#include "sippet/uri/uri_canon_internal.h"

namespace sippet {
namespace uri {

namespace {

template<typename CHAR, typename UCHAR>
void DoAppendStringOfType(const CHAR* source, int length,
                          SharedCharTypes type,
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
      if (!IsCharOfType(uch, type))
        AppendEscapedChar(uch, output);
      else
        output->push_back(uch);
    }
  }
}

} // End of empty namespace

// See the header file for this array's declaration.
const unsigned char kSharedCharTypeTable[0x100] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x00 - 0x0f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x10 - 0x1f
    /* 0x20 ' ' */ 0,
    /* 0x21 '!' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x22 '"' */ 0,
    /* 0x23 '#' */ 0,
    /* 0x24 '$' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x25 '%' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x26 '&' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x27 ''' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x28 '(' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x29 ')' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x2a '*' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x2b '+' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x2c ',' */ CHAR_USERINFO,
    /* 0x2d '-' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x2e '.' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x2f '/' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x30 '0' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x31 '1' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x32 '2' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x33 '3' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x34 '4' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x35 '5' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x36 '6' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x37 '7' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x38 '8' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x39 '9' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x3a ':' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x3b ';' */ CHAR_USERINFO | CHAR_PARAMETERS,
    /* 0x3c '<' */ 0,
    /* 0x3d '=' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x3e '>' */ 0,
    /* 0x3f '?' */ CHAR_USERINFO | CHAR_HEADERS,
    /* 0x40 '@' */ 0,
    /* 0x41 'A' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x42 'B' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x43 'C' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x44 'D' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x45 'E' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x46 'F' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x47 'G' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x48 'H' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x49 'I' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4a 'J' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4b 'K' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4c 'L' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4d 'M' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4e 'N' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x4f 'O' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x50 'P' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x51 'Q' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x52 'R' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x53 'S' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x54 'T' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x55 'U' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x56 'V' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x57 'W' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x58 'X' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x59 'Y' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x5a 'Z' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x5b '[' */ CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x5c '\' */ 0,
    /* 0x5d ']' */ CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x5e '^' */ 0,
    /* 0x5f '_' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x60 '`' */ 0,
    /* 0x61 'a' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x62 'b' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x63 'c' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x64 'd' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x65 'e' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x66 'f' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x67 'g' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x68 'h' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x69 'i' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6a 'j' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6b 'k' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6c 'l' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6d 'm' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6e 'n' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x6f 'o' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x70 'p' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x71 'q' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x72 'r' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x73 's' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x74 't' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x75 'u' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x76 'v' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x77 'w' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x78 'x' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x79 'y' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x7a 'z' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x7b '{' */ 0,
    /* 0x7c '|' */ 0,
    /* 0x7d '}' */ 0,
    /* 0x7e '~' */ CHAR_USERINFO | CHAR_PARAMETERS | CHAR_HEADERS,
    /* 0x7f ' ' */ 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x80 - 0x8f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x90 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xa0 - 0xaf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xb0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xc0 - 0xcf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xd0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xe0 - 0xef
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xf0 - 0xff
};

void AppendStringOfType(const char* source, int length,
                        SharedCharTypes type,
                        CanonOutput* output) {
  DoAppendStringOfType<char, unsigned char>(source, length, type, output);
}

void AppendStringOfType(const base::char16* source, int length,
                        SharedCharTypes type,
                        CanonOutput* output) {
  DoAppendStringOfType<base::char16, base::char16>(source, length, type, output);
}

} // End of uri namespace
} // End of sippet namespace
