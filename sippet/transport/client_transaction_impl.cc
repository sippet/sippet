// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/client_transaction_impl.h"

#include "net/base/net_errors.h"

namespace sippet {

ClientTransactionImpl::ClientTransactionImpl(
                          const std::string &id,
                          const scoped_refptr<Channel> &channel,
                          TransactionDelegate *delegate,
                          TimeDeltaFactory *time_delta_factory)
  : weak_factory_(this),
    id_(id), channel_(channel), delegate_(delegate),
    time_delta_factory_(time_delta_factory) {
  DCHECK(id.length());
  DCHECK(channel);
  DCHECK(delegate);
  DCHECK(time_delta_factory);
}

ClientTransactionImpl::~ClientTransactionImpl() {}

const std::string& ClientTransactionImpl::id() const {
  return id_;
}

scoped_refptr<Channel> ClientTransactionImpl::channel() const {
  return channel_;
}

void ClientTransactionImpl::Start(
      const scoped_refptr<Request> &outgoing_request) {
  DCHECK(outgoing_request);
  initial_request_ = outgoing_request;
  if (Method::INVITE == outgoing_request->method()) {
    mode_ = MODE_INVITE;
    next_state_ = STATE_CALLING;
    time_delta_provider_.reset(time_delta_factory_->CreateClientInvite());
  }
  else {
    mode_ = MODE_NORMAL;
    next_state_ = STATE_TRYING;
    time_delta_provider_.reset(time_delta_factory_->CreateClientNonInvite());
  }
  if (!channel_->is_stream())
    ScheduleRetry();
  ScheduleTimeout();
}

void ClientTransactionImpl::HandleIncomingResponse(
      const scoped_refptr<Response> &response) {
  DCHECK(response);
  DCHECK(next_state_ != STATE_TERMINATED);

  State state = next_state_;
  int response_code = response->response_code();

  if (state != STATE_COMPLETED) {
    response->set_refer_to(initial_request_);
    delegate_->OnIncomingResponse(response);
  }

  switch (state) {
    case STATE_TRYING:
      switch (response_code/100) {
        case 1: next_state_ = STATE_PROCEEDING; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
    case STATE_CALLING:
      switch (response_code/100) {
        case 1: next_state_ = STATE_PROCEED_CALLING; break;
        case 2: next_state_ = STATE_TERMINATED; break;
        default: next_state_ = STATE_COMPLETED; break;
      }
      break;
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
    case STATE_COMPLETED: break;
    default:
      NOTREACHED() << "bad state " << state;
      Terminate();
      return;
  }

  if (mode_ == MODE_INVITE
      && response_code/100 >= 3) {
    SendAck(response->get<To>()->tag());
  }
  if (STATE_CALLING == state
      && STATE_PROCEED_CALLING == next_state_) {
    StopTimers();
  }
  if (STATE_COMPLETED == next_state_) {
    if (channel_->is_stream())
      next_state_ = STATE_TERMINATED;
    else if (next_state_ != state)
      ScheduleTerminate();
  }
  if (STATE_TERMINATED == next_state_) {
    Terminate();
  }
}

void ClientTransactionImpl::Close() {
  StopTimers();
}

void ClientTransactionImpl::OnRetransmit() {
  DCHECK(!channel_->is_stream());
  if (MODE_INVITE == mode_)
    DCHECK(STATE_CALLING == next_state_);
  else
    DCHECK(STATE_TRYING == next_state_ || STATE_PROCEEDING == next_state_);
  int result = channel_->Send(initial_request_,
    base::Bind(&ClientTransactionImpl::OnWrite, this));
  if (net::ERR_IO_PENDING != result)
    OnWrite(result);
}

void ClientTransactionImpl::OnTimedOut() {
  if (MODE_INVITE == mode_)
    DCHECK(STATE_CALLING == next_state_);
  else
    DCHECK(STATE_TRYING == next_state_ || STATE_PROCEEDING == next_state_);
  next_state_ = STATE_TERMINATED;
  delegate_->OnTimedOut(initial_request_);
  Terminate();
}

void ClientTransactionImpl::OnTerminated() {
  DCHECK(!channel_->is_stream());
  DCHECK(STATE_COMPLETED == next_state_);
  next_state_ = STATE_TERMINATED;
  Terminate();
}

void ClientTransactionImpl::OnWrite(int result) {
  if (net::OK == result) {
    ScheduleRetry();
  }
  else if (net::ERR_IO_PENDING != result) {
    delegate_->OnTransportError(initial_request_, result);
  }
}

void ClientTransactionImpl::StopTimers() {
  retryTimer_.Stop();
  timedOutTimer_.Stop();
  terminateTimer_.Stop();
}

void ClientTransactionImpl::SendAck(const std::string &to_tag) {
  if (!generated_ack_)
    ignore_result(initial_request_->CreateAck(to_tag, generated_ack_));
  channel_->Send(generated_ack_, net::CompletionCallback());
}

void ClientTransactionImpl::ScheduleRetry() {
  retryTimer_.Start(FROM_HERE,
    time_delta_provider_->GetNextRetryDelay(),
    base::Bind(&ClientTransactionImpl::OnRetransmit,
      weak_factory_.GetWeakPtr()));
}

void ClientTransactionImpl::ScheduleTimeout() {
  timedOutTimer_.Start(FROM_HERE,
    time_delta_provider_->GetTimeoutDelay(),
    base::Bind(&ClientTransactionImpl::OnTimedOut,
      weak_factory_.GetWeakPtr()));
}

void ClientTransactionImpl::ScheduleTerminate() {
  terminateTimer_.Start(FROM_HERE,
    time_delta_provider_->GetTerminateDelay(),
    base::Bind(&ClientTransactionImpl::OnTerminated,
      weak_factory_.GetWeakPtr()));
}

void ClientTransactionImpl::Terminate() {
  delegate_->OnTransactionTerminated(id_);
}

} // End of sippet namespace
