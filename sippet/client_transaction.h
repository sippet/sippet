// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_CLIENT_TRANSACTION_H_
#define SIPPET_CLIENT_TRANSACTION_H_

#include "base/timer/timer.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/ref_counted.h"
#include "sippet/transaction_config.h"
#include "sippet/transport_layer.h"

namespace base {
class SequencedTaskRunner;
}  // namespace base

namespace sippet {
class Core;
class Request;
class Response;
class TransactionLayerCore;

class ClientTransaction {
 public:
  ClientTransaction(scoped_refptr<Request> request,
      const TransactionConfig& config, TransportLayer* transport_layer,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner, Core* core,
      TransactionLayerCore* transaction_layer_core);
  ~ClientTransaction();

  // Return the client transaction key.
  const std::string& key() const { return key_; }

  // Start the client transaction. The request is sent to the transport layer
  // at this moment.
  void Start();

  // Receive the response sending to the core if necessary. It will not send
  // retransmissions to core.
  void ReceiveResponse(scoped_refptr<Response> response);

  // Terminate this transaction immediately.
  void Terminate();

 private:
   enum State {
    STATE_CALLING,
    STATE_TRYING,
    STATE_PROCEEDING,
    STATE_PROCEED_CALLING,
    STATE_COMPLETED,
    STATE_TERMINATED
  };

  // Start timers
  void ScheduleRetry();
  void ScheduleTimeout();
  void ScheduleTerminate();

  // Timer callbacks
  void RetransmitRequest();
  void RequestTimeout();

  // Other functions
  void StopTimers();
  void SendAck(const std::string &to_tag);
  void OnConnect(scoped_refptr<TransportLayer::Connection> connection);

  // Return the timer durations
  base::TimeDelta GetNextRetryDelay();
  base::TimeDelta GetTimeoutDelay();
  base::TimeDelta GetTerminateDelay();

  State next_state_;

  scoped_refptr<Request> request_;
  scoped_refptr<Request> ack_;
  const TransactionConfig config_;
  TransportLayer* transport_layer_;
  int retransmissions_;

  Core* core_;
  scoped_refptr<base::SequencedTaskRunner> core_task_runner_;

  base::OneShotTimer retry_timer_;
  base::OneShotTimer timeout_timer_;
  base::OneShotTimer terminate_timer_;

  TransactionLayerCore* transaction_layer_core_;
  scoped_refptr<TransportLayer::Connection> connection_;
  std::string branch_id_;
  std::string key_;

  base::WeakPtrFactory<ClientTransaction> weak_ptr_factory_;
};

}  // namespace sippet

#endif  // SIPPET_CLIENT_TRANSACTION_H_