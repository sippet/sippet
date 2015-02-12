// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_controller.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_cache.h"
#include "sippet/message/message.h"
#include "sippet/message/status_code.h"
#include "net/base/net_errors.h"
#include "base/threading/platform_thread.h"

namespace sippet {

// Based on net/http/http_auth_controller.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

AuthController::AuthController(
    AuthCache* auth_cache,
    AuthHandlerFactory* auth_handler_factory)
    : target_(net::HttpAuth::AUTH_NONE),
      auth_cache_(auth_cache),
      auth_handler_factory_(auth_handler_factory),
      default_credentials_used_(false) {
}

AuthController::~AuthController() {
}

int AuthController::HandleAuthChallenge(
    const scoped_refptr<Response> &response,
    const net::BoundNetLog& net_log) {
  DCHECK(response.get());

  // If response code is 401, then it's a Server auth; otherwise,
  // if it's 407, then it's a Proxy auth.
  Auth::Target intended_target;
  int response_code = response->response_code();
  switch (response_code) {
    case SIP_UNAUTHORIZED:
      intended_target = net::HttpAuth::AUTH_SERVER;
      break;
    case SIP_PROXY_AUTHENTICATION_REQUIRED:
      intended_target = net::HttpAuth::AUTH_PROXY;
      break;
    default:
      NOTREACHED();
      return net::OK;
  }

  // Double check the incoming headers: if None, then it's a failure; if Proxy
  // auth, then the current auth cannot be Server auth; if Server auth, current
  // auth must be Proxy auth (empty current credentials) or Server auth.
  Auth::Target target = Auth::GetChallengeTarget(response);
  if (net::HttpAuth::AUTH_NONE == target) {
    // We found no challenge on the response -- let the transaction continue
    // so the app ends up displaying an error message.
    InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
    return net::OK;
  }

  // Response code must match the current credentials header
  if (intended_target != target) {
    // The response code doesn't match the challenge contained in the response
    return net::ERR_UNEXPECTED;
  }

  switch (target_) {
    case net::HttpAuth::AUTH_PROXY:
      if (net::HttpAuth::AUTH_SERVER == target) {
        // We have transitioned from a Proxy to a Server, so empty handler
        // and cache; the authentication origin may have been changed.
        auth_origin_ = Auth::GetResponseOrigin(response);
        InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
      }
      break;
    case net::HttpAuth::AUTH_SERVER:
      if (net::HttpAuth::AUTH_PROXY == target)
        return net::ERR_UNEXPECTED_PROXY_AUTH;
      break;
    default:
      auth_origin_ = Auth::GetResponseOrigin(response);
  }
  
  target_ = target;

  // Give the existing auth handler first try at the authentication headers.
  // This will also evict the entry in the HttpAuthCache if the previous
  // challenge appeared to be rejected, or is using a stale nonce in the Digest
  // case.
  if (HaveAuth()) {
    Auth::AuthorizationResult result =
        Auth::HandleChallengeResponse(handler_.get(),
                                      response,
                                      disabled_schemes_);
    switch (result) {
      case net::HttpAuth::AUTHORIZATION_RESULT_ACCEPT:
        break;
      case net::HttpAuth::AUTHORIZATION_RESULT_INVALID:
        InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
        break;
      case net::HttpAuth::AUTHORIZATION_RESULT_REJECT:
        InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
        break;
      case net::HttpAuth::AUTHORIZATION_RESULT_STALE:
        if (auth_cache_->UpdateStaleChallenge(handler_->realm(),
                                              handler_->auth_scheme())) {
          InvalidateCurrentHandler(INVALIDATE_HANDLER);
        } else {
          // It's possible that a server could incorrectly issue a stale
          // response when the entry is not in the cache. Just evict the
          // current value from the cache.
          InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
        }
        break;
      case net::HttpAuth::AUTHORIZATION_RESULT_DIFFERENT_REALM:
        // If the server changes the authentication realm in a
        // subsequent challenge, invalidate cached credentials for the
        // previous realm.
        InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS);
        break;
      default:
        NOTREACHED();
        break;
    }
  }

