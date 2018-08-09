// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/client_transaction.h"

#include "base/sequenced_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "sippet/base/random_strings.h"
#include "sippet/core.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/transport_layer.h"
#include "sippet/transaction_layer_core.h"

namespace sippet {

ClientTransaction::ClientTransaction(scoped_refptr<Request> request,
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
      branch_id_(CreateBranch()),
      weak_ptr_factory_(this) {
  DCHECK(request);
  DCHECK(transport_layer);
  DCHECK(core);
  DCHECK(core_task_runner);

  key_ = "C->" + branch_id_ + ":" + request->request_method();
}

ClientTransaction::~ClientTransaction() {}

void ClientTransaction::Start() {
  if (request_->request_method() == Request::kInvite) {
    next_state_ = STATE_CALLING;
  } else {
    next_state_ = STATE_TRYING;
  }

  transport_layer_->Connect(request_->request_uri(),
      base::Bind(&ClientTransaction::OnConnect, weak_ptr_factory_.GetWeakPtr()));
}

void ClientTransaction::ReceiveResponse(scoped_refptr<Response> response) {
  DCHECK(response);
  DCHECK(next_state_ != STATE_TERMINATED);

  State state = next_state_;
  int response_code = response->response_code();

  switch (state) {
    case STATE_TRYING:
      switch (response_code / 100) {
        case 1: next_state_ = STATE_PROCEEDING; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_CALLING:
      switch (response_code / 100) {
        case 1: next_state_ = STATE_PROCEED_CALLING; break;
        // When a 2xx arrives and the transaction is of INVITE type, don't
        // terminate immediately, leave the transaction opened to avoid
        // retransmissions with null refer_to attribute.
        case 2: next_state_ = STATE_COMPLETED; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_PROCEEDING:
      switch (response_code / 100) {
        case 1: break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_PROCEED_CALLING:
      switch (response_code / 100) {
        case 1: break;
        // Same as above.
        case 2: next_state_ = STATE_COMPLETED; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_COMPLETED: break;
    default:
      NOTREACHED() << "bad state " << state;
  }

  if (request_->request_method() == Request::kInvite
      && response_code / 100 >= 3) {
    std::string to_tag;
    std::unordered_map<std::string, std::string> parameters;
    if (response->GetTo(nullptr, nullptr, &parameters)) {
      auto it = parameters.find("tag");
      if (it != parameters.end())
        to_tag = it->second;
    }
    SendAck(to_tag);
  }

  if (next_state_ == STATE_COMPLETED) {
    if (state == STATE_TRYING
        || state == STATE_PROCEEDING
        || state == STATE_CALLING) {
      StopTimers();
    }
  } else if (next_state_ == STATE_PROCEED_CALLING) {
    if (state == STATE_CALLING) {
      StopTimers();
    }
  }

  if (state != STATE_COMPLETED) {
    core_task_runner_->PostTask(FROM_HERE,
        base::Bind(&Core::OnIncomingResponse, base::Unretained(core_),
            response));
  }

  if (STATE_COMPLETED == next_state_) {
    std::string protocol;
    request_->EnumerateVia(nullptr, &protocol, nullptr, nullptr);
    if (protocol != Message::kUdp)
      next_state_ = STATE_TERMINATED;
    else if (next_state_ != state)
      ScheduleTerminate();
  }

  if (STATE_TERMINATED == next_state_)
    Terminate();
}

void ClientTransaction::Terminate() {
  transaction_layer_core_->RemoveClientTransaction(this);
}

void ClientTransaction::ScheduleRetry() {
  retry_timer_.Start(FROM_HERE, GetNextRetryDelay(),
      base::Bind(&ClientTransaction::RetransmitRequest,
          weak_ptr_factory_.GetWeakPtr()));
}

void ClientTransaction::ScheduleTimeout() {
  timeout_timer_.Start(FROM_HERE, GetTimeoutDelay(),
      base::Bind(&ClientTransaction::RequestTimeout,
          weak_ptr_factory_.GetWeakPtr()));
}

void ClientTransaction::ScheduleTerminate() {
  terminate_timer_.Start(FROM_HERE, GetTerminateDelay(),
      base::Bind(&ClientTransaction::Terminate,
          weak_ptr_factory_.GetWeakPtr()));
}

void ClientTransaction::RetransmitRequest() {
  if (request_->request_method() == Request::kInvite) {
    DCHECK(STATE_CALLING == next_state_);
  } else {
    DCHECK(STATE_TRYING == next_state_ || STATE_PROCEEDING == next_state_);
  }

  connection_->SendMessage(request_);

  ScheduleRetry();
}

void ClientTransaction::RequestTimeout() {
  if (request_->request_method() == Request::kInvite) {
    DCHECK(STATE_CALLING == next_state_);
  } else {
    DCHECK(STATE_TRYING == next_state_ || STATE_PROCEEDING == next_state_);
  }

  State state = next_state_;
  next_state_ = STATE_TERMINATED;
  if (STATE_COMPLETED != state) {
    core_task_runner_->PostTask(FROM_HERE,
        base::Bind(&Core::OnTimedOut, base::Unretained(core_),
            request_->client_key()));
  }

  Terminate();
}

void ClientTransaction::StopTimers() {
  retry_timer_.Stop();
  timeout_timer_.Stop();
  terminate_timer_.Stop();
}

void ClientTransaction::SendAck(const std::string &to_tag) {
  DCHECK(connection_);
  if (!ack_)
    ack_ = request_->CreateAck(to_tag);
  connection_->SendMessage(ack_);
}

void ClientTransaction::OnConnect(
    scoped_refptr<TransportLayer::Connection> connection) {
  connection_ = connection;

  // Set the Via header in the request now.
  std::string protocol = connection_->GetTransportProtocol();
  net::HostPortPair destination = connection_->GetDestination();
  request_->AddHeader("Via: SIP/2.0/" + protocol + " " + destination.host() +
      ":" + base::IntToString(destination.port()) + ";branch=" + branch_id_ + ";rport");

  connection_->SendMessage(request_);

  if (protocol == Message::kUdp)
    ScheduleRetry();
  ScheduleTimeout();
}

base::TimeDelta ClientTransaction::GetNextRetryDelay() {
  if (request_->request_method() == Request::kInvite) {
    // Implement the exponential backoff *2 at each retransmission
    return config_.t1 * (1 << (++retransmissions_));
  } else {
    // Implement the exponential backoff from T1 up to T2
    return std::min(config_.t1 * (1 << (++retransmissions_)), config_.t2);
  }
}

base::TimeDelta ClientTransaction::GetTimeoutDelay() {
  return 64 * config_.t1;
}

base::TimeDelta ClientTransaction::GetTerminateDelay() {
  if (request_->request_method() == Request::kInvite) {
    return 64 * config_.t1;
  } else {
    return config_.t4;
  }
}

}  // namespace sippet