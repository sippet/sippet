// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_transaction.h"

#include "net/base/net_errors.h"
#include "sippet/message/message.h"
#include "sippet/ua/auth_controller.h"
#include "sippet/ua/dialog.h"

namespace sippet {

AuthTransaction::AuthTransaction(AuthCache *auth_cache,
    AuthHandlerFactory *auth_handler_factory,
    PasswordHandler::Factory *password_handler_factory,
    const net::BoundNetLog &bound_net_log) :
  auth_controller_(new AuthController(auth_cache, auth_handler_factory)),
  next_state_(STATE_NONE),
  bound_net_log_(bound_net_log),
  password_handler_factory_(password_handler_factory) {
  DCHECK(password_handler_factory_);
}

AuthTransaction::~AuthTransaction() {
}

int AuthTransaction::HandleChallengeAuthentication(
    const scoped_refptr<Request> &outgoing_request,
    const scoped_refptr<Response> &incoming_response,
    const net::CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  callback_ = callback;
  incoming_response_ = incoming_response;
  outgoing_request_ = outgoing_request;
  next_state_ = STATE_HANDLE_AUTH_CHALLENGE;
  return DoLoop(net::OK);
}

void AuthTransaction::OnIOComplete(int result) {
  DCHECK_NE(STATE_NONE, next_state_);
  int rv = DoLoop(result);
  if (rv != net::ERR_IO_PENDING)
    RunUserCallback(rv);
}

int AuthTransaction::DoLoop(int last_io_result) {
  DCHECK_NE(next_state_, STATE_NONE);
  int rv = last_io_result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_HANDLE_AUTH_CHALLENGE:
        DCHECK_EQ(net::OK, rv);
        rv = DoHandleAuthChallenge();
        break;
      case STATE_HANDLE_AUTH_CHALLENGE_COMPLETE:
        DCHECK_EQ(net::OK, rv);
        rv = DoHandleAuthChallengeComplete();
        break;
      case STATE_GET_CREDENTIALS:
        DCHECK_EQ(net::OK, rv);
        rv = DoGetCredentials();
        break;
      case STATE_GET_CREDENTIALS_COMPLETE:
        rv = DoGetCredentialsComplete(rv);
        break;
      case STATE_ADD_AUTHORIZATION_HEADERS:
        DCHECK_EQ(net::OK, rv);
        rv = DoAddAuthorizationHeaders();
        break;
      case STATE_ADD_AUTHORIZATION_HEADERS_COMPLETE:
        rv = DoAddAuthorizationHeadersComplete(rv);
        break;
      default:
        NOTREACHED() << "bad state";
        rv = net::ERR_UNEXPECTED;
        break;
    }
  } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);
  return rv;
}

int AuthTransaction::DoHandleAuthChallenge() {
  next_state_ = STATE_HANDLE_AUTH_CHALLENGE_COMPLETE;
  return auth_controller_->HandleAuthChallenge(incoming_response_,
      bound_net_log_);
}

int AuthTransaction::DoHandleAuthChallengeComplete() {
  if (!auth_controller_->HaveAuthHandler())
    return net::ERR_UNSUPPORTED_AUTH_SCHEME;
  if (auth_controller_->auth_info()) {
    next_state_ = STATE_GET_CREDENTIALS;
    return net::OK;
  } else {
    next_state_ = STATE_ADD_AUTHORIZATION_HEADERS;
    return net::OK;
  }
}

int AuthTransaction::DoGetCredentials() {
  DCHECK(auth_controller_->auth_info());
  next_state_ = STATE_GET_CREDENTIALS_COMPLETE;
  scoped_ptr<PasswordHandler> password_handler =
      password_handler_factory_->CreatePasswordHandler();
  return password_handler->GetCredentials(
      auth_controller_->auth_info().get(),
      &username_,
      &password_,
      base::Bind(&AuthTransaction::OnIOComplete,
          base::Unretained(this)));
}

int AuthTransaction::DoGetCredentialsComplete(int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);
  if (result == net::OK) {
    auth_controller_->ResetAuth(net::AuthCredentials(username_, password_));
    next_state_ = STATE_ADD_AUTHORIZATION_HEADERS;
    return net::OK;
  } else {
    // The user failed to provide valid credentials
    next_state_ = STATE_NONE;
    return result;
  }
}

int AuthTransaction::DoAddAuthorizationHeaders() {
  next_state_ = STATE_ADD_AUTHORIZATION_HEADERS_COMPLETE;
  return auth_controller_->AddAuthorizationHeaders(outgoing_request_,
      base::Bind(&AuthTransaction::OnIOComplete,
          base::Unretained(this)), bound_net_log_);
}

int AuthTransaction::DoAddAuthorizationHeadersComplete(int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);
  next_state_ = STATE_NONE;
  return result;
}

void AuthTransaction::RunUserCallback(int status) {
  DCHECK(!callback_.is_null());
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(status);
}

}  // namespace sippet