  identity_.invalid = true;

  do {
    if (!handler_.get()) {
      // Find the best authentication challenge that we support.
      Auth::ChooseBestChallenge(auth_handler_factory_,
                                response,
                                disabled_schemes_,
                                net_log,
                                &handler_);
      if (!handler_.get()) {
        // We found no supported challenge -- let the transaction continue so
        // the app ends up displaying an error page.
        return net::OK;
      }
    }

    if (handler_->NeedsIdentity()) {
      // Pick a new auth identity to try, by looking to the realm and auth
      // cache. If an identity to try is found, it is saved to identity_.
      SelectNextAuthIdentityToTry();
    } else {
      // Proceed with the existing identity or a null identity.
      identity_.invalid = false;
    }

    // From this point on, we are restartable.

    if (identity_.invalid) {
      // We have exhausted all identity possibilities.
      if (!handler_->AllowsExplicitCredentials()) {
        // If the handler doesn't accept explicit credentials, then we need to
        // choose a different auth scheme.
        InvalidateCurrentHandler(INVALIDATE_HANDLER_AND_DISABLE_SCHEME);
      } else {
        // Pass the challenge information back to the client.
        PopulateAuthChallenge();
      }
    } else {
      auth_info_ = nullptr;
    }

    // If we get here and we don't have a handler_, that's because we
    // invalidated it due to not having any viable identities to use with it. Go
    // back and try again.
  } while(!handler_.get());
  return net::OK;
}

void AuthController::ResetAuth(const net::AuthCredentials& credentials) {
  DCHECK(identity_.invalid || credentials.Empty());

  if (identity_.invalid) {
    // Update the credentials.
    identity_.source = net::HttpAuth::IDENT_SRC_EXTERNAL;
    identity_.invalid = false;
    identity_.credentials = credentials;
  }

  // Add the auth entry to the cache before restarting. We don't know whether
  // the identity is valid yet, but if it is valid we want other transactions
  // to know about it. If an entry for (origin, handler->realm()) already
  // exists, we update it.
  //
  // If identity_.source is net::HttpAuth::IDENT_SRC_NONE or
  // net::HttpAuth::IDENT_SRC_DEFAULT_CREDENTIALS, identity_ contains no
  // identity because identity is not required yet or we're using default
  // credentials.
  switch (identity_.source) {
    case net::HttpAuth::IDENT_SRC_NONE:
    case net::HttpAuth::IDENT_SRC_DEFAULT_CREDENTIALS:
      break;
    default:
      auth_cache_->Add(handler_->realm(), handler_->auth_scheme(),
                       identity_.credentials);
      break;
  }
}

int AuthController::AddAuthorizationHeaders(
    const scoped_refptr<Request> &request,
    const net::CompletionCallback& callback,
    const net::BoundNetLog& net_log) {
  const net::AuthCredentials* credentials = nullptr;
  if (identity_.source != net::HttpAuth::IDENT_SRC_DEFAULT_CREDENTIALS)
    credentials = &identity_.credentials;
  DCHECK(callback_.is_null());
  int rv = handler_->GenerateAuth(
      credentials, request,
      base::Bind(&AuthController::OnIOComplete, base::Unretained(this)));
  if (DisableOnAuthHandlerResult(rv))
    rv = net::OK;
  if (rv == net::ERR_IO_PENDING)
    callback_ = callback;
  else
    OnIOComplete(rv);
  return rv;
}

scoped_refptr<net::AuthChallengeInfo> AuthController::auth_info() {
  return auth_info_;
}

bool AuthController::HaveAuth() const {
  return HaveAuthHandler() && !identity_.invalid;
}

bool AuthController::HaveAuthHandler() const {
  return handler_.get() != nullptr;
}

