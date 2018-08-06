// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_LAYER_CORE_H_
#define SIPPET_TRANSACTION_LAYER_CORE_H_

#include <unordered_map>

#include "base/memory/ref_counted.h"
#include "sippet/sippet_export.h"
#include "sippet/transaction_layer.h"
#include "sippet/transaction_config.h"

namespace base {
class SequencedTaskRunner;
class SingleThreadTaskRunner;
}  // namespace base

namespace sippet {
class Core;
class ClientTransaction;
class ServerTransaction;
class TransactionLayerImpl;

class SIPPET_EXPORT_PRIVATE TransactionLayerCore
  : public base::RefCountedThreadSafe<TransactionLayerCore> {
 public:
  TransactionLayerCore(TransportLayer* transport_layer, Core* core,
      const TransactionConfig& config);

  // Starts the layer. It's important that this does not happen in the
  // constructor because it causes the IO thread to begin AddRef()ing and
  // Release()ing us. If our caller hasn't had time to fully construct us and
  // take a reference, the IO thread could interrupt things, run a task,
  // Release() us, and destroy us, leaving the caller with an already-destroyed
  // object when construction finishes.
  void Start();

  // Stops any in-progress load and ensures no callback will happen. It is
  // safe to call this multiple times.
  void Stop();

  // TransactionLayer-like functions.
  void SetRequestContext(
      net::URLRequestContextGetter* request_context_getter);
  void SendRequest(scoped_refptr<Request> request);
  void SendResponse(scoped_refptr<Response> response);
  void Terminate(const std::string& id);
  void OnMessage(scoped_refptr<Message> message);
  void OnTransportError(const std::string& id, int error);

  // Called by ClientTransaction and ServerTransaction to remove themselves.
  void RemoveClientTransaction(const std::string& id);
  void RemoveServerTransaction(const std::string& id);

 private:
  friend class base::RefCountedThreadSafe<sippet::TransactionLayerCore>;

  ~TransactionLayerCore();

  // Wrapper functions that allow us to ensure actions happen on the right
  // thread.
  void SendRequestOnIOThread(scoped_refptr<Request> request);
  void SendResponseOnIOThread(scoped_refptr<Response> response);
  void TerminateOnIOThread(const std::string& id);
  void CancelAllTransactions(int error);

  const TransactionConfig config_;  // The transaction config
  TransportLayer* transport_layer_;  // The transport layer
  Core* core_;  // Object to notify on events
  // Task runner for the creating sequence. Used to interact with the core.
  const scoped_refptr<base::SequencedTaskRunner> core_task_runner_;
  // Task runner for network operations.
  scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;

  // Transaction maps.
  std::unordered_map<std::string, scoped_refptr<ClientTransaction>>
      client_transactions_map_;
  std::unordered_map<std::string, scoped_refptr<ServerTransaction>>
      server_transactions_map_;

  // The URL request context getter.
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;

  // Lock for synchronizing access to

  DISALLOW_COPY_AND_ASSIGN(TransactionLayerCore);
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_LAYER_CORE_H_
