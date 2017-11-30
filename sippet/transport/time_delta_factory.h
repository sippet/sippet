// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TIME_DELTA_FACTORY_H_
#define SIPPET_TRANSPORT_TIME_DELTA_FACTORY_H_

#include "base/macros.h"

namespace sippet {

class TimeDeltaProvider;

class TimeDeltaFactory {
 private:
  DISALLOW_COPY_AND_ASSIGN(TimeDeltaFactory);
 public:
  TimeDeltaFactory() {}
  virtual ~TimeDeltaFactory() {}

  virtual TimeDeltaProvider* CreateClientNonInvite() = 0;
  virtual TimeDeltaProvider* CreateClientInvite() = 0;
  virtual TimeDeltaProvider* CreateServerNonInvite() = 0;
  virtual TimeDeltaProvider* CreateServerInvite() = 0;

  static TimeDeltaFactory *GetDefaultFactory();
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_TIME_DELTA_FACTORY_H_
