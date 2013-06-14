// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/end_point.h"
#include "base/strings/string_split.h"
#include "net/base/net_util.h"

namespace sippet {

EndPoint EndPoint::FromString(const std::string& str) {
  std::vector<std::string> hostport_protocol;
  base::SplitString(str, '/', &hostport_protocol);
  if (hostport_protocol.size() != 2)
    return EndPoint();
  int port;
  std::string host;
  if (!net::ParseHostAndPort(hostport_protocol[0], &host, &port))
    return EndPoint();
  if (host[0] == '[')
    host.assign(host.substr(1, host.size()-2));
  Atom<Protocol> protocol(hostport_protocol[1]);
  if (Protocol::Unknown == protocol)
    return EndPoint();
  EndPoint endpoint;
  endpoint.set_host(host);
  endpoint.set_port(port);
  endpoint.set_protocol(protocol);
  return endpoint;
}

std::string EndPoint::ToString() const {
  return hostport_.ToString() + "/" + protocol_.str();
}

} // End of sippet namespace
