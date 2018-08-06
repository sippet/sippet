// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_layer.h"
#include "sippet/transaction_config.h"
#include "sippet/transaction_layer_impl.h"
#include "sippet/transaction_layer_factory.h"
#include "sippet/transport_layer.h"

namespace sippet {

TransactionLayer::~TransactionLayer() {}

// static
std::unique_ptr<TransactionLayer> TransactionLayer::Create(
    TransportLayer* transport, Core* core) {
  TransactionConfig config;
  return Create(transport, core, config);
}

std::unique_ptr<TransactionLayer> TransactionLayer::Create(
      TransportLayer* transport, Core* core,
      const TransactionConfig& config) {
  TransactionLayerFactory* factory = TransactionLayerImpl::factory();
  return factory ? factory->CreateTransactionLayer(transport, core, config)
                 : std::unique_ptr<TransactionLayer>(new TransactionLayerImpl(
                       transport, core, config));
}

}  // namespace