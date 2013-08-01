// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

namespace sippet {

Message::Message(bool is_request)
  : is_request_(is_request) {}

Message::~Message() {}

void Message::print(raw_ostream &os) const {
  const_iterator i = headers_.begin(), ie = headers_.end();
  for (; i != ie; ++i) {
    i->print(os);
    os << "\r\n";
  }
  os << "\r\n";
  if (has_content()) {
    os.write(content_.data(), content_.length());
  }
}

std::string Message::ToString() const {
  std::string output;
  raw_string_ostream os(output);
  print(os);
  return os.str();
}

}
