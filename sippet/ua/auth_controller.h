// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_CONTROLLER_H_
#define SIPPET_UA_AUTH_CONTROLLER_H_

#include "base/memory/ref_counted.h"
#include "net/base/auth.h"
#include "net/log/net_log.h"
#include "net/base/completion_callback.h"
#include "sippet/ua/auth.h"

namespace sippet {

// Based on net/http/http_auth_controller.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Request;
class Response;
class AuthCache;
class AuthHandlerFactory;

class AuthController :
  public base::RefCountedThreadSafe<AuthController> {
 public:
  // The arguments are self explanatory except possibly for |auth_url|, which
  // should be both the auth target and auth path in a single url argument.
  AuthController(AuthCache* auth_cache,
                 AuthHandlerFactory* auth_handler_factory);

  // Checks for and handles SIP status code 401 or 407. |HandleAuthChallenge()|
  // returns OK on success, or a network error code otherwise.
  int HandleAuthChallenge(const scoped_refptr<Response> &response,
                          const net::BoundNetLog& net_log);

  // Store the supplied credentials and prepare to restart the auth.
  void ResetAuth(const net::AuthCredentials& credentials);

  // Adds either the proxy auth header, or the origin server auth header to
  // the given request. The return value is a net error code. |OK| will be
  // returned both in the case that a token is correctly generated
  // synchronously, as well as when no tokens were necessary.
  int AddAuthorizationHeaders(const scoped_refptr<Request> &request,
                              const net::CompletionCallback& callback,
                              const net::BoundNetLog& net_log);

  // Take the authentication challenge information.
  scoped_refptr<net::AuthChallengeInfo> auth_info();

  // Check whether the controller has any authentication pending.
  bool HaveAuth() const;

  // Check whether the controller has chosen an authentication handler.
  bool HaveAuthHandler() const;

  // Check whether some of the available authentication scheme has been
  // disabled.
  bool IsAuthSchemeDisabled(Auth::Scheme scheme) const;

  // Disable a given authentication scheme.
  void DisableAuthScheme(Auth::Scheme scheme);

  // Returns the current controller target.
  Auth::Target target() const {
    return target_;
  }

 private:
  virtual ~AuthController();

  // So that we can destroy this object.
  friend class base::RefCountedThreadSafe<AuthController>;

  // Actions for InvalidateCurrentHandler()
  enum InvalidateHandlerAction {
    INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS,
    INVALIDATE_HANDLER_AND_DISABLE_SCHEME,
    INVALIDATE_HANDLER
  };

  // Invalidates the current handler.  If |action| is
  // INVALIDATE_HANDLER_AND_CACHED_CREDENTIALS, then also invalidate
  // the cached credentials used by the handler.
  void InvalidateCurrentHandler(InvalidateHandlerAction action);

  // Invalidates any auth cache entries after authentication has failed.
  // The identity that was rejected is |identity_|.
  void InvalidateRejectedAuthFromCache();

  // Sets |identity_| to the next identity that the transaction should try. It
  // chooses candidates by searching the auth cache and the URL for a
  // username:password. Returns true if an identity was found.
  bool SelectNextAuthIdentityToTry();

  // Populates auth_info_ with the challenge information, so that
  // credentials can be prompted.
  void PopulateAuthChallenge();

  // If |result| indicates a permanent failure, disables the current
  // auth scheme for this controller and returns true.  Returns false
  // otherwise.
  bool DisableOnAuthHandlerResult(int result);

  void OnIOComplete(int result);

  // Indicates if this handler is for Proxy auth or Server auth. It's
  // initialized when handling challenges. It's permitted to receive one or
  // more Proxy challenges, but only one Server challenge can be performed.
  Auth::Target target_;

  // Holds the {scheme, host, port} for the authentication target.
  GURL auth_origin_;

  // |handler_| encapsulates the logic for the particular auth-scheme.
  // This includes the challenge's parameters. If NULL, then there is no
  // associated auth handler.
  scoped_ptr<AuthHandler> handler_;

  // |identity_| holds the credentials that should be used by
  // the handler_ to generate challenge responses. This identity can come from
  // a number of places (url, cache, prompt).
  Auth::Identity identity_;

  // Contains information about the auth challenge.
  scoped_refptr<net::AuthChallengeInfo> auth_info_;

  // True if default credentials have already been tried for this transaction
  // in response to an HTTP authentication challenge.
  bool default_credentials_used_;

  // These two are owned by the HttpNetworkSession/IOThread, which own the
  // objects which reference |this|.  Therefore, these raw pointers are valid
  // for the lifetime of this object.
  AuthCache* const auth_cache_;
  AuthHandlerFactory* const auth_handler_factory_;

  std::set<Auth::Scheme> disabled_schemes_;

  net::CompletionCallback callback_;
};

}  // namespace net

#endif // SIPPET_UA_AUTH_CONTROLLER_H_
