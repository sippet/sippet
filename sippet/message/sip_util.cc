// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/sip_util.h"

#include "base/macros.h"
#include "base/strings/string_util.h"

namespace sippet {

// static
bool SipUtil::IsNonCoalescingHeader(std::string::const_iterator name_begin,
                                    std::string::const_iterator name_end) {
  const char* const kNonCoalescingHeaders[] = {
    "date",
    "retry-after",
    "authentication-info",
    "authorization",
    "proxy-authorization",
    "www-authenticate",
    "proxy-authenticate",
  };

  for (const char* header : kNonCoalescingHeaders) {
    if (base::LowerCaseEqualsASCII(base::StringPiece(name_begin, name_end),
                                   header)) {
      return true;
    }
  }
  return false;
}

// static
const char* SipUtil::ExpandHeader(char c) {
  // IANA registers headers and their abbreviations.
  // https://www.cs.columbia.edu/sip/compact.html
  const struct {
    char compact_form;
    const char* const header_name;
  } kCompactHeaders[] = {
    { 'a', "Accept-Contact" },
    { 'b', "Referred-By" },
    { 'c', "Content-Type" },
    { 'e', "Content-Encoding" },
    { 'f', "From" },
    { 'i', "Call-ID" },
    { 'k', "Supported" },
    { 'l', "Content-Length" },
    { 'm', "Contact" },
    { 'o', "Event" },
    { 'r', "Refer-To" },
    { 's', "Subject" },
    { 't', "To" },
    { 'u', "Allow-Events" },
    { 'v', "Via" },
  };

  c = base::ToLowerASCII(c);
  for (size_t i = 0; i < arraysize(kCompactHeaders); i++) {
    if (kCompactHeaders[i].compact_form == c) {
      return kCompactHeaders[i].header_name;
    }
  }

  return nullptr;
}

}  // namespace sippet
