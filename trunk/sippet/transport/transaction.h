// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TRANSACTION_H_
#define SIPPET_TRANSPORT_TRANSACTION_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include "sippet/message/message.h"

namespace sippet {

class Transaction :
  public base::RefCountedThreadSafe<Transaction> {
 private:
  DISALLOW_COPY_AND_ASSIGN(Transaction);
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Called when the transaction wants to deliver a message upside.
    virtual void OnPassMessage(const scoped_refptr<Message> &) = 0;

    // Called when the transaction has ended.
    virtual void OnTransactionTerminated(
        const scoped_refptr<Transaction> &transaction) = 0;
  };

  virtual ~Transaction() {}
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_TRANSACTION_H_
