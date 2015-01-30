// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_HANDLER_DIGEST_H_
#define SIPPET_UA_AUTH_HANDLER_DIGEST_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "sippet/message/headers/authorization.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_handler_factory.h"

namespace sippet {

// Based on net/http/http_auth_handler_digest.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Code for handling http digest authentication.
class AuthHandlerDigest : public AuthHandler {
 public:
  // A NonceGenerator is a simple interface for generating client nonces.
  // Unit tests can override the default client nonce behavior with fixed
  // nonce generation to get reproducible results.
  class NonceGenerator {
   public:
    NonceGenerator();
    virtual ~NonceGenerator();

    // Generates a client nonce.
    virtual std::string GenerateNonce() const = 0;
   private:
    DISALLOW_COPY_AND_ASSIGN(NonceGenerator);
  };

  // DynamicNonceGenerator does a random shuffle of 16
  // characters to generate a client nonce.
  class DynamicNonceGenerator : public NonceGenerator {
   public:
    DynamicNonceGenerator();
    virtual std::string GenerateNonce() const OVERRIDE;
   private:
    DISALLOW_COPY_AND_ASSIGN(DynamicNonceGenerator);
  };

  // FixedNonceGenerator always uses the same string specified at
  // construction time as the client nonce.
  class FixedNonceGenerator : public NonceGenerator {
   public:
    explicit FixedNonceGenerator(const std::string& nonce);

    virtual std::string GenerateNonce() const OVERRIDE;

   private:
    const std::string nonce_;
    DISALLOW_COPY_AND_ASSIGN(FixedNonceGenerator);
  };

  class Factory : public AuthHandlerFactory {
   public:
    Factory();
    virtual ~Factory();

    // This factory owns the passed in |nonce_generator|.
    void set_nonce_generator(const NonceGenerator* nonce_generator);

    virtual int CreateAuthHandler(
        const Challenge &challenge,
        Auth::Target target,
        const GURL& origin,
        CreateReason create_reason,
        int digest_nonce_count,
        const net::BoundNetLog& net_log,
        scoped_ptr<AuthHandler>* handler) OVERRIDE;

   private:
    scoped_ptr<const NonceGenerator> nonce_generator_;
  };

  virtual Auth::AuthorizationResult HandleAnotherChallenge(
      const Challenge& challenge) OVERRIDE;

 protected:
  virtual bool Init(const Challenge& challenge) OVERRIDE;

  virtual int GenerateAuthImpl(
      const net::AuthCredentials* credentials,
      const scoped_refptr<Request> &request,
      const net::CompletionCallback& callback) OVERRIDE;

 private:
  // Possible values for the "algorithm" property.
  enum DigestAlgorithm {
    // No algorithm was specified. According to RFC 2617 this means
    // we should default to ALGORITHM_MD5.
    ALGORITHM_UNSPECIFIED,

    // Hashes are run for every request.
    ALGORITHM_MD5,

    // Hash is run only once during the first WWW-Authenticate handshake.
    // (SESS means session).
    ALGORITHM_MD5_SESS,
  };

  // Possible values for QualityOfProtection.
  enum QualityOfProtection {
    QOP_UNSPECIFIED,
    QOP_AUTH,
    QOP_AUTH_INT,
  };

  // |nonce_count| indicates how many times the server-specified nonce has
  // been used so far.
  // |nonce_generator| is used to create a client nonce, and is not owned by
  // the handler. The lifetime of the |nonce_generator| must exceed that of this
  // handler.
  AuthHandlerDigest(int nonce_count, const NonceGenerator* nonce_generator);
  virtual ~AuthHandlerDigest();

  // Parse the challenge, saving the results into this instance.
  // Returns true on success.
  bool ParseChallenge(const Challenge& challenge);

  // Generates a random string, to be used for client-nonce.
  static std::string GenerateNonce();

  // Convert enum value back to string.
  static std::string QopToString(QualityOfProtection qop);

  // Convert enum value back to Credentials types.
  static Credentials::Algorithm AlgorithmToCredentials(
    DigestAlgorithm algorithm);

  // Extract the method and path of the request, as needed by
  // the 'A2' production. (path may be a hostname for proxy).
  void GetRequestMethodAndRequestUri(const scoped_refptr<Request> &request,
                                     std::string* method,
                                     std::string* request_uri) const;

  // Build up  the 'response' production.
  std::string AssembleResponseDigest(const std::string& method,
                                     const std::string& request_uri,
                                     const std::string& body,
                                     const net::AuthCredentials& credentials,
                                     const std::string& cnonce,
                                     int nonce_count) const;

  // Build up the value for (Authorization/Proxy-Authorization).
  void AssembleCredentials(const std::string& method,
                           const std::string& request_uri,
                           const net::AuthCredentials& credentials,
                           const std::string& cnonce,
                           int nonce_count,
                           const scoped_refptr<Request> &request) const;

  // Information parsed from the challenge.
  std::string nonce_;
  std::string domain_;
  std::string opaque_;
  bool stale_;
  DigestAlgorithm algorithm_;
  QualityOfProtection qop_;

  // The realm as initially encoded over-the-wire. This is used in the
  // challenge text, rather than |realm_| which has been converted to
  // UTF-8.
  std::string original_realm_;

  int nonce_count_;
  const NonceGenerator* nonce_generator_;
};

} // namespace sippet

#endif  // SIPPET_UA_AUTH_HANDLER_DIGEST_H_
