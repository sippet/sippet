/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

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
  public has_branch<ViaParam> {
public:
  ViaParam() : version_(2,0) {}
  ViaParam(const ViaParam &other)
    : version_(other.version_), protocol_(other.protocol_),
      sent_by_(other.sent_by_), has_parameters(other) {}
  ViaParam(const Atom<Protocol> &p,
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

  Atom<Protocol> protocol() const { return protocol_; }
  void set_protocol(const Atom<Protocol> &protocol) { protocol_ = protocol; }

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
  Atom<Protocol> protocol_;
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
    os.write_hname("Via");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_VIA_H_
