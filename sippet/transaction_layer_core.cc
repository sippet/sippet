// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_layer_core.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/strings/string_util.h"
#include "base/memory/ptr_util.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request_context_getter.h"
#include "sippet/core.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/client_transaction.h"
#include "sippet/server_transaction.h"

namespace sippet {

TransactionLayerCore::TransactionLayerCore(TransportLayer* transport_layer,
    Core* core, const TransactionConfig& config)
    : config_(config),
      transport_layer_(transport_layer),
      core_(core),
      core_task_runner_(base::SequencedTaskRunnerHandle::Get()) {
}

TransactionLayerCore::~TransactionLayerCore() {}

void TransactionLayerCore::Start() {
  DCHECK(core_task_runner_);
  DCHECK(request_context_getter_.get()) << "We need an URLRequestContext!";
  if (network_task_runner_.get()) {
    DCHECK_EQ(network_task_runner_,
              request_context_getter_->GetNetworkTaskRunner());
  } else {
    network_task_runner_ = request_context_getter_->GetNetworkTaskRunner();
  }
  DCHECK(network_task_runner_.get()) << "We need an IO task runner";
}

void TransactionLayerCore::Stop() {
  if (core_task_runner_)  // May be NULL in tests.
    DCHECK(core_task_runner_->RunsTasksOnCurrentThread());

  core_ = NULL;
  if (!network_task_runner_.get())
    return;
  if (network_task_runner_->RunsTasksOnCurrentThread()) {
    CancelAllTransactions(net::ERR_ABORTED);
  } else {
    network_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TransactionLayerCore::CancelAllTransactions, this,
            net::ERR_ABORTED));
  }
}

void TransactionLayerCore::SetRequestContext(
    net::URLRequestContextGetter* request_context_getter) {
  DCHECK(!request_context_getter_.get());
  DCHECK(request_context_getter);
  request_context_getter_ = request_context_getter;
}

void TransactionLayerCore::SendRequest(scoped_refptr<Request> request) {
  if (network_task_runner_->RunsTasksOnCurrentThread()) {
    SendRequestOnIOThread(request);
  } else {
    network_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TransactionLayerCore::SendRequestOnIOThread, this,
            request));
  }
}

void TransactionLayerCore::SendResponse(scoped_refptr<Response> response) {
  if (network_task_runner_->RunsTasksOnCurrentThread()) {
    SendResponseOnIOThread(response);
  } else {
    network_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TransactionLayerCore::SendResponseOnIOThread, this,
            response));
  }
}

void TransactionLayerCore::Terminate(const std::string& id) {
  if (network_task_runner_->RunsTasksOnCurrentThread()) {
    TerminateOnIOThread(id);
  } else {
    network_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TransactionLayerCore::TerminateOnIOThread, this, id));
  }
}

void TransactionLayerCore::OnMessage(scoped_refptr<Message> message) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());

  if (message->IsRequest()) {
    scoped_refptr<Request> request = message->as_request();
    std::string server_key = request->server_key();

    auto it = server_transactions_map_.find(server_key);
    if (it == server_transactions_map_.end()) {
      LOG(INFO) << "New server transaction " << server_key;
      std::unique_ptr<ServerTransaction> server_transaction(
          new ServerTransaction(request, config_, transport_layer_));
      server_transactions_map_[server_key] = std::move(server_transaction);
      server_transaction->Start();
    } else {
      it->second->ReceiveRequest(request);
    }
  } else {
    scoped_refptr<Response> response = message->as_response();
    std::string client_key = response->client_key();

    auto it = client_transactions_map_.find(client_key);
    if (it != client_transactions_map_.end()) {
      it->second->ReceiveResponse(response);
    }
  }
}

void TransactionLayerCore::OnTransportError(const std::string& id, int error) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());

  if (base::StartsWith(id, "C->", base::CompareCase::SENSITIVE)) {
    auto it = client_transactions_map_.find(id);
    if (it != client_transactions_map_.end()) {
      it->second->Terminate();
      client_transactions_map_.erase(it);
      core_task_runner_->PostTask(FROM_HERE,
          base::Bind(&Core::OnTransportError, base::Unretained(core_), id,
              error));
    } else {
      LOG(WARNING) << "Client transaction " << id << " not found";
    }
  } else if (base::StartsWith(id, "S->", base::CompareCase::SENSITIVE)) {
    auto it = server_transactions_map_.find(id);
    if (it != server_transactions_map_.end()) {
      it->second->Terminate();
      server_transactions_map_.erase(it);
      core_task_runner_->PostTask(FROM_HERE,
          base::Bind(&Core::OnTransportError, base::Unretained(core_), id,
              error));
    } else {
      LOG(WARNING) << "Server transaction " << id << " not found";
    }
  } else {
    LOG(WARNING) << "Invalid transaction id " << id;
  }
}

void TransactionLayerCore::RemoveClientTransaction(const std::string& id) {
  client_transactions_map_.erase(id);
}

void TransactionLayerCore::RemoveServerTransaction(const std::string& id) {
  server_transactions_map_.erase(id);
}

void TransactionLayerCore::SendRequestOnIOThread(
    scoped_refptr<Request> request) {
  std::string client_key = request->client_key();

  auto it = client_transactions_map_.find(client_key);
  if (it == client_transactions_map_.end()) {
    LOG(INFO) << "New client transaction " << client_key;
    std::unique_ptr<ClientTransaction> client_transaction(
        new ClientTransaction(request, config_, transport_layer_));
    client_transactions_map_[client_key] = std::move(client_transaction);
    client_transaction->Start();
  } else {
    LOG(WARNING) << "Trying to duplicate an existing transaction, ignored";
  }
}

void TransactionLayerCore::SendResponseOnIOThread(
    scoped_refptr<Response> response) {
  std::string server_key = response->server_key();

  auto it = server_transactions_map_.find(server_key);
  if (it != server_transactions_map_.end()) {
    it->second->SendResponse(response);
  } else {
    LOG(WARNING) << "Server transaction " << server_key << " not found";
  }
}

void TransactionLayerCore::TerminateOnIOThread(const std::string& id) {
  if (base::StartsWith(id, "C->", base::CompareCase::SENSITIVE)) {
    auto it = client_transactions_map_.find(id);
    if (it != client_transactions_map_.end()) {
      it->second->Terminate();
      client_transactions_map_.erase(it);
    }
  } else if (base::StartsWith(id, "S->", base::CompareCase::SENSITIVE)) {
    auto it = server_transactions_map_.find(id);
    if (it != server_transactions_map_.end()) {
      it->second->Terminate();
      server_transactions_map_.erase(it);
    }
  } else {
    LOG(WARNING) << "Invalid transaction id " << id;
  }
}

void TransactionLayerCore::CancelAllTransactions(int error) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());

  for (auto& it : client_transactions_map_) {
    it.second->Terminate();
  }
  for (auto& it : server_transactions_map_) {
    it.second->Terminate();
  }

  client_transactions_map_.clear();
  server_transactions_map_.clear();
}

}  // namespace sippet
