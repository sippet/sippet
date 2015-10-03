// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/time_delta_factory.h"
#include "sippet/transport/time_delta_provider.h"

#include "base/lazy_instance.h"
#include "base/compiler_specific.h"

namespace sippet {

namespace {

class ClientNonInvite : public TimeDeltaProvider {
 public:
  ClientNonInvite() : count_(0) {}
  ~ClientNonInvite() override {}
  base::TimeDelta GetNextRetryDelay() override {
    // Implement the exponential backoff up to 4 seconds
    int64 seconds;
    switch (count_++) {
      case 0: seconds = 500; break;
      case 1: seconds = 1000; break;
      case 2: seconds = 2000; break;
      default: seconds = 4000; break;
    }
    return base::TimeDelta::FromMilliseconds(seconds);
  }
  base::TimeDelta GetTimeoutDelay() override {
    // This is 64*T1, where T1 = 500ms
    return base::TimeDelta::FromSeconds(32);
  }
  base::TimeDelta GetTerminateDelay() override {
    // Timer K equal to 5s
    return base::TimeDelta::FromSeconds(5);
  }

 private:
  int count_;
};

class ClientInvite : public TimeDeltaProvider {
 public:
  ClientInvite() : multiply_(1) {}
  ~ClientInvite() override {}
  base::TimeDelta GetNextRetryDelay() override {
    // Implement the exponential backoff *2 at each retransmission
    int64 seconds = multiply_ * 500;
    multiply_ <<= 1;
    return base::TimeDelta::FromMilliseconds(seconds);
  }
  base::TimeDelta GetTimeoutDelay() override {
    // This is 64*T1, where T1 = 500ms
    return base::TimeDelta::FromSeconds(32);
  }
  base::TimeDelta GetTerminateDelay() override {
    // Timer D is greater than 32s (35 is greater than 32s)
    return base::TimeDelta::FromSeconds(35);
  }
 private:
  int multiply_;
};

class ServerNonInvite : public TimeDeltaProvider {
 public:
  ServerNonInvite() {}
  ~ServerNonInvite() override {}
  base::TimeDelta GetNextRetryDelay() override {
    // There's no retry on server non-INVITE transactions
    return base::TimeDelta();
  }
  base::TimeDelta GetTimeoutDelay() override {
    // There's no timeout on server non-INVITE transactions
    return base::TimeDelta();
  }
  base::TimeDelta GetTerminateDelay() override {
    // Timer J equal to 5s
    return base::TimeDelta::FromSeconds(32);
  }
};

class ServerInvite : public TimeDeltaProvider {
 public:
  ServerInvite() : count_(0) {}
  ~ServerInvite() override {}
  base::TimeDelta GetNextRetryDelay() override {
    // Timer G: implement the exponential backoff up to 4 seconds
    int64 seconds;
    switch (count_++) {
      case 0: seconds = 500; break;
      case 1: seconds = 1000; break;
      case 2: seconds = 2000; break;
      default: seconds = 4000; break;
    }
    return base::TimeDelta::FromMilliseconds(seconds);
  }
  base::TimeDelta GetTimeoutDelay() override {
    // Timer H is 64*T1, where T1 = 500ms
    return base::TimeDelta::FromSeconds(32);
  }
  base::TimeDelta GetTerminateDelay() override {
    // Timer I equal to 5s
    return base::TimeDelta::FromSeconds(5);
  }

 private:
  int count_;
};

class DefaultTimeDeltaFactory : public TimeDeltaFactory {
 public:
  DefaultTimeDeltaFactory() {}
  ~DefaultTimeDeltaFactory() override {}

  TimeDeltaProvider* CreateClientNonInvite() override {
    return new ClientNonInvite;
  }

  TimeDeltaProvider* CreateClientInvite() override {
    return new ClientInvite;
  }

  TimeDeltaProvider* CreateServerNonInvite() override {
    return new ServerNonInvite;
  }

  TimeDeltaProvider* CreateServerInvite() override {
    return new ServerInvite;
  }
};

static base::LazyInstance<DefaultTimeDeltaFactory>::Leaky
  g_default_time_delta_factory = LAZY_INSTANCE_INITIALIZER;

}  // namespace

TimeDeltaFactory *TimeDeltaFactory::GetDefaultFactory() {
  return g_default_time_delta_factory.Pointer();
}

}  // namespace sippet
