// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_LAYER_H_
#define SIPPET_TRANSPORT_LAYER_H_

#include "base/memory/ref_counted.h"
#include "sippet/sippet_export.h"

namespace sippet {
class Core;
class Message;
class TransactionLayer;

class SIPPET_EXPORT TransportLayer {
 public:
  virtual ~TransportLayer();

  // Create a new transport layer.
  static std::unique_ptr<TransportLayer> Create();

  // Called by the transaction layer while setting up the stack.
  // |transaction_layer| must not be null.
  // |core| must not be null.
  virtual void Init(TransactionLayer* transaction_layer, Core* core) = 0;

  // Start receiving messages.
  // It is called by the |TransactionLayer| during the stack setup. The Core
  // and the TransactionLayer are set when it is called.
  virtual void Start() = 0;

  // Stop receiving messages.
  // It should release any system resource it holds.
  virtual void Stop() = 0;

  // Sends a message to the network.
  virtual void SendMessage(scoped_refptr<Message> message) = 0;
};

}  // namespace sippet

#endif  // SIPPET_TRANSPORT_LAYER_H_
