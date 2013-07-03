// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

TEST(Url, Parser) {
  struct pair {
    const char *key;
    const char *value;
  };
  struct {
    const char *input;
    bool valid;
    const char *host;
    int port;
    int effective_port;
    bool has_username;
    const char *username;
    bool has_password;
    const char *password;
    pair parameters[10];
    pair headers[10];
  } tests[] = {
    {"sip:sip.domain.com", true,
     "sip.domain.com", -1, 5060, false, "", false, "", {{0}}, {{0}} },
    {"sip:user@sip.domain.com", true,
     "sip.domain.com", -1, 5060, true, "user", false, "", {{0}}, {{0}} },
    {"sip:user@sip.domain.com;param=1234", true,
     "sip.domain.com", -1, 5060, true, "user", false, "",
     {{"param","1234"},{0}}, {{0}} },
    {"sip:1234@sip.domain.com:5060;TCID-0", true,
     "sip.domain.com", -1, 5060, true, "1234", false, "",
     {{"TCID-0",""},{0}}, {{0}} },
    {"sip:user@sip.domain.com?header=1234", true, "sip.domain.com", -1, 5060,
     true, "user", false, "", {{0}}, {{"header","1234"},{0}} },
    {"sip:[5f1b:df00:ce3e:e200:20:800:121.12.131.12]", true,
     "[5f1b:df00:ce3e:e200:20:800:790c:830c]", -1, 5060,
     false, "", false, "", {{0}}, {{0}} },
    {"sip:user@[5f1b:df00:ce3e:e200:20:800:121.12.131.12]", true,
     "[5f1b:df00:ce3e:e200:20:800:790c:830c]", -1, 5060,
     true, "user", false, "", {{0}}, {{0}} },
    {"sips:192.168.2.12", true,
     "192.168.2.12", -1, 5061, false, "", false, "", {{0}}, {{0}} },
    {"sips:host.foo.com", true,
     "host.foo.com", -1, 5061, false, "", false, "", {{0}}, {{0}} },
    //{"sip:user;x-v17:password@host.com:5555", true,
    // "host.com", 5555, 5555, true, "user;x-v17", true, "password",
    // {{0}}, {{0}} },
    {"sip:wombat@192.168.2.221:5062;transport=Udp", true,
     "192.168.2.221", 5062, 5062, true, "wombat", false, "",
     {{"transport","udp"},{0}}, {{0}} },
    //{"sip:+358-555-1234567;isub=1411;postd=pp2@company.com;user=phone", true,
    // "company.com", -1, 5060,
    // true, "+358-555-1234567;isub=1411;postd=pp2", false, "",
    // {{"user","phone"},{0}}, {{0}} },
    {"sip:biloxi.com;transport=tcp;method=REGISTER?to=sip:bob%40biloxi.com",
     true, "biloxi.com", -1, 5060, false, "", false, "",
     {{"transport","tcp"},{"method","register"},{0}},
     {{"To","sip:bob@biloxi.com"},{0}} },
    {"sip:alice@atlanta.com?subject=Project%20X&priority=urgent", true,
     "atlanta.com", -1, 5060, true, "alice", false, "",
     {{0}}, {{"Subject","Project X"}, {"Priority","uergent"}, {0}} },
    //{"sip:Blue Face", false},
    {"*", false},
  };

  for (int i = 0; i < arraysize(tests); ++i) {
    URI url(tests[i].input);
    EXPECT_EQ(tests[i].valid, url.is_valid());
    if (tests[i].valid) {
      EXPECT_EQ(tests[i].host, url.host());
      EXPECT_EQ(tests[i].port, url.IntPort());
      EXPECT_EQ(tests[i].effective_port, url.EffectiveIntPort());
      EXPECT_EQ(tests[i].has_username, url.has_username());
      if (tests[i].has_username)
        EXPECT_EQ(tests[i].username, url.username());
      EXPECT_EQ(tests[i].has_password, url.has_password());
      if (tests[i].has_password)
        EXPECT_EQ(tests[i].password, url.password());
      for (pair *x = tests[i].parameters; x->key != 0; ++x) {
        // TODO
      }
      for (pair *y = tests[i].headers; y->key != 0; ++y) {
        // TODO
      }
    }
  }
}

//    {"tel:+358-555-1234567;pOstd=pP2;isUb=1411", true
//    {"tel:+358-555-1234567;pOstd=pP2;isUb=1411", true
