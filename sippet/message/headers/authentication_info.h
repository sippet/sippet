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

#ifndef SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_
#define SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/auth_setters.h"
#include "sippet/message/headers/bits/has_auth_params.h"

namespace sippet {

class AuthenticationInfo :
  public Header,
  public has_nextnonce,
  public has_qop,
  public has_rspauth,
  public has_cnonce,
  public has_nc {
private:
  AuthenticationInfo(const AuthenticationInfo &other)
    : Header(other), has_nextnonce(other), has_qop(other), has_rspauth(other),
      has_cnonce(other), has_nc(other) {}
  AuthenticationInfo &operator=(const AuthenticationInfo &);
  virtual AuthenticationInfo *DoClone() const {
    return new AuthenticationInfo(*this);
  }
public:
  AuthenticationInfo() : Header(Header::HDR_AUTHENTICATION_INFO) {}

  scoped_ptr<AuthenticationInfo> Clone() const {
    return scoped_ptr<AuthenticationInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Authentication-Info");
    bool written = false;
    if (!nextnonce().empty()) {
      has_nextnonce::print(os), written = true;
    }
    if (!qop().empty()) {
      if (written) os << ", ";
      has_qop::print(os), written = true;
    }
    if (!rspauth().empty()) {
      if (written) os << ", ";
      has_rspauth::print(os), written = true;
    }
    if (!cnonce().empty()) {
      if (written) os << ", ";
      has_cnonce::print(os), written = true;
    }
    if (nc() != 0) {
      if (written) os << ", ";
      has_nc::print(os);
    }
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_
