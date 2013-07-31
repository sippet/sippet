// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_DEFAULT_TIME_DELTA_PROVIDER_H_
#define SIPPET_TRANSPORT_DEFAULT_TIME_DELTA_PROVIDER_H_

#include "sippet/transport/time_delta_factory.h"
#include "base/compiler_specific.h"

namespace sippet {

class DefaultTimeDeltaFactory : public TimeDeltaFactory {
 private:
  DISALLOW_COPY_AND_ASSIGN(DefaultTimeDeltaFactory);
 public:
  DefaultTimeDeltaFactory() {}
  virtual ~DefaultTimeDeltaFactory() {}

  virtual TimeDeltaProvider* CreateClientNonInvite() OVERRIDE;
  virtual TimeDeltaProvider* CreateClientInvite() OVERRIDE;
  virtual TimeDeltaProvider* CreateServerNonInvite() OVERRIDE;
  virtual TimeDeltaProvider* CreateServerInvite() OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_DEFAULT_TIME_DELTA_PROVIDER_H_
