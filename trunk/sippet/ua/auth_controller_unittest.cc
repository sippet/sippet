// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_mock.h"
#include "sippet/ua/auth_controller.h"
#include "sippet/ua/auth_cache.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::_;

namespace sippet {

class AuthControllerTest : public testing::Test {
 public:
  virtual void SetUp() {
    controller_ = new AuthController(&dummy_auth_cache_,
      &auth_handler_mock_factory_);
  }
 
  virtual void TearDown() {
  }

  AuthHandlerMock::Factory &factory() {
    return auth_handler_mock_factory_;
  }

  AuthController &controller() {
    return *controller_;
  }

  enum HandlerRunMode {
    RUN_HANDLER_SYNC,
    RUN_HANDLER_ASYNC
  };

  enum SchemeState {
    SCHEME_IS_DISABLED,
    SCHEME_IS_ENABLED
  };

  struct ResponseTest {
    Auth::Target target_;
    HandlerRunMode run_mode_;
    SchemeState scheme_state_;
    const char *response_;
    int handler_rv_;
    int expected_challenge_rv_;
    int expected_controller_rv_;
  };

  void RunResponseTests(ResponseTest *tests, int size) {
    for (int i = 0; i < size; i++) {
      ResponseTest *test = &tests[i];
      RunResponseTest(test);
    }
  }

  void RunResponseTest(ResponseTest *test) {
    net::BoundNetLog dummy_log;

    current_test_ = test;
    scoped_refptr<Request> request(dyn_cast<Request>(
      Message::Parse("REGISTER sip:some.domain.com SIP/2.0\r\n"
                     "\r\n")));
    request->set_direction(Message::Incoming);
    scoped_refptr<Response> response(dyn_cast<Response>(
      Message::Parse(test->response_)));
    response->set_refer_to(request);

    NiceMock<AuthHandlerMock>* auth_handler = new NiceMock<AuthHandlerMock>;
    ON_CALL(*auth_handler, HandleAnotherChallenge(_))
      .WillByDefault(Return(net::HttpAuth::AUTHORIZATION_RESULT_ACCEPT));
    ON_CALL(*auth_handler, GenerateAuthImpl(_, _, _))
      .WillByDefault(Invoke(this, &AuthControllerTest::GenerateAuthImpl));

    factory().AddMockHandler(auth_handler, test->target_, true);

    ASSERT_EQ(test->expected_challenge_rv_,
              controller().HandleAuthChallenge(response, dummy_log));
    if (net::OK == test->expected_challenge_rv_) {
      ASSERT_EQ(test->target_, controller().target());
      ASSERT_TRUE(controller().HaveAuthHandler());
      controller().ResetAuth(net::AuthCredentials());
      EXPECT_TRUE(controller().HaveAuth());

      net::TestCompletionCallback callback;
      EXPECT_EQ((test->run_mode_ == RUN_HANDLER_ASYNC)? net::ERR_IO_PENDING:
                test->expected_controller_rv_,
                controller().AddAuthorizationHeaders(request,
                    callback.callback(), dummy_log));
      if (test->run_mode_ == RUN_HANDLER_ASYNC)
        EXPECT_EQ(test->expected_controller_rv_, callback.WaitForResult());
      EXPECT_EQ((test->scheme_state_ == SCHEME_IS_DISABLED),
                controller().IsAuthSchemeDisabled(net::HttpAuth::AUTH_SCHEME_MOCK));
    }
  }

 private:
  int GenerateAuthImpl(
      const net::AuthCredentials* credentials,
      const scoped_refptr<Request> &request,
      const net::CompletionCallback& callback) {
    if (current_test_->run_mode_ == RUN_HANDLER_SYNC)
      return current_test_->handler_rv_;
    callback_ = callback;
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&AuthControllerTest::OnIOComplete,
            base::Unretained(this), current_test_->handler_rv_));
    return net::ERR_IO_PENDING;
  }

  void OnIOComplete(int rv) {
    net::CompletionCallback c = callback_;
    callback_.Reset();
    c.Run(rv);
  }

  ResponseTest *current_test_;
  AuthCache dummy_auth_cache_;
  net::CompletionCallback callback_;
  scoped_refptr<AuthController> controller_;
  AuthHandlerMock::Factory auth_handler_mock_factory_;
};

