// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_mock.h"
#include "net/base/net_errors.h"
#include "base/stl_util.h"

namespace sippet {

AuthHandlerMock::Factory::Factory() {}

AuthHandlerMock::Factory::~Factory() {}

void AuthHandlerMock::Factory::AddMockHandler(
    std::unique_ptr<AuthHandler> handler, Auth::Target target,
    bool call_init_from_challenge) {
  DCHECK(target != net::HttpAuth::AUTH_NONE);
  handlers_[target].push_back(
    std::make_pair(call_init_from_challenge, std::move(handler)));
}

int AuthHandlerMock::Factory::CreateAuthHandler(
        const Challenge &challenge,
        Auth::Target target,
        const GURL& origin,
        CreateReason create_reason,
        int digest_nonce_count,
        const net::NetLogWithSource& net_log,
        std::unique_ptr<AuthHandler>* handler) {
  if (handlers_[target].empty())
    return net::ERR_UNEXPECTED;
  bool call_init_from_challenge = handlers_[target].front().first;
  std::unique_ptr<AuthHandler> tmp_handler(
      std::move(handlers_[target].front().second));
  handlers_[target].pop_front();
  if (call_init_from_challenge &&
      !tmp_handler->InitFromChallenge(challenge, target, origin, net_log))
    return net::ERR_INVALID_RESPONSE;
  handler->swap(tmp_handler);
  return net::OK;
}

AuthHandlerMock::AuthHandlerMock()
  : allows_default_credentials_(optional(false, false)),
    allows_explicit_credentials_(optional(false, false)),
    needs_identity_(optional(false, false)) {
}

AuthHandlerMock::~AuthHandlerMock() {
}

void AuthHandlerMock::set_allows_default_credentials(bool value) {
  allows_default_credentials_ = optional(true, value);
}

void AuthHandlerMock::set_allows_explicit_credentials(bool value) {
  allows_explicit_credentials_ = optional(true, value);
}

void AuthHandlerMock::set_needs_identity(bool value) {
  needs_identity_ = optional(true, value);
}

bool AuthHandlerMock::AllowsDefaultCredentials() {
  if (allows_default_credentials_.first)
    return allows_default_credentials_.second;
  return AuthHandler::AllowsDefaultCredentials();
}

bool AuthHandlerMock::AllowsExplicitCredentials() {
  if (allows_explicit_credentials_.first)
    return allows_explicit_credentials_.second;
  return AuthHandler::AllowsExplicitCredentials();
}

bool AuthHandlerMock::NeedsIdentity() {
  if (needs_identity_.first)
    return needs_identity_.second;
  return AuthHandler::NeedsIdentity();
}

bool AuthHandlerMock::Init(const Challenge& challenge) {
  auth_scheme_ = net::HttpAuth::AUTH_SCHEME_MOCK;
  score_ = 1;
  return true;
}

}  // namespace sippet
