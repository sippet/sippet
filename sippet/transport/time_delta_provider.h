// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TIME_DELTA_PROVIDER_H_
#define SIPPET_TRANSPORT_TIME_DELTA_PROVIDER_H_

#include "base/basictypes.h"
#include "base/time.h"

namespace sippet {

class TimeDeltaProvider {
 private:
  DISALLOW_COPY_AND_ASSIGN(TimeDeltaProvider);
 public:
  TimeDeltaProvider() {}
  virtual ~TimeDeltaProvider() {}

  virtual base::TimeDelta GetNextRetryDelay() = 0;
  virtual base::TimeDelta GetTimeoutDelay() = 0;
  virtual base::TimeDelta GetTerminateDelay() = 0;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_TIME_DELTA_PROVIDER_H_
