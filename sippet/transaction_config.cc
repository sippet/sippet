// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_config.h"

namespace sippet {

TransactionConfig::TransactionConfig()
    : t1(base::TimeDelta::FromMilliseconds(500)),
      t2(base::TimeDelta::FromSeconds(4)),
      t4(base::TimeDelta::FromSeconds(5)) {}

TransactionConfig::TransactionConfig(const TransactionConfig& other) = default;

TransactionConfig::~TransactionConfig() {}

}  // namespace sippet