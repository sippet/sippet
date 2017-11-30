// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_END_POINT_H_
#define SIPPET_TRANSPORT_END_POINT_H_

#include <string>
#include <functional>

#include "net/base/host_port_pair.h"
#include "sippet/message/protocol.h"
#include "sippet/uri/uri.h"

namespace sippet {

class EndPoint {
 public:
  EndPoint();
  EndPoint(const EndPoint &other);
  EndPoint(const net::HostPortPair &hostport, const Protocol &protocol);
  EndPoint(const net::HostPortPair &hostport, Protocol::Type protocol);

  // If |host| represents an IPv6 address, it should not bracket the address.
  EndPoint(const std::string& host, uint16_t port, const Protocol &protocol);
  EndPoint(const std::string& host, uint16_t port, Protocol::Type protocol);

  ~EndPoint();

  // Creates an EndPoint from a string formatted in same manner as ToString().
  static EndPoint FromString(const std::string& str);

  // Creates an EndPoint from a SIP-URI.
  static EndPoint FromSipURI(const SipURI& uri);

  // Creates an EndPoint from a GURL.
  static EndPoint FromGURL(const GURL& url);

  bool IsEmpty() const {
    return protocol_ == Protocol::Unknown && hostport_.IsEmpty();
  }

  bool Equals(const EndPoint &other) const;

  // XXX: broken Google style, but doesn't matter now
  bool operator==(const EndPoint &other) const {
    return Equals(other);
  }

  const std::string& host() const {
    return hostport_.host();
  }

  uint16_t port() const {
    return hostport_.port();
  }

  Protocol protocol() const {
    return protocol_;
  }

  net::HostPortPair hostport() const {
    return hostport_;
  }

  void set_host(const std::string& host) {
    hostport_.set_host(host);
  }

  void set_port(uint16_t port) {
    hostport_.set_port(port);
  }

  void set_protocol(const Protocol &protocol) {
    protocol_ = protocol;
  }

  void set_hostport(const net::HostPortPair &hostport) {
    hostport_ = hostport;
  }

  // ToString() will convert the EndPoint tuple into "host:port/protocol".
  // If |host| is an IPv6 literal, it will add brackets around |host|.
  std::string ToString() const;

private:
  friend struct EndPointEquals;
  friend struct EndPointLess;
  net::HostPortPair hostport_;
  Protocol protocol_;
};

// To be used in std::find
struct EndPointEquals :
  public std::binary_function<EndPoint, EndPoint, bool> {
  bool operator()(const EndPoint &a, const EndPoint &b) const {
    return a.protocol_.type() == b.protocol_.type()
           && a.hostport_.Equals(b.hostport_);
  }
};

// To be used in std::map
struct EndPointLess :
  public std::binary_function<EndPoint, EndPoint, bool> {
  bool operator()(const EndPoint &a, const EndPoint &b) const {
    if (a.protocol_.type() != b.protocol_.type())
      return a.protocol_.type() < b.protocol_.type();
    return a.hostport_ < b.hostport_;
  }
};

inline
bool EndPoint::Equals(const EndPoint &other) const {
  return EndPointEquals()(*this, other);
}

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_END_POINT_H_
