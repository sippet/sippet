// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "url/url_util.h"
#include "sippet/uri/uri_canon.h"

namespace sippet {
namespace uri {

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
  return url::LowerCaseEqualsASCII(a_begin, a_end, b);
}
inline
bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b_begin,
                          const char* b_end) {
  return url::LowerCaseEqualsASCII(a_begin, a_end, b_begin, b_end);
}
inline
bool LowerCaseEqualsASCII(const base::char16* a_begin,
                          const base::char16* a_end,
                          const char* b) {
  return url::LowerCaseEqualsASCII(a_begin, a_end, b);
}

// URI library wrappers -------------------------------------------------------

// Parses the given spec according to the extracted scheme type. Normal users
// should use the URI object, although this may be useful if performance is
// critical and you don't want to do the heap allocation for the std::string.
//
// As with the uri::Canonicalize* functions, the charset converter can
// be NULL to use UTF-8 (it will be faster in this case).
//
// Returns true if a valid URI was produced, false if not. On failure, the
// output and parsed structures will still be filled and will be consistent,
// but they will not represent a loadable URI.
bool Canonicalize(const char* spec,
                  int spec_len,
                  uri::CharsetConverter* charset_converter,
                  uri::CanonOutput* output,
                  uri::Parsed* output_parsed);
bool Canonicalize(const base::char16* spec,
                  int spec_len,
                  uri::CharsetConverter* charset_converter,
                  uri::CanonOutput* output,
                  uri::Parsed* output_parsed);

} // End of uri namespace
} // End of sippet namespace
