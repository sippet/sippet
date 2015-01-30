// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_HANDLER_MOCK_H_
#define SIPPET_UA_AUTH_HANDLER_MOCK_H_

#include <utility>
#include <deque>

#include "sippet/message/message.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_handler_factory.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace sippet {

// AuthHandlerMock is used in tests to reliably trigger edge cases.
class AuthHandlerMock : public AuthHandler {
 public:
  // The Factory class returns handlers in the order they were added via
  // AddMockHandler.
  class Factory : public AuthHandlerFactory {
   public:
    Factory();
    virtual ~Factory();

    void AddMockHandler(AuthHandler* handler, Auth::Target target,
      bool call_init_from_challenge);

    // AuthHandlerFactory:
    virtual int CreateAuthHandler(
        const Challenge &challenge,
        Auth::Target target,
        const GURL& origin,
        CreateReason create_reason,
        int digest_nonce_count,
        const net::BoundNetLog& net_log,
        scoped_ptr<AuthHandler>* handler) override;

   private:
    typedef std::deque<std::pair<bool, AuthHandler*> > Handlers;
    Handlers handlers_[net::HttpAuth::AUTH_NUM_TARGETS];
  };

  AuthHandlerMock();
  virtual ~AuthHandlerMock();

  MOCK_METHOD1(HandleAnotherChallenge, Auth::AuthorizationResult(
      const Challenge& challenge));

  MOCK_METHOD3(GenerateAuthImpl, int(
      const net::AuthCredentials* credentials,
      const scoped_refptr<Request> &request,
      const net::CompletionCallback& callback));

  void set_allows_default_credentials(bool value);
  void set_allows_explicit_credentials(bool value);
  void set_needs_identity(bool value);

  virtual bool AllowsDefaultCredentials();
  virtual bool AllowsExplicitCredentials();
  virtual bool NeedsIdentity();

  virtual bool Init(const Challenge& challenge) override;

private:
  typedef std::pair<bool,bool> optional;

  optional allows_default_credentials_;
  optional allows_explicit_credentials_;
  optional needs_identity_;
};

} // namespace sippet

#endif  // SIPPET_UA_AUTH_HANDLER_MOCK_H_
