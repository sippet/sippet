// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_CONFIG_H_
#define SIPPET_TRANSACTION_CONFIG_H_

#include <vector>

#include "base/time/time.h"
#include "sippet/sippet_export.h"

namespace sippet {

struct SIPPET_EXPORT TransactionConfig {
  TransactionConfig();
  TransactionConfig(const TransactionConfig& other);
  ~TransactionConfig();

  // The RTT Estimate.
  base::TimeDelta t1;

  // The maximum retransmit interval for non-INVITE requests and INVITE
  // responses.
  base::TimeDelta t2;

  // Maximum duration a message will remain in the network.
  base::TimeDelta t4;
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_CONFIG_H_
