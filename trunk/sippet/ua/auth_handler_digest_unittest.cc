// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/basictypes.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "sippet/message/request.h"
#include "sippet/ua/auth_handler_digest.h"
#include "sippet/uri/uri.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

namespace {

// RespondToChallenge creates an AuthHandlerDigest for the specified
// |challenge|, and generates a response to the challenge which is returned in
// |token|.
//
// The return value indicates whether the |token| was successfully created.
bool RespondToChallenge(const std::string& challenge,
                        const Method& method,
                        const std::string& request_uri,
                        const std::string& body,
                        const std::string& cnonce,
                        std::string* token) {
  // Input validation.
  if (token == NULL) {
    ADD_FAILURE() << "|token| must be non-NULL";
    return false;
  }

  token->clear();
  scoped_ptr<AuthHandlerDigest::Factory> factory(
      new AuthHandlerDigest::Factory());
  AuthHandlerDigest::NonceGenerator* nonce_generator =
      new AuthHandlerDigest::FixedNonceGenerator(cnonce);
  factory->set_nonce_generator(nonce_generator);
  scoped_ptr<AuthHandler> handler;

  scoped_ptr<Header> header(
    Header::Parse(std::string("WWW-Authenticate: " + challenge)));
  EXPECT_TRUE(header.get() != NULL);
  WwwAuthenticate *www_authenticate = dyn_cast<WwwAuthenticate>(header);
  EXPECT_TRUE(www_authenticate != NULL);

  // Create a handler for a particular challenge.
  SipURI uri_origin(request_uri);
  int rv_create = factory->CreateAuthHandler(
    *www_authenticate, net::HttpAuth::AUTH_SERVER,
    GURL(uri_origin.GetOrigin().spec()),
    AuthHandlerFactory::CREATE_CHALLENGE, 1,
    net::BoundNetLog(), &handler);
  if (rv_create != net::OK || handler.get() == NULL) {
    ADD_FAILURE() << "Unable to create auth handler.";
    return false;
  }

  // Create a token in response to the challenge.
  // NOTE: AuthHandlerDigest's implementation of GenerateAuthToken always
  // completes synchronously. That's why this test can get away with a
  // TestCompletionCallback without an IO thread.
  net::TestCompletionCallback callback;
  scoped_refptr<Request> request(
    new Request(method, GURL(request_uri)));
  if (body.length() > 0)
    request->set_content(body);
  net::AuthCredentials credentials(base::ASCIIToUTF16("bob"),
    base::ASCIIToUTF16("zanzibar"));
  int rv_generate = handler->GenerateAuth(
      &credentials, request.get(), callback.callback());
  if (rv_generate != net::OK) {
    ADD_FAILURE() << "Problems generating auth token";
    return false;
  }
  EXPECT_EQ(1, request->size());
  Authorization *authorization = request->get<Authorization>();
  EXPECT_TRUE(authorization != NULL);
  *token = authorization->ToString();
  return true;
}

}  // namespace

TEST(AuthHandlerDigest, AlgorithmAndQopNotSpecified) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "response=\"bf57e4e0d0bffc0fbaedce64d59add5e\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
            auth_token);
}

TEST(AuthHandlerDigest, AuthAndAlgorithmUnspecified) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "qop=\"auth,auth-int\", "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "response=\"89eb0059246c02b2f6ee02c7961d5ea3\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", "
            "qop=auth, "
            "nc=00000001, "
            "cnonce=\"0a4f113b\"",
            auth_token);
}

TEST(AuthHandlerDigest, AuthAndMd5) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "qop=\"auth,auth-int\", "
      "algorithm=MD5, "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "algorithm=MD5, "
            "response=\"89eb0059246c02b2f6ee02c7961d5ea3\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", "
            "qop=auth, "
            "nc=00000001, "
            "cnonce=\"0a4f113b\"",
            auth_token);
}

TEST(AuthHandlerDigest, AuthAndMd5Sess) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "qop=\"auth,auth-int\", "
      "algorithm=MD5-sess, "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "algorithm=MD5-sess, "
            "response=\"e4e4ea61d186d07a92c9e1f6919902e9\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", "
            "qop=auth, "
            "nc=00000001, "
            "cnonce=\"0a4f113b\"",
            auth_token);
}

