// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"
#include "url/url_parse_internal.h"

namespace sippet {
namespace uri {

// Given an already-initialized begin index and length, this shrinks the range
// to eliminate "should-be-trimmed" characters. Note that the length does *not*
// indicate the length of untrimmed data from |*begin|, but rather the position
// in the input string (so the string starts at character |*begin| in the spec,
// and goes until |*len|).
template<typename CHAR>
inline void TrimURI(const CHAR* spec, int* begin, int* len) {
  return url::TrimURL(spec, begin, len);
}

// Internal functions in uri.cc that parse the parameters and headers,
// that is, everything following the authority section. The input is the range
// of everything following the authority section, and the output is the
// identified ranges.
void ParseAfterAuthorityInternal(const char* spec,
                                 const Component& remaining,
                                 Component* parameters,
                                 Component* headers);
void ParseAfterAuthorityInternal(const base::char16* spec,
                                 const Component& remaining,
                                 Component* parameters,
                                 Component* headers);

// Given a spec and a pointer to the character after the colon following the
// scheme, this parses it and fills in the structure, Every item in the parsed
// structure is filled EXCEPT for the scheme, which is untouched.
void ParseAfterScheme(const char* spec,
                      int spec_len,
                      int after_scheme,
                      Parsed* parsed);
void ParseAfterScheme(const base::char16* spec,
                      int spec_len,
                      int after_scheme,
                      Parsed* parsed);

} // End of uri
} // End of sippet namespace
