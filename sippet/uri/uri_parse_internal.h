// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "googleurl/src/url_parse_internal.h"

namespace sippet {
namespace uri_parse {

// Given an already-initialized begin index and length, this shrinks the range
// to eliminate "should-be-trimmed" characters. Note that the length does *not*
// indicate the length of untrimmed data from |*begin|, but rather the position
// in the input string (so the string starts at character |*begin| in the spec,
// and goes until |*len|).
template<typename CHAR>
inline void TrimURI(const CHAR* spec, int* begin, int* len) {
  return url_parse::TrimURL(spec, begin, len);
}

} // End of uri_parse
} // End of sippet namespace