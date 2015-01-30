// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TRANSACTION_DELEGATE_H_
#define SIPPET_TRANSPORT_TRANSACTION_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "sippet/message/message.h"

namespace sippet {

class TransactionDelegate {
 public:
  virtual ~TransactionDelegate() {}

  // Called when the transaction wants to deliver a response upside.
  virtual void OnIncomingResponse(const scoped_refptr<Response> &) = 0;

  // Called when the transaction timed-out.
  virtual void OnTimedOut(const scoped_refptr<Request> &request) = 0;

  // Called when the transaction detects a network error.
  virtual void OnTransportError(
      const scoped_refptr<Request> &request, int error) = 0;

  // Called when the transaction has ended.
  virtual void OnTransactionTerminated(const std::string &transaction_id) = 0;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_TRANSACTION_DELEGATE_H_
