// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/channel_base.h"

#include "base/stl_util.h"
#include "net/socket/socket.h"
#include "net/base/net_errors.h"
#include "sippet/message/message.h"

namespace sippet {

ChannelBase::ChannelBase(net::Socket *socket)
  : socket_(socket) {
}

ChannelBase::~ChannelBase() {
  STLDeleteElements(&pending_messages_);
}

int ChannelBase::Send(const Message& message,
                      const net::CompletionCallback& callback) {
  std::string serialized(message.ToString());
  scoped_refptr<net::IOBuffer> block(
      new net::IOBuffer(serialized.size()));
  memcpy(block->data(), serialized.data(), serialized.size());

  int done = 0;
  if (pending_messages_.empty()) {
    int res = socket_->Write(block, serialized.size(),
        base::Bind(&ChannelBase::DidWrite, this));
    if (res > 0) {
      if (res == serialized.size())
        return net::OK;
      else
        done = res;
    }
    else if (res == 0)
      return net::ERR_CONNECTION_RESET;
    else if (res != net::ERR_IO_PENDING)
      return res;
  }

  scoped_refptr<net::DrainableIOBuffer> drainable_block(
      new net::DrainableIOBuffer(block, serialized.size()));
  if (done > 0)
    drainable_block->DidConsume(done);
  pending_messages_.push_back(new PendingMessage(drainable_block, callback));
  return net::ERR_IO_PENDING;
}

void ChannelBase::CloseWithError(int err) {
  while (!pending_messages_.empty())
    Pop(err);
}

void ChannelBase::DidWrite(int result) {
  DCHECK(!pending_messages_.empty());

  if (result > 0)
    DidConsume(result);
  else
    CloseWithError(result);
}

void ChannelBase::DidConsume(int result) {
  do {
    PendingMessage *pending = pending_messages_.front();
    pending->block_->DidConsume(result);
    if (pending->block_->BytesRemaining() == 0) {
      Pop(net::OK);
      continue;
    }
    else {
      result = socket_->Write(pending->block_,
          pending->block_->BytesRemaining(),
          base::Bind(&ChannelBase::DidWrite, this));
      if (result > 0)
        continue; // synchronous write
      else if (result == net::ERR_IO_PENDING)
        break; // yield, wait for more data
      else
        CloseWithError(result); // synchronous error
    }
  } while (!pending_messages_.empty());
}

void ChannelBase::Pop(int result) {
  if (result == 0) {
    // Emulates a connection reset.  The net::Socket documentation says
    // the behavior is undefined when writing to a closed socket, but
    // normally a zero is given by the OS to indicate that the connection
    // have been reset by peer.
    result = net::ERR_CONNECTION_RESET;
  }

  PendingMessage *pending = pending_messages_.front();
  pending->callback_.Run(result);
  delete pending;
  pending_messages_.pop_front();
}

} // End of sippet namespace
