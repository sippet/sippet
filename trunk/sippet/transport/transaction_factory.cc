// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/transaction_factory.h"
#include "sippet/transport/client_transaction_impl.h"
#include "sippet/transport/server_transaction_impl.h"

#include "base/lazy_instance.h"

namespace sippet {

namespace {

class DefaultTransactionFactory : public TransactionFactory {
 public:
  DefaultTransactionFactory() {}
  ~DefaultTransactionFactory() override {}

  ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) override {
    return new ClientTransactionImpl(transaction_id, channel,
        delegate, time_delta_factory);
  }

  ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) override {
    return new ServerTransactionImpl(transaction_id, channel,
        delegate, time_delta_factory);
  }
};

static base::LazyInstance<DefaultTransactionFactory>::Leaky
  g_default_transaction_factory = LAZY_INSTANCE_INITIALIZER;

} // End of empty namespace

TransactionFactory *TransactionFactory::GetDefaultTransactionFactory() {
  return g_default_transaction_factory.Pointer();
}

} // End of sippet namespace
