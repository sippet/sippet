// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_CONTROLLER_H_
#define SIPPET_UA_AUTH_CONTROLLER_H_

#include "base/memory/ref_counted.h"
#include "net/base/auth.h"
#include "net/base/net_log.h"
#include "net/base/completion_callback.h"
#include "sippet/ua/auth.h"

namespace sippet {

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

  // Generate an authentication token for given request if necessary. The
  // return value is a net error code. |OK| will be returned both in the case
  // that a token is correctly generated synchronously, as well as when no
  // tokens were necessary.
  int MaybeGenerateAuthToken(const scoped_refptr<Request> &request,
                             const net::CompletionCallback& callback,
                             const net::BoundNetLog& net_log);

  // Adds either the proxy auth header, or the origin server auth header. It
  // must be called only when |HaveAuth| is true.
  void AddAuthorizationHeaders(const scoped_refptr<Request> &request);

  // Checks for and handles SIP status code 401 or 407. |HandleAuthChallenge()|
  // returns OK on success, or a network error code otherwise.
  int HandleAuthChallenge(const scoped_refptr<Response> &response,
                          const net::BoundNetLog& net_log);

  // Check whether the controller has any authentication pending.
  bool HaveAuth() const;

  // Take the authentication challenge information.
  scoped_refptr<net::AuthChallengeInfo> auth_info();

  // Check whether some of the available authentication scheme has been
  // disabled.
  bool IsAuthSchemeDisabled(Auth::Scheme scheme) const;

  // Disable a given authentication scheme.
  void DisableAuthScheme(Auth::Scheme scheme);
};

}  // namespace net

#endif // SIPPET_UA_AUTH_CONTROLLER_H_
