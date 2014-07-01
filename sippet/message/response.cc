// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/response.h"

namespace sippet {

Response::Response(int response_code,
    const std::string &reason_phrase,
    Direction direction,
    const Version &version)
  : Message(false, direction), response_code_(response_code),
    reason_phrase_(reason_phrase),
    version_(version) {}

void Response::print(raw_ostream &os) const {
  os << "SIP/" << version_.major_value()
     << "." << version_.minor_value()
     << " " << response_code_
     << " " << reason_phrase_
     << "\r\n";
  Message::print(os);
}

std::string Response::GetDialogId() {
  std::string call_id(get<CallId>()->value());
  std::string from_tag(get<From>()->tag());
  std::string to_tag(get<To>()->tag());
  std::ostringstream oss;
  oss << call_id << ":";
  if (direction() == Outgoing) {
    oss << to_tag << ":" << from_tag;
  } else {
    oss << from_tag << ":" << to_tag;
  }
  return oss.str();
}

} // End of sippet namespace
