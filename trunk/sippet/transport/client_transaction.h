// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CLIENT_TRANSACTION_H_
#define SIPPET_TRANSPORT_CLIENT_TRANSACTION_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include "sippet/message/message.h"

namespace sippet {

class ClientTransaction :
  public base::RefCountedThreadSafe<ClientTransaction> {
 private:
  DISALLOW_COPY_AND_ASSIGN(ClientTransaction);
 public:
  ClientTransaction() {}
  virtual ~ClientTransaction() {}

  virtual const std::string& id() const = 0;
  virtual scoped_refptr<Channel> channel() const = 0;

  virtual void Start(const scoped_refptr<Request> &outgoing_request) = 0;

  virtual void HandleIncomingResponse(
                    const scoped_refptr<Response> &response) = 0;

  virtual void Close() = 0;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CLIENT_TRANSACTION_H_
