// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_LAYER_IMPL_H_
#define SIPPET_TRANSACTION_LAYER_IMPL_H_

#include "sippet/sippet_export.h"
#include "sippet/transaction_layer.h"

namespace sippet {
class TransactionLayerCore;
class TransactionLayerFactory;
struct TransactionConfig;

class SIPPET_EXPORT_PRIVATE TransactionLayerImpl : public TransactionLayer {
 public:
  TransactionLayerImpl(TransportLayer* transport, Core* core,
      const TransactionConfig& config);
  ~TransactionLayerImpl() override;

  // TransactionLayer implementation:
  void SetRequestContext(
      net::URLRequestContextGetter* request_context_getter) override;
  void Start() override;
  void SendRequest(scoped_refptr<Request> request) override;
  void SendResponse(scoped_refptr<Response> response) override;
  void Terminate(const std::string& id) override;
  void OnMessage(scoped_refptr<Message> message) override;
  void OnTransportError(const std::string& id, int error) override;

  // Sets the factory used by the static method Create to create a
  // TransactionLayer. TransactionLayer does not take ownership of |factory|.
  // A value of NULL results in a TransactionLayerImpl being created directly.
  static void set_factory(TransactionLayerFactory* factory);
  static TransactionLayerFactory* factory();

 private:
  const scoped_refptr<TransactionLayerCore> core_;

  DISALLOW_COPY_AND_ASSIGN(TransactionLayerImpl);
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_LAYER_IMPL_H_
