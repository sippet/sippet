// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "googleurl/src/url_util.h"

namespace sippet {
namespace uri_util {

// String helper functions ----------------------------------------------------

// Compare the lower-case form of the given string against the given ASCII
// string.  This is useful for doing checking if an input string matches some
// token, and it is optimized to avoid intermediate string copies.
//
// The versions of this function that don't take a b_end assume that the b
// string is NULL terminated.
inline
bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b) {
  return url_util::LowerCaseEqualsASCII(a_begin, a_end, b);
}
inline
bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b_begin,
                          const char* b_end) {
  return url_util::LowerCaseEqualsASCII(a_begin, a_end, b_begin, b_end);
}
inline
bool LowerCaseEqualsASCII(const char16* a_begin,
                          const char16* a_end,
                          const char* b) {
  return url_util::LowerCaseEqualsASCII(a_begin, a_end, b);
}


} // End of uri_util namespace
} // End of sippet namespace