bool AuthController::IsAuthSchemeDisabled(Auth::Scheme scheme) const {
  return disabled_schemes_.find(scheme) != disabled_schemes_.end();
}

void AuthController::DisableAuthScheme(Auth::Scheme scheme) {
  disabled_schemes_.insert(scheme);
}

void AuthController::InvalidateCurrentHandler(
    InvalidateHandlerAction action) {
  DCHECK(handler_.get());

  if (action == INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS)
    InvalidateRejectedAuthFromCache();
  if (action == INVALIDATE_HANDLER_AND_DISABLE_SCHEME)
    DisableAuthScheme(handler_->auth_scheme());
  handler_.reset();
  identity_ = Auth::Identity();
}

void AuthController::InvalidateRejectedAuthFromCache() {
  DCHECK(HaveAuth());

  // Clear the cache entry for the identity we just failed on.
  // Note: we require the credentials to match before invalidating
  // since the entry in the cache may be newer than what we used last time.
  auth_cache_->Remove(handler_->realm(), handler_->auth_scheme(),
                      identity_.credentials);
}

bool AuthController::SelectNextAuthIdentityToTry() {
  DCHECK(handler_.get());
  DCHECK(identity_.invalid);

  // Check the auth cache for a realm entry.
  AuthCache::Entry* entry =
      auth_cache_->Lookup(handler_->realm(), handler_->auth_scheme());

  if (entry) {
    identity_.source = net::HttpAuth::IDENT_SRC_REALM_LOOKUP;
    identity_.invalid = false;
    identity_.credentials = entry->credentials();
    return true;
  }

  // Use default credentials (single sign on) if this is the first attempt
  // at identity.  Do not allow multiple times as it will infinite loop.
  // We use default credentials after checking the auth cache so that if
  // single sign-on doesn't work, we won't try default credentials for future
  // transactions.
  if (!default_credentials_used_ && handler_->AllowsDefaultCredentials()) {
    identity_.source = net::HttpAuth::IDENT_SRC_DEFAULT_CREDENTIALS;
    identity_.invalid = false;
    identity_.credentials = net::AuthCredentials();
    default_credentials_used_ = true;
    return true;
  }

  return false;
}

void AuthController::PopulateAuthChallenge() {
  // Populates auth_info_ with the authentication challenge info.

  auth_info_ = new net::AuthChallengeInfo;
  auth_info_->is_proxy = (target_ == net::HttpAuth::AUTH_PROXY);
  auth_info_->challenger = net::HostPortPair::FromURL(auth_origin_);
  auth_info_->scheme = Auth::SchemeToString(handler_->auth_scheme());
  auth_info_->realm = handler_->realm();
}

bool AuthController::DisableOnAuthHandlerResult(int result) {
  switch (result) {
    // Occurs with GSSAPI, if the user has not already logged in.
    case net::ERR_MISSING_AUTH_CREDENTIALS:

    // Can occur with GSSAPI or SSPI if the underlying library reports
    // a permanent error.
    case net::ERR_UNSUPPORTED_AUTH_SCHEME:

    // These two error codes represent failures we aren't handling.
    case net::ERR_UNEXPECTED_SECURITY_LIBRARY_STATUS:
    case net::ERR_UNDOCUMENTED_SECURITY_LIBRARY_STATUS:

    // Can be returned by SSPI if the authenticating authority or
    // target is not known.
    case net::ERR_MISCONFIGURED_AUTH_ENVIRONMENT:

      // In these cases, disable the current scheme as it cannot
      // succeed.
      DisableAuthScheme(handler_->auth_scheme());
      return true;

    default:
      return false;
  }
}

void AuthController::OnIOComplete(int result) {
  if (DisableOnAuthHandlerResult(result))
    result = net::OK;
  if (!callback_.is_null()) {
    net::CompletionCallback c = callback_;
    callback_.Reset();
    c.Run(result);
  }
}

} // namespace sippet
