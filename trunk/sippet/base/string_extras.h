/*
 * University of Illinois/NCSA
 * Open Source License
 * 
 * Copyright (c) 2003-2013 University of Illinois at Urbana-Champaign.
 * All rights reserved.
 * 
 * Developed by:
 *     LLVM Team
 *     University of Illinois at Urbana-Champaign
 *     http://llvm.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal with
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimers in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the names of the LLVM Team, University of Illinois at
 *       Urbana-Champaign, nor the names of its contributors may be used to
 *       endorse or promote products derived from this Software without specific
 *       prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 * SOFTWARE.
 */

#ifndef SIPPET_BASE_STRING_EXTRAS_H_
#define SIPPET_BASE_STRING_EXTRAS_H_

#include "base/strings/string_piece.h"

namespace sippet {

/// hexdigit - Return the hexadecimal character for the
/// given number \p X (which should be less than 16).
static inline char hexdigit(unsigned X, bool LowerCase = false) {
  const char HexChar = LowerCase ? 'a' : 'A';
  return X < 10 ? '0' + X : HexChar + X - 10;
}

/// Interpret the given character \p C as a hexadecimal digit and return its
/// value.
///
/// If \p C is not a valid hex digit, -1U is returned.
static inline unsigned hexDigitValue(char C) {
  if (C >= '0' && C <= '9') return C-'0';
  if (C >= 'a' && C <= 'f') return C-'a'+10U;
  if (C >= 'A' && C <= 'F') return C-'A'+10U;
  return static_cast<unsigned>(-1);
}

/// utohex_buffer - Emit the specified number into the buffer specified by
/// BufferEnd, returning a pointer to the start of the string.  This can be used
/// like this: (note that the buffer must be large enough to handle any number):
///    char Buffer[40];
///    printf("0x%s", utohex_buffer(X, Buffer+40));
///
/// This should only be used with unsigned types.
///
template<typename IntTy>
static inline char *utohex_buffer(IntTy X, char *BufferEnd) {
  char *BufPtr = BufferEnd;
  *--BufPtr = 0;      // Null terminate buffer.
  if (X == 0) {
    *--BufPtr = '0';  // Handle special case.
    return BufPtr;
  }

  while (X) {
    unsigned char Mod = static_cast<unsigned char>(X) & 15;
    *--BufPtr = hexdigit(Mod);
    X >>= 4;
  }
  return BufPtr;
}

static inline std::string utohexstr(uint64 X) {
  char Buffer[17];
  return utohex_buffer(X, Buffer+17);
}

static inline std::string utostr_32(uint32 X, bool isNeg = false) {
  char Buffer[11];
  char *BufPtr = Buffer+11;

  if (X == 0) *--BufPtr = '0';  // Handle special case...

  while (X) {
    *--BufPtr = '0' + char(X % 10);
    X /= 10;
  }

  if (isNeg) *--BufPtr = '-';   // Add negative sign...

  return std::string(BufPtr, Buffer+11);
}

static inline std::string utostr(uint64 X, bool isNeg = false) {
  char Buffer[21];
  char *BufPtr = Buffer+21;

  if (X == 0) *--BufPtr = '0';  // Handle special case...

  while (X) {
    *--BufPtr = '0' + char(X % 10);
    X /= 10;
  }

  if (isNeg) *--BufPtr = '-';   // Add negative sign...
  return std::string(BufPtr, Buffer+21);
}


static inline std::string itostr(int64 X) {
  if (X < 0)
    return utostr(static_cast<uint64>(-X), true);
  else
    return utostr(static_cast<uint64>(X));
}

/// StrInStrNoCase - Portable version of strcasestr.  Locates the first
/// occurrence of string 's1' in string 's2', ignoring case.  Returns
/// the offset of s2 in s1 or npos if s2 cannot be found.
base::StringPiece::size_type StrInStrNoCase(const base::StringPiece &s1,
                                            const base::StringPiece &s2);

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<base::StringPiece, base::StringPiece> getToken(
    const base::StringPiece &Source,
    const base::StringPiece &Delimiters = " \t\n\v\f\r");

/// HashString - Hash function for strings.
///
/// This is the Bernstein hash function.
//
// FIXME: Investigate whether a modified bernstein hash function performs
// better: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
//   X*33+c -> X*33^c
static inline unsigned HashString(const base::StringPiece &Str, unsigned Result = 0) {
  for (size_t i = 0, e = Str.size(); i != e; ++i)
    Result = Result * 33 + (unsigned char)Str[i];
  return Result;
}

/// Returns the English suffix for an ordinal integer (-st, -nd, -rd, -th).
static inline base::StringPiece getOrdinalSuffix(unsigned Val) {
  // It is critically important that we do this perfectly for
  // user-written sequences with over 100 elements.
  switch (Val % 100) {
  case 11:
  case 12:
  case 13:
    return "th";
  default:
    switch (Val % 10) {
      case 1: return "st";
      case 2: return "nd";
      case 3: return "rd";
      default: return "th";
    }
  }
}

} // End of sippet namespace

#endif // SIPPET_BASE_STRING_EXTRAS_H_
