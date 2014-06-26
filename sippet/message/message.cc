// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

namespace sippet {

Message::Message(bool is_request)
  : is_request_(is_request) {}

Message::~Message() {}

void Message::print(raw_ostream &os) const {
  for (const_iterator i = headers_.begin(), ie = headers_.end();
       i != ie; ++i) {
    if (isa<ContentLength>(i))
      continue;
    i->print(os);
    os << "\r\n";
  }

  // Force the Content Length to match the content size
  scoped_ptr<ContentLength> content_length(
    new ContentLength(content_.length()));
  content_length->print(os);
  os << "\r\n";

  // End of header
  os << "\r\n";

  // Append the content when available
  if (has_content())
    os.write(content_.data(), content_.length());
}

std::string Message::ToString() const {
  std::string output;
  raw_string_ostream os(output);
  print(os);
  return os.str();
}

}
