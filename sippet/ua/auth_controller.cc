// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_controller.h"
#include "net/base/net_errors.h"

namespace sippet {

// Based on net/http/http_auth_controller.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

AuthController::AuthController(
    AuthCache* auth_cache,
    AuthHandlerFactory* auth_handler_factory) {}

int AuthController::MaybeGenerateAuthToken(
    const scoped_refptr<Request> &request,
    const net::CompletionCallback& callback,
    const net::BoundNetLog& net_log) {
  return net::ERR_NOT_IMPLEMENTED;
}

void AuthController::AddAuthorizationHeaders(
    const scoped_refptr<Request> &request) {
  // TODO
}

int AuthController::HandleAuthChallenge(
    const scoped_refptr<Response> &response,
    const net::BoundNetLog& net_log) {
  // TODO
  return net::ERR_NOT_IMPLEMENTED;
}

bool AuthController::HaveAuth() const {
  // TODO
  return false;
}

scoped_refptr<net::AuthChallengeInfo> AuthController::auth_info() {
  // TODO
  return 0;
}

bool AuthController::IsAuthSchemeDisabled(Auth::Scheme scheme) const {
  // TODO
  return false;
}

void DisableAuthScheme(Auth::Scheme scheme) {
  // TODO
}

} // namespace sippet