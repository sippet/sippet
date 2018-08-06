// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_SERVER_TRANSACTION_H_
#define SIPPET_SERVER_TRANSACTION_H_

#include "base/timer/timer.h"
#include "base/memory/ref_counted.h"
#include "sippet/transaction_config.h"

namespace sippet {
class Request;
class Response;
class TransportLayer;

class ServerTransaction : public base::RefCounted<ServerTransaction> {
 public:
  ServerTransaction(scoped_refptr<Request> request,
      const TransactionConfig& config, TransportLayer* transport_layer);

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
  friend class base::RefCounted<ServerTransaction>;
  virtual ~ServerTransaction();

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

#endif  // SIPPET_SERVER_TRANSACTION_H_
