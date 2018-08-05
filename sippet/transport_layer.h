// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_LAYER_H_
#define SIPPET_TRANSPORT_LAYER_H_

#include "sippet/sippet_export.h"

namespace sippet {

class SIPPET_EXPORT TransportLayer {
 public:
  virtual ~TransportLayer();

  // Get the stack Core.
  Core* core() { return core_; }

  // Get the transaction layer attached to this transport layer.
  TransactionLayer* transaction_layer() { return transaction_layer_; }

  // Called by the transaction layer while setting up the stack.
  // |transaction_layer| must not be null.
  // |core| must not be null.
  void Init(TransactionLayer* transaction_layer, Core* core) {
    transaction_layer_ = transaction_layer;
    core_ = core;
  }

  // Start receiving messages.
  // It is called by the |TransactionLayer| during the stack setup. The Core
  // and the TransactionLayer are set when it is called.
  virtual void Start() = 0;

  // Stop receiving messages.
  // It should release any system resource it holds.
  virtual void Stop() = 0;

  // Sends a message to the network.
  //
  // If specified, the |id| will be used to notify the transaction layer if a
  // transport error occurs. See |TransactionLayer::ReceiveTransportError|.
  virtual int SendMessage(scoped_refptr<Message> message) = 0;
  virtual int SendMessage(scoped_refptr<Message> message, int id) = 0;

 private:
  // The transaction layer.
  TransactionLayer* transaction_layer_;

  // The stack core.
  Core* core_;
};

}  // namespace sippet

#endif  // SIPPET_TRANSPORT_LAYER_H_