TEST(AuthHandlerDigest, AuthIntAndMd5) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "qop=\"auth-int\", "
      "algorithm=MD5, "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "v=0\r\n"
      "o=bob 2890844526 2890844526 IN IP4 media.biloxi.com\r\n"
      "s=-\r\n"
      "c=IN IP4 media.biloxi.com\r\n"
      "t=0 0\r\n"
      "m=audio 49170 RTP/AVP 0\r\n"
      "a=rtpmap:0 PCMU/8000\r\n"
      "m=video 51372 RTP/AVP 31\r\n"
      "a=rtpmap:31 H261/90000\r\n"
      "m=video 53000 RTP/AVP 32\r\n"
      "a=rtpmap:32 MPV/90000\r\n",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "algorithm=MD5, "
            "response=\"bdbeebb2da6adb6bca02599c2239e192\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", "
            "qop=auth-int, "
            "nc=00000001, "
            "cnonce=\"0a4f113b\"",
            auth_token);
}

TEST(AuthHandlerDigest, AuthIntAndMd5Sess) {
  std::string auth_token;
  EXPECT_TRUE(RespondToChallenge(
      "Digest realm=\"biloxi.com\", "
      "qop=\"auth-int\", "
      "algorithm=MD5-sess, "
      "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
      "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"",
      Method::INVITE,
      "sip:bob@biloxi.com",
      "v=0\r\n"
      "o=bob 2890844526 2890844526 IN IP4 media.biloxi.com\r\n"
      "s=-\r\n"
      "c=IN IP4 media.biloxi.com\r\n"
      "t=0 0\r\n"
      "m=audio 49170 RTP/AVP 0\r\n"
      "a=rtpmap:0 PCMU/8000\r\n"
      "m=video 51372 RTP/AVP 31\r\n"
      "a=rtpmap:31 H261/90000\r\n"
      "m=video 53000 RTP/AVP 32\r\n"
      "a=rtpmap:32 MPV/90000\r\n",
      "0a4f113b",
      &auth_token));
  EXPECT_EQ("Authorization: Digest username=\"bob\", "
            "realm=\"biloxi.com\", "
            "nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
            "uri=\"sip:bob@biloxi.com\", "
            "algorithm=MD5-sess, "
            "response=\"91984da2d8663716e91554859c22ca70\", "
            "opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", "
            "qop=auth-int, "
            "nc=00000001, "
            "cnonce=\"0a4f113b\"",
            auth_token);
}

TEST(AuthHandlerDigest, HandleAnotherChallenge) {
  scoped_ptr<AuthHandlerDigest::Factory> factory(
      new AuthHandlerDigest::Factory());
  scoped_ptr<AuthHandler> handler;
  std::string default_challenge =
      "Digest realm=\"biloxi.com\", nonce=\"nonce-value\"";
  scoped_ptr<Header> header =
    Header::Parse(std::string("WWW-Authenticate: " + default_challenge));
  Challenge *challenge = dyn_cast<WwwAuthenticate>(header);
  SipURI uri_origin("sip:bob@biloxi.com");
  int rv = factory->CreateAuthHandler(
    *challenge, net::HttpAuth::AUTH_SERVER,
    GURL(uri_origin.GetOrigin().spec()),
    AuthHandlerFactory::CREATE_CHALLENGE, 1,
    net::BoundNetLog(), &handler);
  EXPECT_EQ(net::OK, rv);
  ASSERT_TRUE(handler.get() != NULL);

  EXPECT_EQ(net::HttpAuth::AUTHORIZATION_RESULT_REJECT,
            handler->HandleAnotherChallenge(*challenge));

  std::string stale_challenge = default_challenge + ", stale=true";
  scoped_ptr<Header> header1 =
    Header::Parse(std::string("WWW-Authenticate: " + stale_challenge));
  challenge = dyn_cast<WwwAuthenticate>(header1);
  EXPECT_EQ(net::HttpAuth::AUTHORIZATION_RESULT_STALE,
            handler->HandleAnotherChallenge(*challenge));

  std::string stale_false_challenge = default_challenge + ", stale=false";
  scoped_ptr<Header> header2 =
    Header::Parse(std::string("WWW-Authenticate: " + stale_false_challenge));
  challenge = dyn_cast<WwwAuthenticate>(header2);
  EXPECT_EQ(net::HttpAuth::AUTHORIZATION_RESULT_REJECT,
            handler->HandleAnotherChallenge(*challenge));

  std::string realm_change_challenge =
      "Digest realm=\"p1.biloxi.com\", nonce=\"nonce-value2\"";
  scoped_ptr<Header> header3 =
    Header::Parse(std::string("WWW-Authenticate: " + realm_change_challenge));
  challenge = dyn_cast<WwwAuthenticate>(header3);
  EXPECT_EQ(net::HttpAuth::AUTHORIZATION_RESULT_DIFFERENT_REALM,
            handler->HandleAnotherChallenge(*challenge));
}

} // namespace sippet
