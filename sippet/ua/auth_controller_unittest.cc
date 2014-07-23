// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_mock.h"
#include "sippet/ua/auth_controller.h"
#include "sippet/ua/auth_cache.h"
#include "base/message_loop/message_loop.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::NiceMock;
using ::testing::Return;
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
      Request::Parse("REGISTER sip:some.domain.com SIP/2.0\r\n"
                     "\r\n")));
    request->set_direction(Message::Incoming);
    scoped_refptr<Response> response(dyn_cast<Response>(
      Response::Parse(test->response_)));
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

TEST_F(AuthControllerTest, NoExplicitCredentialsAllowed) {
  // TODO
}

} // namespace sippet
