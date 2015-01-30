// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/test/standalone_test_server/standalone_test_server.h"

#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

class StandaloneTestServerTest: public testing::Test {
 public:
  StandaloneTestServerTest() {
  }

  virtual void TearDown() OVERRIDE {
    if (server_)
      ASSERT_TRUE(server_->ShutdownAndWaitUntilComplete());
  }

  void Init(const Protocol &protocol) {
    server_.reset(new StandaloneTestServer(protocol));
    ASSERT_TRUE(server_->InitializeAndWaitUntilReady());
  }

  void Init(const Protocol &protocol,
      const StandaloneTestServer::SSLOptions &ssl_options) {
    server_.reset(new StandaloneTestServer(protocol, ssl_options));
    ASSERT_TRUE(server_->InitializeAndWaitUntilReady());
  }

 protected:
  scoped_ptr<StandaloneTestServer> server_;
};

TEST_F(StandaloneTestServerTest, GetUDPBaseURL) {
  Init(Protocol::UDP);
  EXPECT_EQ(
    base::StringPrintf("sip:127.0.0.1:%d", server_->port()),
    server_->base_uri().spec());
}

TEST_F(StandaloneTestServerTest, GetTCPBaseURL) {
  Init(Protocol::TCP);
  EXPECT_EQ(
    base::StringPrintf("sip:127.0.0.1:%d;transport=TCP", server_->port()),
    server_->base_uri().spec());
}

TEST_F(StandaloneTestServerTest, GetTLSBaseURL) {
  base::FilePath certs_dir;
  PathService::Get(base::DIR_SOURCE_ROOT, &certs_dir);
  certs_dir.Append(FILE_PATH_LITERAL("net/data/ssl/certificates"));

  StandaloneTestServer::SSLOptions ssl_options;
  ssl_options.certificate_file = certs_dir.AppendASCII("unittest.selfsigned.der");
  ssl_options.privatekey_file = certs_dir.AppendASCII("unittest.key.bin");

  Init(Protocol::TLS, ssl_options);
  EXPECT_EQ(
    base::StringPrintf("sips:127.0.0.1:%d", server_->port()),
    server_->base_uri().spec());
}

} // namespace sippet
