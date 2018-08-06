// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_CLIENT_TRANSACTION_H_
#define SIPPET_CLIENT_TRANSACTION_H_

#include "base/timer/timer.h"
#include "base/memory/ref_counted.h"
#include "sippet/transaction_config.h"

namespace sippet {
class Request;
class Response;
class TransportLayer;

class ClientTransaction {
 public:
  ClientTransaction(scoped_refptr<Request> request,
      const TransactionConfig& config, TransportLayer* transport_layer);
  ~ClientTransaction();

  // Start the client transaction. The request is sent to the transport layer
  // at this moment.
  void Start();

  // Receive the response sending to the core if necessary. It will not send
  // retransmissions to core.
  void ReceiveResponse(scoped_refptr<Response> response);

  // Terminate this transaction immediately.
  void Terminate();

 private:
  scoped_refptr<Request> request_;
  const TransactionConfig config_;
  TransportLayer* transport_layer_;
  int retransmissions_;

  base::OneShotTimer retry_timer_;
  base::OneShotTimer timeout_timer_;
  base::OneShotTimer terminate_timer_;

  base::TimeDelta GetNextRetryDelay();
  base::TimeDelta GetTimeoutDelay();
  base::TimeDelta GetTerminateDelay();
};

}  // namespace sippet

#endif  // SIPPET_CLIENT_TRANSACTION_H_
