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

#ifndef SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
#define SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_

#include "sippet/message/headers/bits/auth_setters.h"

namespace sippet {

class Challenge :
  public has_realm<Challenge>,
  public has_domain<Challenge>,
  public has_nonce<Challenge>,
  public has_opaque<Challenge>,
  public has_stale<Challenge>,
  public has_algorithm<Challenge>,
  public has_qop_options<Challenge>,
  public has_auth_params {
public:
  Challenge() {}
  Challenge(Scheme s) : has_auth_params(s) {}
  Challenge(const std::string &scheme) : has_auth_params(scheme) {}
  Challenge(const Challenge &other) : has_auth_params(other) {}
  ~Challenge() {}
};

class WwwAuthenticate :
  public Header,
  public Challenge {
private:
  DISALLOW_ASSIGN(WwwAuthenticate);
  WwwAuthenticate(const WwwAuthenticate &other)
    : Header(other), Challenge(other) {}
  virtual WwwAuthenticate *DoClone() const {
    return new WwwAuthenticate(*this);
  }
public:
  WwwAuthenticate() : Header(Header::HDR_WWW_AUTHENTICATE) {}
  WwwAuthenticate(Scheme s)
    : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(s) {}
  WwwAuthenticate(const std::string &scheme)
    : Header(Header::HDR_WWW_AUTHENTICATE), Challenge(scheme) {}

  scoped_ptr<WwwAuthenticate> Clone() const {
    return scoped_ptr<WwwAuthenticate>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("WWW-Authenticate");
    Challenge::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_WWW_AUTHENTICATE_H_
