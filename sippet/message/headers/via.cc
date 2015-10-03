// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/via.h"

#include <string>

namespace sippet {

ViaParam::ViaParam()
  : version_(2, 0) {
}

ViaParam::ViaParam(const ViaParam &other)
  : version_(other.version_), protocol_(other.protocol_),
    sent_by_(other.sent_by_), has_parameters(other) {
}

ViaParam::ViaParam(const Protocol &p,
                   const net::HostPortPair &sent_by)
  : version_(2, 0), protocol_(p), sent_by_(sent_by) {
}

ViaParam::ViaParam(const std::string &protocol,
                   const net::HostPortPair &sent_by)
  : version_(2, 0), protocol_(protocol), sent_by_(sent_by) {
}

ViaParam::ViaParam(const Version &version,
                   const std::string &protocol,
                   const net::HostPortPair &sent_by)
  : version_(version), protocol_(protocol), sent_by_(sent_by) {
}

ViaParam::~ViaParam() {
}

ViaParam &ViaParam::operator=(const ViaParam &other) {
  version_ = other.version_;
  protocol_ = other.protocol_;
  sent_by_ = other.sent_by_;
  has_parameters::operator=(other);
  return *this;
}

void ViaParam::print(raw_ostream &os) const {
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
  if (!HasRport())  // RFC 3581
    os << ";rport";
  has_parameters::print(os);
}

Via::Via()
  : Header(Header::HDR_VIA) {
}

Via::Via(const ViaParam &param)
  : Header(Header::HDR_VIA) {
  push_back(param);
}

Via::Via(const Via &other)
  : Header(other), has_multiple(other) {
}

Via::~Via() {
}

Via *Via::DoClone() const {
  return new Via(*this);
}

void Via::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
