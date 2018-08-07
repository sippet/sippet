// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_SERVER_TRANSACTION_H_
#define SIPPET_SERVER_TRANSACTION_H_

#include "base/timer/timer.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/ref_counted.h"
#include "sippet/transaction_config.h"

namespace base {
class SequencedTaskRunner;
}  // namespace base

namespace sippet {
class Core;
class Request;
class Response;
class TransportLayer;
class TransactionLayerCore;

class ServerTransaction {
 public:
  ServerTransaction(scoped_refptr<Request> request,
      const TransactionConfig& config, TransportLayer* transport_layer,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner, Core* core,
      TransactionLayerCore* transaction_layer_core);
  ~ServerTransaction();

  // Start the client transaction. The request is sent to the transport layer
  // at this moment.
  void Start();

  // Receive a request retransmission. It will not send retransmissions to
  // core.
  void ReceiveRequest(scoped_refptr<Request> request);

  // Send a response.
  void SendResponse(scoped_refptr<Response> response);

  // Terminate this transaction immediately.
  void Terminate();

 private:
   enum State {
    STATE_TRYING,
    STATE_PROCEEDING,
    STATE_PROCEED_CALLING,
    STATE_COMPLETED,
    STATE_CONFIRMED,
    STATE_TERMINATED
  };

  base::TimeDelta GetNextRetryDelay();
  base::TimeDelta GetTimeoutDelay();
  base::TimeDelta GetTerminateDelay();

  State next_state_;

  scoped_refptr<Request> request_;
  const TransactionConfig config_;
  TransportLayer* transport_layer_;
  int retransmissions_;

  Core* core_;
  scoped_refptr<base::SequencedTaskRunner> core_task_runner_;

  base::OneShotTimer retry_timer_;
  base::OneShotTimer timeout_timer_;
  base::OneShotTimer terminate_timer_;

  TransactionLayerCore* transaction_layer_core_;

  base::WeakPtrFactory<ServerTransaction> weak_ptr_factory_;
};

}  // namespace sippet

#endif  // SIPPET_SERVER_TRANSACTION_H_
