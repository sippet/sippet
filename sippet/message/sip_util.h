// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_SIP_UTIL_H_
#define SIPPET_SIP_UTIL_H_

#include <stddef.h>
#include <stdint.h>

#include "base/strings/string_tokenizer.h"
#include "base/strings/string_piece.h"
#include "net/http/http_util.h"

namespace sippet {

class SipUtil {
 public:
  // Delegates to |net::HttpUtil::ParseContentType|.
  static void ParseContentType(const std::string& content_type_str,
                               std::string* mime_type,
                               std::string* charset,
                               bool* had_charset,
                               std::string* boundary) {
    net::HttpUtil::ParseContentType(content_type_str, mime_type, charset,
        had_charset, boundary);
  }

  // Multiple occurances of some headers cannot be coalesced into a comma-
  // separated list.
  static bool IsNonCoalescingHeader(const base::StringPiece& name);

  // Contact-like headers need special handling.
  static bool IsContactLikeHeader(const base::StringPiece& name);

  // Expand the header compact form into their long formats.
  static const char* ExpandHeader(char c);

  // Reusing the |net::HttpUtil::HeadersIterator|, as it is equivalent.
  using HeadersIterator = net::HttpUtil::HeadersIterator;

  // Reusing the |net::HttpUtil::ValuesIterator|, as it is equivalent.
  using ValuesIterator = net::HttpUtil::ValuesIterator;

  // Reusing the |net::HttpUtil::NameValuePairsIterator|, as it is equivalent.
  using NameValuePairsIterator = net::HttpUtil::NameValuePairsIterator;
};

}  // namespace sippet

#endif  // SIPPET_SIP_UTIL_H_
