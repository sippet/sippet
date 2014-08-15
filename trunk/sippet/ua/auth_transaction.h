// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_TRANSACTION_H_
#define SIPPET_UA_AUTH_TRANSACTION_H_

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "net/base/completion_callback.h"
#include "net/base/net_log.h"
#include "sippet/ua/password_handler.h"

namespace sippet {

class Request;
class Response;
class AuthCache;
class AuthController;
class AuthHandlerFactory;

// The AuthTransaction collects password from the user, when needed, and also
// waits for asynchronous authentication schemes, such as GSSAPI and SSPI.
class AuthTransaction {
 public:
  AuthTransaction(AuthCache *auth_cache,
                  AuthHandlerFactory *auth_handler_factory,
                  PasswordHandler::Factory *password_handler_factory,
                  const net::BoundNetLog &bound_net_log);
  virtual ~AuthTransaction();

  int HandleChallengeAuthentication(const scoped_refptr<Response> &response,
                                    const net::CompletionCallback& callback);

 private:
  enum State {
    STATE_HANDLE_AUTH_CHALLENGE,
    STATE_HANDLE_AUTH_CHALLENGE_COMPLETE,
    STATE_GET_CREDENTIALS,
    STATE_GET_CREDENTIALS_COMPLETE,
    STATE_ADD_AUTHORIZATION_HEADERS,
    STATE_ADD_AUTHORIZATION_HEADERS_COMPLETE,
    STATE_NONE,
  };

  void OnIOComplete(int result);

  int DoLoop(int last_io_result);
  int DoHandleAuthChallenge();
  int DoHandleAuthChallengeComplete();
  int DoGetCredentials();
  int DoGetCredentialsComplete(int result);
  int DoAddAuthorizationHeaders();
  int DoAddAuthorizationHeadersComplete(int result);

  void RunUserCallback(int status);

  State next_state_;

  net::CompletionCallback callback_;
  net::BoundNetLog bound_net_log_;

  scoped_refptr<Request> request_;
  scoped_refptr<Response> response_;
  scoped_refptr<AuthController> auth_controller_;
  PasswordHandler::Factory *password_handler_factory_;
  scoped_ptr<PasswordHandler> password_handler_;

  string16 username_;
  string16 password_;

  DISALLOW_COPY_AND_ASSIGN(AuthTransaction);
};

}  // namespace sippet

#endif // SIPPET_UA_AUTH_TRANSACTION_H_

