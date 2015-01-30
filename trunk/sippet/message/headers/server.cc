// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/server.h"

namespace sippet {

Server::Server()
  : Header(Header::HDR_SERVER) {
}

Server::Server(const single_value::value_type &subject)
  : Header(Header::HDR_SERVER), single_value(subject) {
}

Server::Server(const Server &other)
  : Header(other), single_value(other) {
}

Server::~Server() {
}

Server *Server::DoClone() const {
  return new Server(*this);
}

void Server::print(raw_ostream &os) const {
  Header::print(os);
  single_value::print(os);
}

} // End of sippet namespace
