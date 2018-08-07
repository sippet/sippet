// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/server_transaction.h"

#include "base/sequenced_task_runner.h"
#include "sippet/core.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/transport_layer.h"
#include "sippet/transaction_layer_core.h"

namespace sippet {

ServerTransaction::ServerTransaction(scoped_refptr<Request> request,
      const TransactionConfig& config, TransportLayer* transport_layer,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner, Core* core,
      TransactionLayerCore* transaction_layer_core)
    : request_(request),
      config_(config),
      transport_layer_(transport_layer),
      retransmissions_(0),
      core_(core),
      core_task_runner_(core_task_runner),
      transaction_layer_core_(transaction_layer_core),
      weak_ptr_factory_(this) {
  DCHECK(request);
  DCHECK(transport_layer);
  DCHECK(core);
  DCHECK(core_task_runner);
}

ServerTransaction::~ServerTransaction() {}

void ServerTransaction::Start() {
  if (request_->request_method() == Request::kInvite) {
    next_state_ = STATE_PROCEED_CALLING;
    //ScheduleProvisionalResponse();
  } else {
    next_state_ = STATE_TRYING;
  }
}

void ServerTransaction::ReceiveRequest(scoped_refptr<Request> request) {
  // TODO(guibv)
}

void ServerTransaction::SendResponse(scoped_refptr<Response> response) {
  // TODO(guibv)
}

void ServerTransaction::Terminate() {
  transaction_layer_core_->RemoveServerTransaction(request_->server_key());
}

base::TimeDelta ServerTransaction::GetNextRetryDelay() {
  DCHECK(request_->request_method() == Request::kInvite)
      << "There's no retry on server non-INVITE transactions";
  return std::min(config_.t1 * (1 << (++retransmissions_)), config_.t2);
}

base::TimeDelta ServerTransaction::GetTimeoutDelay() {
  DCHECK(request_->request_method() == Request::kInvite)
      << "There's no timeout delay on server non-INVITE transactions";
  return 64 * config_.t1;
}

base::TimeDelta ServerTransaction::GetTerminateDelay() {
  if (request_->request_method() == Request::kInvite) {
    return config_.t4;
  } else {
    return 64 * config_.t1;
  }
}

}  // namespace sippet