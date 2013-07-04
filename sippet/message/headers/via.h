// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_VIA_H_
#define SIPPET_MESSAGE_HEADERS_VIA_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/protocol.h"
#include "sippet/message/version.h"
#include "sippet/base/format.h"
#include "net/base/host_port_pair.h"

namespace sippet {

class ViaParam :
  public has_parameters,
  public has_ttl<ViaParam>,
  public has_maddr<ViaParam>,
  public has_received<ViaParam>,
  public has_branch<ViaParam>,
  public has_rport<ViaParam> {
public:
  ViaParam() : version_(2,0) {}
  ViaParam(const ViaParam &other)
    : version_(other.version_), protocol_(other.protocol_),
      sent_by_(other.sent_by_), has_parameters(other) {}
  ViaParam(const Protocol &p,
           const net::HostPortPair &sent_by)
    : version_(2,0), protocol_(p), sent_by_(sent_by) {}
  ViaParam(const std::string &protocol,
           const net::HostPortPair &sent_by)
    : version_(2,0), protocol_(protocol), sent_by_(sent_by) {}
  ViaParam(const Version &version,
           const std::string &protocol,
           const net::HostPortPair &sent_by)
    : version_(version), protocol_(protocol), sent_by_(sent_by) {}

  ~ViaParam() {}

  ViaParam &operator=(const ViaParam &other) {
    version_ = other.version_;
    protocol_ = other.protocol_;
    sent_by_ = other.sent_by_;
    has_parameters::operator=(other);
    return *this;
  }

  Version version() const { return version_; }
  void set_version(const Version &version) { version_ = version; }

  Protocol protocol() const { return protocol_; }
  void set_protocol(const Protocol &protocol) { protocol_ = protocol; }

  net::HostPortPair sent_by() const { return sent_by_; }
  void set_sent_by(const net::HostPortPair &sent_by) {
    sent_by_ = sent_by;
  }

  void print(raw_ostream &os) const {
    os << "SIP/"
       << version_.major_value() << "."
       << version_.minor_value() << "/"
       << protocol_
       << " ";
    if (sent_by_.host().find(':') != std::string::npos)
      os << "[" << sent_by_.host() << "]";
    else
      os << sent_by_.host();
    if (sent_by_.port() != 0)
      os << ":" << sent_by_.port();
    has_parameters::print(os);
  }
private:
  Version version_;
  Protocol protocol_;
  net::HostPortPair sent_by_;
};

inline
raw_ostream &operator<<(raw_ostream &os, const ViaParam &p) {
  p.print(os);
  return os;
}

class Via :
  public Header,
  public has_multiple<ViaParam> {
private:
  DISALLOW_ASSIGN(Via);
  Via(const Via &other)
    : Header(other), has_multiple(other) {}
  virtual Via *DoClone() const {
    return new Via(*this);
  }
public:
  Via()
    : Header(Header::HDR_VIA) {}
  Via(const ViaParam &param)
    : Header(Header::HDR_VIA) { push_back(param); }

  scoped_ptr<Via> Clone() const {
    return scoped_ptr<Via>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_VIA_H_
