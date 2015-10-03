// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_URI_URI_UTIL_H_
#define SIPPET_URI_URI_UTIL_H_

#include "base/strings/string16.h"

namespace url {
struct CharsetConverter;
template<typename T> struct CanonOutputT;
struct Parsed;
}  // namespace url

namespace sippet {
namespace uri {

using url::CharsetConverter;
using url::CanonOutputT;
struct Parsed;

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
                  CharsetConverter* charset_converter,
                  CanonOutputT<char>* output,
                  Parsed* output_parsed);
bool Canonicalize(const base::char16* spec,
                  int spec_len,
                  CharsetConverter* charset_converter,
                  CanonOutputT<char>* output,
                  Parsed* output_parsed);

// Unescapes the given string using URL escaping rules.
void DecodeURIEscapeSequences(const char* input, int length,
                              CanonOutputT<char>* output);

} // End of uri namespace
} // End of sippet namespace

#endif // SIPPET_URI_URI_UTIL_H_

