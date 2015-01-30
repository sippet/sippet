// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_
#define SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/auth_setters.h"
#include "sippet/message/headers/bits/has_auth_params.h"

namespace sippet {

class AuthenticationInfo :
  public Header,
  public has_nextnonce<AuthenticationInfo>,
  public has_qop<AuthenticationInfo>,
  public has_rspauth<AuthenticationInfo>,
  public has_cnonce<AuthenticationInfo>,
  public has_nc<AuthenticationInfo>,
  public has_auth_params {
 private:
  DISALLOW_ASSIGN(AuthenticationInfo);
  AuthenticationInfo(const AuthenticationInfo &other);
  virtual AuthenticationInfo *DoClone() const override;

 public:
  AuthenticationInfo();
  virtual ~AuthenticationInfo();

  scoped_ptr<AuthenticationInfo> Clone() const {
    return scoped_ptr<AuthenticationInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_AUTHENTICATION_INFO_H_
