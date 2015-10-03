// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/ice_server.h"

namespace sippet {
namespace phone {

IceServer::IceServer() {
}

IceServer::~IceServer() {
}

IceServer::IceServer(const std::string& uri) :
  uri_(uri) {
}

IceServer::IceServer(const std::string& uri,
                     const std::string& username,
                     const std::string& password) :
  uri_(uri),
  username_(username),
  password_(password) {
}

}  // namespace phone
}  // namespace sippet
