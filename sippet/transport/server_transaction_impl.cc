// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/server_transaction_impl.h"

namespace sippet {

ServerTransactionImpl::ServerTransactionImpl(
                          const std::string &id,
                          const scoped_refptr<Channel> &channel,
                          TransactionDelegate *delegate,
                          TimeDeltaFactory *time_delta_factory)
  : ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)),
    id_(id), channel_(channel), delegate_(delegate),
    time_delta_factory_(time_delta_factory) {
  DCHECK(id.length());
  DCHECK(channel);
  DCHECK(delegate);
  DCHECK(time_delta_factory);
}

ServerTransactionImpl::~ServerTransactionImpl() {}

const std::string& ServerTransactionImpl::id() const {
  return id_;
}

scoped_refptr<Channel> ServerTransactionImpl::channel() const {
  return channel_;
}

void ServerTransactionImpl::Start(
      const scoped_refptr<Request> &incoming_request) {
  DCHECK(incoming_request);
  initial_request_ = incoming_request;
  if (Method::INVITE == incoming_request->method()) {
    mode_ = MODE_INVITE;
    next_state_ = STATE_PROCEEDING;
    time_delta_provider_.reset(time_delta_factory_->CreateServerInvite());
    ScheduleProvisionalResponse();
  }
  else {
    mode_ = MODE_NORMAL;
    next_state_ = STATE_TRYING;
    time_delta_provider_.reset(time_delta_factory_->CreateServerNonInvite());
  }
}

void ServerTransactionImpl::Send(const scoped_refptr<Response> &response) {
  DCHECK(response);
  if (STATE_PROCEED_CALLING < next_state_) {
    DVLOG(1) << "Ignored second final response attempt";
    return;
  }

  if (MODE_INVITE == mode_
      && STATE_PROCEEDING == next_state_) {
    StopProvisionalResponse();
  }

  latest_response_ = response;
  channel_->Send(response, net::CompletionCallback());

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
  }
  if (STATE_COMPLETED == next_state_
      && next_state_ != state) {
    if (MODE_INVITE == mode_) {
      if (!channel_->is_stream())
        ScheduleRetry();
      ScheduleTimeout();
    }
    else {
      ScheduleTerminate();
    }
  }
  if (STATE_TERMINATED == next_state_) {
    Terminate();
  }
}

void ServerTransactionImpl::HandleIncomingRequest(
      const scoped_refptr<Request> &request) {
  DCHECK(request);
  DCHECK(next_state_ != STATE_TERMINATED);

  State state = next_state_;
  switch (state) {
    case STATE_TRYING:
      break;
    case STATE_PROCEEDING:
    case STATE_PROCEED_CALLING:
      channel_->Send(latest_response_, net::CompletionCallback());
      break;
    case STATE_COMPLETED:
      if (Method::ACK == request->method())
        next_state_ = STATE_CONFIRMED;
      else
        channel_->Send(latest_response_, net::CompletionCallback());
      break;
    case STATE_CONFIRMED:
      break;
  }
  if (STATE_CONFIRMED == next_state_
      && next_state_ != state) {
    StopTimers();
    ScheduleTerminate();
  }
}

void ServerTransactionImpl::Close() {
  StopTimers();
}

void ServerTransactionImpl::OnRetransmit() {
  DCHECK(!channel_->is_stream());
  DCHECK(MODE_INVITE == mode_);
  DCHECK(STATE_COMPLETED == next_state_);
  channel_->Send(latest_response_, net::CompletionCallback());
  ScheduleRetry();
}

void ServerTransactionImpl::OnTimedOut() {
  DCHECK(MODE_INVITE == mode_);
  DCHECK(STATE_COMPLETED == next_state_);
  next_state_ = STATE_TERMINATED;
  Terminate();
}

void ServerTransactionImpl::OnTerminated() {
  DCHECK(!channel_->is_stream());
  if (MODE_INVITE == mode_)
    DCHECK(STATE_CONFIRMED == next_state_);
  else
    DCHECK(STATE_COMPLETED == next_state_);
  next_state_ = STATE_TERMINATED;
  Terminate();
}

void ServerTransactionImpl::OnSendProvisionalResponse() {
  DCHECK(MODE_INVITE == mode_ && STATE_PROCEEDING == next_state_);
  scoped_refptr<Response> response = initial_request_->MakeResponse(100);
  channel_->Send(response, net::CompletionCallback());
}

void ServerTransactionImpl::StopTimers() {
  retryTimer_.Stop();
  timedOutTimer_.Stop();
  terminateTimer_.Stop();
  provisionalTimer_.Stop();
}

void ServerTransactionImpl::StopProvisionalResponse() {
  provisionalTimer_.Stop();
}

void ServerTransactionImpl::ScheduleRetry() {
  retryTimer_.Start(FROM_HERE,
    time_delta_provider_->GetNextRetryDelay(),
    base::Bind(&ServerTransactionImpl::OnRetransmit,
      weak_factory_.GetWeakPtr()));
}

void ServerTransactionImpl::ScheduleTimeout() {
  timedOutTimer_.Start(FROM_HERE,
    time_delta_provider_->GetTimeoutDelay(),
    base::Bind(&ServerTransactionImpl::OnTimedOut,
      weak_factory_.GetWeakPtr()));
}

void ServerTransactionImpl::ScheduleTerminate() {
  terminateTimer_.Start(FROM_HERE,
    time_delta_provider_->GetTerminateDelay(),
    base::Bind(&ServerTransactionImpl::OnTerminated,
      weak_factory_.GetWeakPtr()));
}

void ServerTransactionImpl::ScheduleProvisionalResponse() {
  provisionalTimer_.Start(FROM_HERE,
    base::TimeDelta::FromMilliseconds(200),
    base::Bind(&ServerTransactionImpl::OnSendProvisionalResponse,
      weak_factory_.GetWeakPtr()));
}

void ServerTransactionImpl::Terminate() {
  delegate_->OnTransactionTerminated(id_);
}

} // End of sippet namespace
