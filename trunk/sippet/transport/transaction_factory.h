// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TRANSACTION_FACTORY_H_
#define SIPPET_TRANSPORT_TRANSACTION_FACTORY_H_

#include <string>
#include "base/memory/scoped_ptr.h"
#include "sippet/message/method.h"
#include "sippet/transport/channel.h"

namespace sippet {

class ClientTransaction;
class ServerTransaction;
class TransactionDelegate;
class TimeDeltaFactory;

class TransactionFactory {
 public:
  virtual ~TransactionFactory() {}

  virtual ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) = 0;

  virtual ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) = 0;

  // Returns the default TransactionFactory.
  static TransactionFactory *GetDefaultTransactionFactory();
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_TRANSACTION_FACTORY_H_
