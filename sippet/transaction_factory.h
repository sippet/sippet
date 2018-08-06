// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_FACTORY_H_
#define SIPPET_TRANSACTION_FACTORY_H_

#include <memory>

#include "base/memory/ref_counted.h"

namespace base {
class SequencedTaskRunner;
}  // namespace base

namespace sippet {
class Core;
class TransportLayer;
class ClientTransaction;
class ServerTransaction;
class TransactionConfig;

// TransactionLayerCore::CreateClientTransaction and
// TransactionLayerCore::CreateServerTransaction uses the currently registered
// Factory to create ClientTransaction and ServerTransaction. Factory is
// intended for testing.
class TransactionFactory {
 public:
  virtual std::unique_ptr<ClientTransaction> CreateClientTransaction(
      TransportLayer* transport,
      const TransactionConfig& config,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner,
      Core* core) = 0;

  virtual std::unique_ptr<ServerTransaction> CreateServerTransaction(
      TransportLayer* transport,
      const TransactionConfig& config,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner,
      Core* core) = 0;

 protected:
  virtual ~TransactionFactory() {}
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_FACTORY_H_
