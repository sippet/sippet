// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_H_
#define SIPPET_UA_AUTH_H_

#include <set>
#include "net/base/auth.h"
#include "net/base/net_log.h"
#include "net/http/http_auth.h"
#include "sippet/message/header.h"

namespace sippet {

// Based on net/http/http_auth.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Response;
class AuthHandler;
class AuthHandlerFactory;
class Challenge;

// Utility class for SIP authentication.
struct Auth {
  // Http authentication can be done the the proxy server, origin server,
  // or both. This enum tracks who the target is.
  typedef net::HttpAuth::Target Target;

  // What the WWW-Authenticate/Proxy-Authenticate headers indicate about
  // the previous authorization attempt.
  typedef net::HttpAuth::AuthorizationResult AuthorizationResult;

  // Describes where the identity used for authentication came from.
  typedef net::HttpAuth::IdentitySource IdentitySource;

  // Helper structure used to track the current identity being used
  // for authorization.
  typedef net::HttpAuth::Identity Identity;

  // The authentication scheme
  typedef net::HttpAuth::Scheme Scheme;

  // Returns the challenge header type corresponding to the given
  // authentication target.
  static Header::Type GetChallengeHeaderType(Target target);

  // Returns the authorization header type corresponding to the given
  // authentication target.
  static Header::Type GetAuthorizationHeaderType(Target target);

  // Returns a string representation of an authentication Scheme.
  static const char* SchemeToString(Scheme scheme);

  // Returns the authentication target available in the given response.
  static Target GetChallengeTarget(const scoped_refptr<Response> &response);

  // Returns the challenge from a given authenticate header.
  static Challenge& GetChallengeFromHeader(Header* header);

  // Returns the response origin.
  static GURL GetResponseOrigin(const scoped_refptr<Response>& response);

  // Iterate through the challenge headers, and pick the best one that
  // we support. Obtains the implementation class for handling the challenge,
  // and passes it back in |*handler|. If no supported challenge was found,
  // |*handler| is set to NULL.
  //
  // |target| is discovered from the challenge contained in the response.
  //
  // |disabled_schemes| is the set of schemes that we should not use.
  static void ChooseBestChallenge(
      AuthHandlerFactory* auth_handler_factory,
      const scoped_refptr<Response> &response,
      const std::set<Scheme>& disabled_schemes,
      const net::BoundNetLog& net_log,
      scoped_ptr<AuthHandler>* handler);

  // Handle a 401/407 response from a server/proxy after a previous
  // authentication attempt. For connection-based authentication schemes, the
  // new response may be another round in a multi-round authentication sequence.
  // For request-based schemes, a 401/407 response is typically treated like a
  // rejection of the previous challenge, except in the Digest case when a
  // "stale" attribute is present. The request referred by the response 
  //
  // |handler| must be non-NULL, and is the HttpAuthHandler from the previous
  // authentication round.
  //
  // |headers| must be non-NULL and contain the new HTTP response.
  //
  // |target| specifies whether the authentication challenge response came
  // from a server or a proxy.
  //
  // |disabled_schemes| are the authentication schemes to ignore.
  //
  // |challenge_used| is the text of the authentication challenge used in
  // support of the returned AuthorizationResult. If no headers were used for
  // the result (for example, all headers have unknown authentication schemes),
  // the value is cleared.
  static AuthorizationResult HandleChallengeResponse(
      AuthHandler* handler,
      const scoped_refptr<Response> &response,
      const std::set<Scheme>& disabled_schemes);
};

}  // namespace sippet

#endif // SIPPET_UA_AUTH_H_