TEST_F(AuthControllerTest, SuccessServerAuth) {
  ResponseTest responses[] = {
    { net::HttpAuth::AUTH_PROXY, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 407\r\n"
      "Proxy-Authenticate: MOCK foo\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
  };

  RunResponseTests(responses, arraysize(responses));
}

TEST_F(AuthControllerTest, SuccessProxyAndServerAuth) {
  ResponseTest responses[] = {
    { net::HttpAuth::AUTH_PROXY, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 407\r\n"
      "Proxy-Authenticate: MOCK foo\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
    { net::HttpAuth::AUTH_SERVER, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 401\r\n"
      "WWW-Authenticate: MOCK bar\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
  };

  RunResponseTests(responses, arraysize(responses));
}

TEST_F(AuthControllerTest, SuccessiveProxiesThenServerAuth) {
  ResponseTest responses[] = {
    { net::HttpAuth::AUTH_PROXY, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 407\r\n"
      "Proxy-Authenticate: MOCK foo\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
    { net::HttpAuth::AUTH_PROXY, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 407\r\n"
      "Proxy-Authenticate: MOCK bar\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
    { net::HttpAuth::AUTH_SERVER, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 401\r\n"
      "WWW-Authenticate: MOCK baz\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
  };

  RunResponseTests(responses, arraysize(responses));
}

TEST_F(AuthControllerTest, ServerThenProxyNotAllowed) {
  ResponseTest responses[] = {
    { net::HttpAuth::AUTH_SERVER, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 401\r\n"
      "WWW-Authenticate: MOCK bar\r\n"
      "\r\n",
      net::OK,
      net::OK,
      net::OK },
    { net::HttpAuth::AUTH_PROXY, RUN_HANDLER_SYNC, SCHEME_IS_ENABLED,
      "SIP/2.0 407\r\n"
      "Proxy-Authenticate: MOCK foo\r\n"
      "\r\n",
      net::OK,
      net::ERR_UNEXPECTED_PROXY_AUTH },
  };

  RunResponseTests(responses, arraysize(responses));
}

TEST_F(AuthControllerTest, UnexpectedSecurityLibraryStatus) {
  // Run a synchronous handler that returns
  // ERR_UNEXPECTED_SECURITY_LIBRARY_STATUS.  We expect a return value
  // of OK from the controller so we can retry the request.
  ResponseTest case1 = {
    net::HttpAuth::AUTH_SERVER, RUN_HANDLER_SYNC, SCHEME_IS_DISABLED,
    "SIP/2.0 401\r\n"
    "WWW-Authenticate: MOCK bar\r\n"
    "\r\n",
    net::ERR_UNEXPECTED_SECURITY_LIBRARY_STATUS,
    net::OK,
    net::OK };

  RunResponseTest(&case1);
}

TEST_F(AuthControllerTest, MissingAuthCredentials) {
  // Now try an async handler that returns
  // ERR_MISSING_AUTH_CREDENTIALS.  Async and sync handlers invoke
  // different code paths in AuthController when generating
  // tokens.
  ResponseTest case2 = {
    net::HttpAuth::AUTH_SERVER, RUN_HANDLER_ASYNC, SCHEME_IS_DISABLED,
    "SIP/2.0 401\r\n"
    "WWW-Authenticate: MOCK bar\r\n"
    "\r\n",
    net::ERR_MISSING_AUTH_CREDENTIALS,
    net::OK,
    net::OK };

  RunResponseTest(&case2);
}

TEST_F(AuthControllerTest, InvalidAuthCredentials) {
  // If a non-permanent error is returned by the handler, then the
  // controller should report it unchanged.
  ResponseTest case3 = {
    net::HttpAuth::AUTH_SERVER, RUN_HANDLER_ASYNC, SCHEME_IS_ENABLED,
    "SIP/2.0 401\r\n"
    "WWW-Authenticate: MOCK bar\r\n"
    "\r\n",
    net::ERR_INVALID_AUTH_CREDENTIALS,
    net::OK,
    net::ERR_INVALID_AUTH_CREDENTIALS };

  RunResponseTest(&case3);
}

// If an AuthHandler indicates that it doesn't allow explicit
// credentials, don't prompt for credentials.
TEST_F(AuthControllerTest, NoExplicitCredentialsAllowed) {
  // Modified mock HttpAuthHandler for this test.
  class MockHandler : public AuthHandlerMock {
   public:
    MockHandler(int expected_rv, Auth::Scheme scheme)
        : expected_scheme_(scheme) {
      ON_CALL(*this, GenerateAuthImpl(_, _, _))
        .WillByDefault(Return(expected_rv));
      ON_CALL(*this, HandleAnotherChallenge(_))
        .WillByDefault(Invoke(this, &MockHandler::HandleAnotherChallenge));
    }

   protected:
    virtual bool Init(const Challenge& challenge) OVERRIDE {
      AuthHandlerMock::Init(challenge);
      set_allows_default_credentials(true);
      set_allows_explicit_credentials(false);
      // Pretend to be SCHEME_DIGEST so we can test failover logic.
      if (challenge.scheme() == "Digest") {
        auth_scheme_ = net::HttpAuth::AUTH_SCHEME_DIGEST;
        --score_;  // Reduce score, so we rank below Mock.
        set_allows_explicit_credentials(true);
      }
      EXPECT_EQ(expected_scheme_, auth_scheme_);
      return true;
    }

    Auth::AuthorizationResult HandleAnotherChallenge(
        const Challenge& challenge) {
      // If we receive an empty challenge for a connection based scheme, or a second
      // challenge for a non connection based scheme, assume it's a rejection.
      if (challenge.param_empty())
        return net::HttpAuth::AUTHORIZATION_RESULT_REJECT;
      if (!LowerCaseEqualsASCII(challenge.scheme(), "mock"))
        return net::HttpAuth::AUTHORIZATION_RESULT_INVALID;
      return net::HttpAuth::AUTHORIZATION_RESULT_ACCEPT;
    }

   private:
    Auth::Scheme expected_scheme_;
  };

  net::BoundNetLog dummy_log;
  AuthCache dummy_auth_cache;
  scoped_refptr<Request> request(dyn_cast<Request>(
    Message::Parse("REGISTER sip:some.domain.com SIP/2.0\r\n"
                   "\r\n")));
  request->set_direction(Message::Incoming);
  scoped_refptr<Response> response(dyn_cast<Response>(
      Message::Parse("SIP/2.0 401\r\n"
                     "WWW-Authenticate: Mock\r\n"
                     "WWW-Authenticate: Digest\r\n"
                     "\r\n")));
  response->set_refer_to(request);

  // Handlers for the first attempt at authentication.  AUTH_SCHEME_MOCK handler
  // accepts the default identity and successfully constructs a token.
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::OK, net::HttpAuth::AUTH_SCHEME_MOCK),
      net::HttpAuth::AUTH_SERVER, true);
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::ERR_UNEXPECTED, net::HttpAuth::AUTH_SCHEME_DIGEST),
      net::HttpAuth::AUTH_SERVER, true);

  // Handlers for the second attempt.  Neither should be used to generate a
  // token.  Instead the controller should realize that there are no viable
  // identities to use with the AUTH_SCHEME_MOCK handler and fail.
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::ERR_UNEXPECTED, net::HttpAuth::AUTH_SCHEME_MOCK),
      net::HttpAuth::AUTH_SERVER, true);
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::ERR_UNEXPECTED, net::HttpAuth::AUTH_SCHEME_DIGEST),
      net::HttpAuth::AUTH_SERVER, true);

  // Fallback handlers for the second attempt.  The AUTH_SCHEME_MOCK handler
  // should be discarded due to the disabled scheme, and the AUTH_SCHEME_BASIC
  // handler should successfully be used to generate a token.
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::ERR_UNEXPECTED, net::HttpAuth::AUTH_SCHEME_MOCK),
      net::HttpAuth::AUTH_SERVER, true);
  factory().AddMockHandler(
      new NiceMock<MockHandler>(net::OK, net::HttpAuth::AUTH_SCHEME_DIGEST),
      net::HttpAuth::AUTH_SERVER, true);

  ASSERT_EQ(net::OK,
            controller().HandleAuthChallenge(response, dummy_log));
  ASSERT_TRUE(controller().HaveAuthHandler());
  controller().ResetAuth(net::AuthCredentials());
  EXPECT_TRUE(controller().HaveAuth());

  // Should only succeed if we are using the AUTH_SCHEME_MOCK MockHandler.
  EXPECT_EQ(net::OK, controller().AddAuthorizationHeaders(
      request, net::CompletionCallback(), dummy_log));

  // Once a token is generated, simulate the receipt of a server response
  // indicating that the authentication attempt was rejected.
  ASSERT_EQ(net::OK,
            controller().HandleAuthChallenge(response, dummy_log));
  ASSERT_TRUE(controller().HaveAuthHandler());
  controller().ResetAuth(net::AuthCredentials(ASCIIToUTF16("Hello"),
                         base::string16()));
  EXPECT_TRUE(controller().HaveAuth());
  EXPECT_TRUE(controller().IsAuthSchemeDisabled(
    net::HttpAuth::AUTH_SCHEME_MOCK));
  EXPECT_FALSE(controller().IsAuthSchemeDisabled(
    net::HttpAuth::AUTH_SCHEME_DIGEST));

  // Should only succeed if we are using the AUTH_SCHEME_BASIC MockHandler.
  EXPECT_EQ(net::OK, controller().AddAuthorizationHeaders(
      request, net::CompletionCallback(), dummy_log));
}

} // namespace sippet
