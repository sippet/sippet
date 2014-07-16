// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_HANDLER_H_
#define SIPPET_UA_AUTH_HANDLER_H_

#include "sippet/ua/auth.h"
#include "net/base/net_log.h"
#include "net/base/completion_callback.h"

namespace sippet {

// Based on net/http/http_auth_handler.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Request;

// AuthHandler is the interface for the authentication schemes
// (basic, digest, NTLM, Negotiate).
// AuthHandler objects are typically created by an AuthHandlerFactory.
class AuthHandler {
 public:
  AuthHandler();
  virtual ~AuthHandler();

  // Initializes the handler using a challenge issued by a server.
  // |challenge| must be non-NULL. |target| and |origin| are both stored
  // for later use, and are not part of the initial challenge.
  bool InitFromChallenge(const Challenge& challenge,
                         Auth::Target target,
                         const GURL& origin,
                         const net::BoundNetLog& net_log);

  // Determines how the previous authorization attempt was received.
  //
  // This is called when the server/proxy responds with a 401/407 after an
  // earlier authorization attempt. Although this normally means that the
  // previous attempt was rejected, in multi-round schemes such as
  // NTLM+Negotiate it may indicate that another round of challenge+response
  // is required. For Digest authentication it may also mean that the previous
  // attempt used a stale nonce (and nonce-count) and that a new attempt should
  // be made with a different nonce provided in the challenge.
  //
  // |challenge| must be non-NULL.
  virtual Auth::AuthorizationResult HandleAnotherChallenge(
      const Challenge& challenge) = 0;

  // Generates an authentication token and add to the given request,
  // potentially asynchronously.
  //
  // When |credentials| is NULL, the default credentials for the currently
  // logged in user are used. |AllowsDefaultCredentials()| MUST be true in this
  // case.
  //
  // |request|, |callback|, and |auth_token| must be non-NULL.
  //
  // The return value is a net error code.
  //
  // If |OK| is returned, |request| is filled in with an authentication.
  //
  // If |ERR_IO_PENDING| is returned, |request| will be filled in
  // asynchronously and |callback| will be invoked. The lifetime of
  // |request|, |callback|, and |auth_token| must last until |callback| is
  // invoked, but |credentials| is only used during the initial call.
  //
  // All other return codes indicate that there was a problem generating a
  // token, and the |request| is not modified.
  int GenerateAuth(const net::AuthCredentials* credentials,
                   const scoped_refptr<Request> &request,
                   const net::CompletionCallback& callback);

  // The authentication scheme as an enumerated value.
  Auth::Scheme auth_scheme() const {
    return auth_scheme_;
  }

  // The realm, encoded as UTF-8. This may be empty.
  const std::string& realm() const {
    return realm_;
  }

  // Numeric rank based on the challenge's security level. Higher
  // numbers are better. Used by Auth::ChooseBestChallenge().
  int score() const {
    return score_;
  }

  // The target type when the handler was created.
  Auth::Target target() const {
    return target_;
  }

  // Returns the proxy or server which issued the authentication challenge
  // that this AuthHandler is handling. The URI includes only scheme, host
  // port and protocol.
  const GURL& origin() const {
    return origin_;
  }

  // Returns whether the default credentials may be used for the |origin| passed
  // into |InitFromChallenge|. If true, the user does not need to be prompted
  // for username and password to establish credentials.
  virtual bool AllowsDefaultCredentials();

  // Returns whether explicit credentials can be used with this handler.  If
  // true the user may be prompted for credentials if an implicit identity
  // cannot be determined.
  virtual bool AllowsExplicitCredentials();

 protected:
  // Initializes the handler using a challenge issued by a server.
  // Implementations are expected to initialize the following members:
  // scheme_, realm_ and score_
  virtual bool Init(const Challenge& challenge) = 0;

  // |GenerateAuthImpl()} is the auth-scheme specific implementation
  // of generating the next auth token. Callers should use |GenerateAuth()|
  // which will in turn call |GenerateAuthImpl()|
  virtual int GenerateAuthImpl(
      const net::AuthCredentials* credentials,
      const scoped_refptr<Request> &request,
      const net::CompletionCallback& callback) = 0;

  // The auth-scheme as an enumerated value.
  Auth::Scheme auth_scheme_;

  // The realm, encoded as UTF-8. Used by "basic" and "digest".
  std::string realm_;

  // The {scheme, host, port} for the authentication target.  Used by "ntlm"
  // and "negotiate" to construct the service principal name.
  GURL origin_;

  // The score for this challenge. Higher numbers are better.
  int score_;

  // Whether this authentication request is for a proxy server, or an
  // origin server.
  Auth::Target target_;

  net::BoundNetLog net_log_;

 private:
  void OnGenerateAuthComplete(int rv);
  void FinishGenerateAuth();

  net::CompletionCallback callback_;
};

} // namespace sippet

#endif  // SIPPET_UA_AUTH_HANDLER_H_