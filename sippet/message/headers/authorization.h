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

#ifndef SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_
#define SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/auth_setters.h"
#include "sippet/message/headers/bits/has_auth_params.h"

namespace sippet {

class Authorization :
  public Header,
  public has_username,
  public has_realm,
  public has_nonce,
  public has_uri,
  public has_response,
  public has_algorithm,
  public has_cnonce,
  public has_opaque,
  public has_auth_params {
private:
  Authorization(const Authorization &other)
    : Header(other), has_username(other), has_realm(other), has_nonce(other),
      has_uri(other), has_response(other), has_algorithm(other),
      has_cnonce(other), has_opaque(other), has_auth_params(other) {}
  Authorization &operator=(const Authorization &);
  virtual Authorization *DoClone() const {
    return new Authorization(*this);
  }
public:
  Authorization() : Header(Header::HDR_AUTHORIZATION) {}

  scoped_ptr<Authorization> Clone() const {
    return scoped_ptr<Authorization>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Authorization");
    bool written = false;
    if (!username().empty()) {
      has_username::print(os), written = true;
    }
    if (!realm().empty()) {
      if (written) os << ", ";
      has_realm::print(os), written = true;
    }
    if (!nonce().empty()) {
      if (written) os << ", ";
      has_nonce::print(os), written = true;
    }
    if (!uri().empty()) {
      if (written) os << ", ";
      has_uri::print(os), written = true;
    }
    if (!response().empty()) {
      if (written) os << ", ";
      has_response::print(os), written = true;
    }
    if (!algorithm().empty()) {
      if (written) os << ", ";
      has_algorithm::print(os), written = true;
    }
    if (!cnonce().empty()) {
      if (written) os << ", ";
      has_cnonce::print(os), written = true;
    }
    if (!opaque().empty()) {
      if (written) os << ", ";
      has_opaque::print(os), written = true;
    }
    if (!has_auth_params::param_empty()) {
      if (written) os << ", ";
      has_auth_params::print(os);
    }
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_
