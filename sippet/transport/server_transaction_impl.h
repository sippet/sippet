// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_SERVER_TRANSACTION_IMPL_H_
#define SIPPET_TRANSPORT_SERVER_TRANSACTION_IMPL_H_

#include "base/timer.h"
#include "sippet/transport/server_transaction.h"
#include "sippet/transport/transaction_delegate.h"
#include "sippet/transport/time_delta_factory.h"
#include "sippet/transport/time_delta_provider.h"

namespace sippet {

class ServerTransactionImpl : public ServerTransaction {
 private:
  DISALLOW_COPY_AND_ASSIGN(ServerTransactionImpl);
 public:
  ServerTransactionImpl(
        const std::string &id,
        const scoped_refptr<Channel> &channel,
        TransactionDelegate *delegate,
        TimeDeltaFactory *time_delta_factory);
  virtual ~ServerTransactionImpl();

  // ServerTransaction methods:
  virtual const std::string& id() const OVERRIDE;
  virtual scoped_refptr<Channel> channel() const OVERRIDE;
  virtual void Start(const scoped_refptr<Request> &incoming_request) OVERRIDE;
  virtual void Send(const scoped_refptr<Response> &response) OVERRIDE;
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request> &request) OVERRIDE;

  virtual void Close() OVERRIDE;
 private:
  enum Mode {
    MODE_NORMAL,
    MODE_INVITE
  };

  enum State {
    STATE_TRYING,
    STATE_PROCEEDING,
    STATE_PROCEED_CALLING,
    STATE_COMPLETED,
    STATE_CONFIRMED,
    STATE_TERMINATED
  };

  std::string id_;
  scoped_refptr<Channel> channel_;

  Mode mode_;
  State next_state_;
  
  TransactionDelegate *delegate_;
  scoped_refptr<Request> initial_request_;
  scoped_refptr<Response> latest_response_;
  base::OneShotTimer<ServerTransactionImpl> retryTimer_;
  base::OneShotTimer<ServerTransactionImpl> timedOutTimer_;
  base::OneShotTimer<ServerTransactionImpl> terminateTimer_;
  base::OneShotTimer<ServerTransactionImpl> provisionalTimer_;
  base::WeakPtrFactory<ServerTransactionImpl> weak_factory_;

  void OnRetransmit();
  void OnTimedOut();
  void OnTerminated();
  void OnSendProvisionalResponse();
  
  void OnSendWriteComplete(scoped_refptr<Response> response, int result);
  void OnRepeatResponseWriteComplete(scoped_refptr<Request> request, int result);
  void OnRetransmitWriteComplete(int result);
  void OnSendProvisionalResponseWriteComplete(int result);

  void StopTimers();
  void StopProvisionalResponse();
  void ScheduleRetry();
  void ScheduleTimeout();
  void ScheduleTerminate();
  void ScheduleProvisionalResponse();
  void Terminate();

  TimeDeltaFactory *time_delta_factory_;
  scoped_ptr<TimeDeltaProvider> time_delta_provider_;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_SERVER_TRANSACTION_IMPL_H_
