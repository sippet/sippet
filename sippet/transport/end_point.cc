// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/end_point.h"

#include <utility>
#include <vector>

#include "base/strings/string_split.h"
#include "net/base/url_util.h"

namespace sippet {

EndPoint::EndPoint()
  : protocol_(Protocol::Unknown) {
}

EndPoint::EndPoint(const EndPoint &other)
  : hostport_(other.hostport_), protocol_(other.protocol_) {
}

EndPoint::EndPoint(const net::HostPortPair &hostport, const Protocol &protocol)
  : hostport_(hostport), protocol_(protocol) {
}

EndPoint::EndPoint(const net::HostPortPair &hostport, Protocol::Type protocol)
  : hostport_(hostport), protocol_(protocol) {
}

EndPoint::EndPoint(const std::string& host, uint16_t port,
                   const Protocol &protocol)
  : hostport_(host, port), protocol_(protocol) {
}

EndPoint::EndPoint(const std::string& host, uint16_t port,
                   Protocol::Type protocol)
  : hostport_(host, port), protocol_(protocol) {
}

EndPoint::~EndPoint() {
}

EndPoint EndPoint::FromString(const std::string& str) {
  std::vector<std::string> hostport_protocol(
      base::SplitString(str, "/",
          base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY));
  if (hostport_protocol.size() != 2)
    return EndPoint();
  int port;
  std::string host;
  if (!net::ParseHostAndPort(hostport_protocol[0], &host, &port))
    return EndPoint();
  if (host[0] == '[')
    host.assign(host.substr(1, host.size()-2));
  Protocol protocol(hostport_protocol[1]);
  if (Protocol::Unknown == protocol)
    return EndPoint();
  EndPoint endpoint;
  endpoint.set_host(host);
  endpoint.set_port(port);
  endpoint.set_protocol(protocol);
  return endpoint;
}

EndPoint EndPoint::FromSipURI(const SipURI& uri) {
  std::pair<bool, std::string> result =
    uri.parameter("transport");
  // There should be no transport parameter when using SIPS
  if (uri.SchemeIsSecure() && result.first) {
    NOTREACHED() << "'sips' scheme doesn't accept transport parameter";
    return EndPoint();
  }
  // The default transport is UDP.
  Protocol protocol = (!result.first)
                    ? (uri.SchemeIsSecure() ? Protocol::TLS : Protocol::UDP)
                    : Protocol(result.second);
  return EndPoint(uri.host(), uri.EffectiveIntPort(), protocol);
}

EndPoint EndPoint::FromGURL(const GURL& url) {
  if (!url.SchemeIs("sip") && !url.SchemeIs("sips"))
    return EndPoint();
  return FromSipURI(SipURI(url));
}

std::string EndPoint::ToString() const {
  return hostport_.ToString() + "/" + protocol_.str();
}

}  // namespace sippet
