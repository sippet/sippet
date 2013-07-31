// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/default_time_delta_factory.h"
#include "sippet/transport/time_delta_provider.h"

namespace sippet {

namespace {

class ClientNonInvite : public TimeDeltaProvider {
 public:
  ClientNonInvite() : count_(0) {}
  virtual ~ClientNonInvite() {}
  virtual base::TimeDelta GetNextRetryDelay() OVERRIDE {
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
  virtual base::TimeDelta GetTimeoutDelay() OVERRIDE {
    // This is 64*T1, where T1 = 500ms
    return base::TimeDelta::FromSeconds(32);
  }
  virtual base::TimeDelta GetTerminateDelay() OVERRIDE {
    // Timer K equal to 5s
    return base::TimeDelta::FromSeconds(5);
  }
 private:
  int count_;
};

class ClientInvite : public TimeDeltaProvider {
 public:
  ClientInvite() : multiply_(1) {}
  virtual ~ClientInvite() {}
  virtual base::TimeDelta GetNextRetryDelay() OVERRIDE {
    // Implement the exponential backoff *2 at each retransmission
    int64 seconds = multiply_ * 500;
    multiply_ <<= 1;
    return base::TimeDelta::FromMilliseconds(seconds);
  }
  virtual base::TimeDelta GetTimeoutDelay() OVERRIDE {
    // This is 64*T1, where T1 = 500ms
    return base::TimeDelta::FromSeconds(32);
  }
  virtual base::TimeDelta GetTerminateDelay() OVERRIDE {
    // Timer D is greater than 32s (35 is greater than 32s)
    return base::TimeDelta::FromSeconds(35);
  }
 private:
  int multiply_;
};

} // End of empty namespace

TimeDeltaProvider* DefaultTimeDeltaFactory::CreateClientNonInvite() {
  return new ClientNonInvite;
}

TimeDeltaProvider* DefaultTimeDeltaFactory::CreateClientInvite() {
  return new ClientInvite;
}

TimeDeltaProvider* DefaultTimeDeltaFactory::CreateServerNonInvite() {
  return 0;
}

TimeDeltaProvider* DefaultTimeDeltaFactory::CreateServerInvite() {
  return 0;
}

} // End of sippet namespace
