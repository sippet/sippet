// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/end_point.h"

#include "testing/gtest/include/gtest/gtest.h"

using sippet::Protocol;
using sippet::EndPoint;

TEST(EndPoint, FromString) {
  struct {
    const char *input;
    bool valid;
    const char *host;
    int port;
    Protocol::Type protocol;
  } cases[] = {
    { "192.168.0.1:5060/TCP", true, "192.168.0.1", 5060, Protocol::TCP },
    { "127.0.0.1:4050/UDP", true, "127.0.0.1", 4050, Protocol::UDP },
    { "0.0.0.0:5554/UDP", true, "0.0.0.0", 5554, Protocol::UDP },
    { "[::1]:123/UDP", true, "::1", 123, Protocol::UDP },
    { "host.name:1024/TLS", true, "host.name", 1024, Protocol::TLS },
    { ":123/UDP", false },
    { "[::1]:/UDP", false },
    { "[::1]:123/", false },
  };

  for (int i = 0; i < arraysize(cases); ++i) {
    EndPoint parsed = EndPoint::FromString(cases[i].input);
    if (!cases[i].valid) {
      EXPECT_TRUE(parsed.IsEmpty());
    } else {
      EXPECT_EQ(cases[i].host, parsed.host());
      EXPECT_EQ(cases[i].port, parsed.port());
      EXPECT_EQ(cases[i].protocol, parsed.protocol());
    }
  }
}

TEST(EndPoint, ToString) {
  struct {
    const char *host;
    int port;
    Protocol::Type protocol;
    const char *output;
  } cases[] = {
    { "192.168.0.1", 5060, Protocol::TCP, "192.168.0.1:5060/TCP" },
    { "::1", 123, Protocol::UDP, "[::1]:123/UDP" },
    { "host.name", 1024, Protocol::TLS, "host.name:1024/TLS" },
  };

  for (int i = 0; i < arraysize(cases); ++i) {
    EndPoint endpoint(cases[i].host, cases[i].port, cases[i].protocol);
    std::string str = endpoint.ToString();
    EXPECT_EQ(cases[i].output, str);
  }
}
