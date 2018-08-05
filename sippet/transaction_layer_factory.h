// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_LAYER_FACTORY_H_
#define SIPPET_TRANSACTION_LAYER_FACTORY_H_

#include "sippet/transaction_layer.h"

namespace sippet {
class Core;
class TransportLayer;

// TransactionLayer::Create uses the currently registered Factory to create the
// TransactionLayer. Factory is intended for testing.
class TransactionLayerFactory {
 public:
  virtual std::unique_ptr<TransactionLayer> CreateTransactionLayer(
      TransportLayer* transport, Core* core) = 0;

 protected:
  virtual ~TransactionLayerFactory() {}
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_LAYER_FACTORY_H_
