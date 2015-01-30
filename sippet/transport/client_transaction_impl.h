// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CLIENT_TRANSACTION_IMPL_H_
#define SIPPET_TRANSPORT_CLIENT_TRANSACTION_IMPL_H_

#include "base/timer/timer.h"
#include "base/memory/weak_ptr.h"
#include "sippet/transport/client_transaction.h"
#include "sippet/transport/transaction_delegate.h"
#include "sippet/transport/time_delta_factory.h"
#include "sippet/transport/time_delta_provider.h"

namespace sippet {

class ClientTransactionImpl : public ClientTransaction {
 private:
  DISALLOW_COPY_AND_ASSIGN(ClientTransactionImpl);
 public:
  ClientTransactionImpl(
        const std::string &id,
        const scoped_refptr<Channel> &channel,
        TransactionDelegate *delegate,
        TimeDeltaFactory *time_delta_factory);

  // ClientTransaction methods:
  virtual const std::string& id() const override;
  virtual scoped_refptr<Channel> channel() const override;
  virtual void Start(
      const scoped_refptr<Request> &outgoing_request) override;
  virtual void HandleIncomingResponse(
      const scoped_refptr<Response> &response) override;
  virtual void Close() override;
 private:
  friend class base::RefCountedThreadSafe<ClientTransactionImpl>;
  virtual ~ClientTransactionImpl();

  enum Mode {
    MODE_NORMAL,
    MODE_INVITE
  };

  enum State {
    STATE_CALLING,
    STATE_TRYING,
    STATE_PROCEEDING,
    STATE_PROCEED_CALLING,
    STATE_COMPLETED,
    STATE_TERMINATED
  };

  std::string id_;
  scoped_refptr<Channel> channel_;

  Mode mode_;
  State next_state_;
  
  TransactionDelegate *delegate_;
  scoped_refptr<Request> initial_request_;
  scoped_refptr<Request> generated_ack_;
  base::OneShotTimer<ClientTransactionImpl> retryTimer_;
  base::OneShotTimer<ClientTransactionImpl> timedOutTimer_;
  base::OneShotTimer<ClientTransactionImpl> terminateTimer_;
  base::WeakPtrFactory<ClientTransactionImpl> weak_factory_;

  void OnRetransmit();
  void OnTimedOut();
  void OnTerminated();
  void OnWrite(int result);

  void StopTimers();
  void SendAck(const std::string &to_tag);
  void ScheduleRetry();
  void ScheduleTimeout();
  void ScheduleTerminate();
  void Terminate();

  TimeDeltaFactory *time_delta_factory_;
  scoped_ptr<TimeDeltaProvider> time_delta_provider_;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CLIENT_TRANSACTION_IMPL_H_
