// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/stream_channel.h"

#include "base/stl_util.h"
#include "net/socket/socket.h"
#include "net/base/net_errors.h"
#include "sippet/message/message.h"

namespace sippet {

StreamChannel::StreamChannel(net::Socket *socket)
  : socket_(socket) {
}

StreamChannel::~StreamChannel() {
  STLDeleteElements(&pending_messages_);
}

int StreamChannel::Send(const Message& message,
                      const net::CompletionCallback& callback) {
  std::string data(message.ToString());
  scoped_refptr<net::IOBuffer> buffer(
      new net::IOBuffer(data.size()));
  scoped_refptr<net::DrainableIOBuffer> drainable_buffer(
      new net::DrainableIOBuffer(buffer, data.size()));
  memcpy(drainable_buffer->data(), data.data(), data.size());

  if (pending_messages_.empty()) {
    int res = Drain(drainable_buffer);
    if (res == net::OK || res != net::ERR_IO_PENDING)
      return res;
  }

  pending_messages_.push_back(new PendingMessage(drainable_buffer, callback));
  return net::ERR_IO_PENDING;
}

void StreamChannel::CloseWithError(int err) {
  while (!pending_messages_.empty())
    Pop(err);
}

void StreamChannel::DidWrite(int result) {
  DCHECK(!pending_messages_.empty());

  if (result > 0)
    DidConsume(result);
  else {
    if (result == 0)
      result = net::ERR_CONNECTION_RESET;
    CloseWithError(result);
  }
}

void StreamChannel::DidConsume(int result) {
  PendingMessage *pending = pending_messages_.front();
  pending->block_->DidConsume(result);
  for (;;) {
    if (pending->block_->BytesRemaining() == 0) {
      Pop(net::OK);
      if (pending_messages_.empty())
        break; // done
      pending = pending_messages_.front();
      continue;
    }
    else {
      result = Drain(pending->block_);
      if (result < 0) {
        if (result != net::ERR_IO_PENDING)
          CloseWithError(result);
        break;
      }
    }
  }
}

void StreamChannel::Pop(int result) {
  PendingMessage *pending = pending_messages_.front();
  pending->callback_.Run(result);
  delete pending;
  pending_messages_.pop_front();
}

int StreamChannel::Drain(net::DrainableIOBuffer* buf) {
  int res;
  do {
    res = socket_->Write(buf, buf->BytesRemaining(),
        base::Bind(&StreamChannel::DidWrite, this));
    if (res > 0) {
      buf->DidConsume(res);
      if (buf->BytesRemaining() == 0) {
        // If the buffer has been sent, return OK
        res = net::OK;
        break;
      }
      continue;
    }
    else if (res == 0) {
      // Emulates a connection reset.  The net::Socket documentation says
      // the behavior is undefined when writing to a closed socket, but
      // normally a zero is given by the OS to indicate that the connection
      // have been reset by peer.
      res = net::ERR_CONNECTION_RESET;
    }
    // In case of error, just return it
    break;
  } while (buf->BytesRemaining() > 0);
  return res;
}

} // End of sippet namespace
