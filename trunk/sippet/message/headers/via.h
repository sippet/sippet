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
  ViaParam();
  ViaParam(const ViaParam &other);
  ViaParam(const Protocol &p,
           const net::HostPortPair &sent_by);
  ViaParam(const std::string &protocol,
           const net::HostPortPair &sent_by);
  ViaParam(const Version &version,
           const std::string &protocol,
           const net::HostPortPair &sent_by);
  ~ViaParam();

  ViaParam &operator=(const ViaParam &other);

  Version version() const { return version_; }
  void set_version(const Version &version) { version_ = version; }

  Protocol protocol() const { return protocol_; }
  void set_protocol(const Protocol &protocol) { protocol_ = protocol; }

  net::HostPortPair sent_by() const { return sent_by_; }
  void set_sent_by(const net::HostPortPair &sent_by) {
    sent_by_ = sent_by;
  }

  void print(raw_ostream &os) const;

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
  Via(const Via &other);
  virtual Via *DoClone() const override;

 public:
  Via();
  Via(const ViaParam &param);
  virtual ~Via();

  scoped_ptr<Via> Clone() const {
    return scoped_ptr<Via>(DoClone());
  }

  virtual void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_VIA_H_
