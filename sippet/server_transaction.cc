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
      scoped_refptr<TransportLayer::Connection> connection,
      const TransactionConfig& config,
      scoped_refptr<base::SequencedTaskRunner> core_task_runner,
      Core* core, TransactionLayerCore* transaction_layer_core)
    : request_(request),
      config_(config),
      retransmissions_(0),
      core_(core),
      core_task_runner_(core_task_runner),
      transaction_layer_core_(transaction_layer_core),
      connection_(connection),
      weak_ptr_factory_(this) {
  DCHECK(request);
  DCHECK(core);
  DCHECK(core_task_runner);

  key_ = request->server_key();
}

ServerTransaction::~ServerTransaction() {}

void ServerTransaction::Start() {
  if (request_->request_method() == Request::kInvite) {
    next_state_ = STATE_PROCEED_CALLING;
    ScheduleProvisionalResponse();
  } else {
    next_state_ = STATE_TRYING;
  }
}

void ServerTransaction::ReceiveRequest(scoped_refptr<Request> request) {
  DCHECK(request);
  DCHECK(next_state_ != STATE_TERMINATED);

  if (request->request_method() != Request::kAck) {
    if (STATE_PROCEEDING == next_state_
        || STATE_PROCEED_CALLING == next_state_
        || STATE_COMPLETED == next_state_) {
      if (response_) {
        connection_->SendMessage(response_);
      }
    }
  }

  State state = next_state_;
  switch (state) {
    case STATE_TRYING:
      break;
    case STATE_PROCEEDING:
    case STATE_PROCEED_CALLING:
      break;
    case STATE_COMPLETED:
      if (request->request_method() == Request::kAck) {
        next_state_ = STATE_CONFIRMED;
      }
      break;
    case STATE_CONFIRMED:
      break;
    default:
      NOTREACHED();
      break;
  }

  if (STATE_CONFIRMED == next_state_
      && next_state_ != state) {
    StopTimers();
    std::string protocol;
    request_->EnumerateVia(nullptr, &protocol, nullptr, nullptr);
    if (protocol != Message::kUdp) {
      next_state_ = STATE_TERMINATED;
    } else {
      ScheduleTerminate();
    }
  }

  if (STATE_TERMINATED == next_state_) {
    Terminate();
  }
}

void ServerTransaction::SendResponse(scoped_refptr<Response> response) {
  DCHECK(response);

  if (STATE_PROCEED_CALLING < next_state_) {
    DVLOG(1) << "Ignored second final response attempt";
    return;
  }

  if (STATE_PROCEED_CALLING == next_state_)
    StopProvisionalResponse();

  response_ = response;
  connection_->SendMessage(response);

  State state = next_state_;
  int response_code = response->response_code();
  switch (state) {
    case STATE_TRYING:
      switch (response_code/100) {
        case 1: next_state_ = STATE_PROCEEDING; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
    case STATE_PROCEEDING:
      switch (response_code/100) {
        case 1: break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_PROCEED_CALLING:
      switch (response_code/100) {
        case 1: break;
        case 2: next_state_ = STATE_TERMINATED; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    default:
      NOTREACHED() << "bad state " << state;
  }

  if (STATE_COMPLETED == next_state_
      && next_state_ != state) {
    std::string protocol;
    request_->EnumerateVia(nullptr, &protocol, nullptr, nullptr);
    if (request_->request_method() == Request::kInvite) {
      if (protocol == Message::kUdp)
        ScheduleRetry();
      ScheduleTimeout();
    } else {
      if (protocol != Message::kUdp) {
        next_state_ = STATE_TERMINATED;
      } else {
        ScheduleTerminate();
      }
    }
  }

  if (STATE_TERMINATED == next_state_)
    Terminate();
}

void ServerTransaction::Terminate() {
  transaction_layer_core_->RemoveServerTransaction(this);
}

void ServerTransaction::ScheduleRetry() {
  retry_timer_.Start(FROM_HERE, GetNextRetryDelay(),
      base::Bind(&ServerTransaction::RetransmitResponse,
          weak_ptr_factory_.GetWeakPtr()));
}

void ServerTransaction::ScheduleTimeout() {
  timeout_timer_.Start(FROM_HERE, GetTimeoutDelay(),
    base::Bind(&ServerTransaction::ResponseTimeout,
        weak_ptr_factory_.GetWeakPtr()));
}

void ServerTransaction::ScheduleTerminate() {
  terminate_timer_.Start(FROM_HERE, GetTerminateDelay(),
    base::Bind(&ServerTransaction::Terminate,
        weak_ptr_factory_.GetWeakPtr()));
}

void ServerTransaction::ScheduleProvisionalResponse() {
  base::TimeDelta delay = base::TimeDelta::FromMilliseconds(200);
  provisional_response_timer_.Start(FROM_HERE, delay,
      base::Bind(&ServerTransaction::SendProvisionalResponse,
          weak_ptr_factory_.GetWeakPtr()));
}

void ServerTransaction::RetransmitResponse() {
  DCHECK(request_->request_method() == Request::kInvite);
  DCHECK(STATE_COMPLETED == next_state_);

  connection_->SendMessage(response_);

  ScheduleRetry();
}

void ServerTransaction::ResponseTimeout() {
  DCHECK(request_->request_method() == Request::kInvite);
  DCHECK(STATE_COMPLETED == next_state_);

  next_state_ = STATE_TERMINATED;
  core_task_runner_->PostTask(FROM_HERE,
      base::Bind(&Core::OnTimedOut, base::Unretained(core_),
          request_->server_key()));

  Terminate();
}

void ServerTransaction::SendProvisionalResponse() {
  DCHECK(request_->request_method() == Request::kInvite);
  DCHECK(STATE_PROCEED_CALLING == next_state_);
  DCHECK(!response_ || response_->response_code() == 100);

  if (!response_) {
    response_ = request_->CreateResponse(100);
  }

  connection_->SendMessage(response_);
}

void ServerTransaction::StopTimers() {
  retry_timer_.Stop();
  timeout_timer_.Stop();
  terminate_timer_.Stop();
  provisional_response_timer_.Stop();
}

void ServerTransaction::StopProvisionalResponse() {
  provisional_response_timer_.Stop();
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