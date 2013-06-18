// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHANNEL_BASE_H_
#define SIPPET_TRANSPORT_CHANNEL_BASE_H_

#include <deque>
#include "sippet/transport/channel.h"

namespace net {
  class Socket;
  class DrainableIOBuffer;
} // End of namespace net

namespace sippet {

class NET_EXPORT_PRIVATE ChannelBase :
  public Channel {
 public:
  ChannelBase(net::Socket *socket);
  virtual ~ChannelBase();

  // Channel methods:
  virtual int Send(const Message& message,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual void CloseWithError(int err) OVERRIDE;

 private:
  net::Socket *socket_;

  struct PendingMessage {
    PendingMessage(net::DrainableIOBuffer *block,
                   const net::CompletionCallback& callback)
      : block_(block), callback_(callback) {}
    scoped_refptr<net::DrainableIOBuffer> block_;
    net::CompletionCallback callback_;
  };

  std::deque<PendingMessage*> pending_messages_;

  void DidWrite(int result);
  void DidConsume(int result);
  void Pop(int result);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_CHANNEL_BASE_H_
