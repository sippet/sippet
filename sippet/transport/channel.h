// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHANNEL_H_
#define SIPPET_TRANSPORT_CHANNEL_H_

#include "net/base/completion_callback.h"
#include "net/base/net_export.h"
#include "net/base/io_buffer.h"
#include "base/memory/ref_counted.h"

namespace sippet {

class Message;

class NET_EXPORT_PRIVATE Channel :
  public base::RefCountedThreadSafe<Channel> {
 public:
  Channel() {}
  virtual ~Channel() {}

  // Closes the channel emulating the given error for pending callbacks.
  virtual void CloseWithError(int err) = 0;

  // Writes the message to the underlying socket.
  // ERR_IO_PENDING is returned if the operation could not be completed
  // synchronously, in which case the result will be passed to the callback
  // when available. Returns OK on success.
  // |response| must outlive the HttpStreamBase.
  virtual int Send(const Message& message,
                   const net::CompletionCallback& callback) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Channel);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHANNEL_H_
