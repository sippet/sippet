// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/response.h"

namespace sippet {

Response::Response(int response_code,
    const std::string &reason_phrase,
    const Version &version)
  : Message(false), response_code_(response_code),
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

} // End of sippet namespace
