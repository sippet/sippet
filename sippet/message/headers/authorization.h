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

class Credentials :
  public has_username<Credentials>,
  public has_realm<Credentials>,
  public has_nonce<Credentials>,
  public has_uri<Credentials>,
  public has_response<Credentials>,
  public has_algorithm<Credentials>,
  public has_cnonce<Credentials>,
  public has_opaque<Credentials>,
  public has_nc<Credentials>,
  public has_auth_params {
public:
  Credentials() {}
  Credentials(Scheme s) : has_auth_params(s) {}
  Credentials(const std::string &scheme) : has_auth_params(scheme) {}
  Credentials(const Credentials &other)
    : has_auth_params(other) {}
  ~Credentials() {}
};

class Authorization :
  public Header,
  public Credentials {
private:
  DISALLOW_ASSIGN(Authorization);
  Authorization(const Authorization &other)
    : Header(other), Credentials(other) {}
  virtual Authorization *DoClone() const {
    return new Authorization(*this);
  }
public:
  Authorization() : Header(Header::HDR_AUTHORIZATION) {}
  Authorization(Scheme s)
    : Header(Header::HDR_AUTHORIZATION), Credentials(s) {}
  Authorization(const std::string &scheme)
    : Header(Header::HDR_AUTHORIZATION), Credentials(scheme) {}

  scoped_ptr<Authorization> Clone() const {
    return scoped_ptr<Authorization>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Authorization");
    Credentials::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_AUTHORIZATION_H_
