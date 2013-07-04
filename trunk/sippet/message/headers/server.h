// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_SERVER_H_
#define SIPPET_MESSAGE_HEADERS_SERVER_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Server :
  public Header,
  public single_value<std::string> {
private:
  DISALLOW_ASSIGN(Server);
  Server(const Server &other) : Header(other), single_value(other) {}
  virtual Server *DoClone() const {
    return new Server(*this);
  }
public:
  Server() : Header(Header::HDR_SERVER) {}
  Server(const single_value::value_type &subject)
    : Header(Header::HDR_SERVER), single_value(subject) {}

  scoped_ptr<Server> Clone() const {
    return scoped_ptr<Server>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    Header::print(os);
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_SERVER_H_
