// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/framed_write_stream_socket.h"

#include "base/stl_util.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"

namespace sippet {

FramedWriteStreamSocket::PendingFrame::PendingFrame(
        net::IOBuffer *buf, int buf_len,
        const net::CompletionCallback& callback)
  : buf_(buf), buf_len_(buf_len), callback_(callback) {
}

FramedWriteStreamSocket::PendingFrame::~PendingFrame() {
}

FramedWriteStreamSocket::FramedWriteStreamSocket(
    net::Socket *socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      weak_factory_(this),
      error_(net::OK) {
}

FramedWriteStreamSocket::~FramedWriteStreamSocket() {
  STLDeleteElements(&pending_messages_);
}

int FramedWriteStreamSocket::Write(net::IOBuffer* buf, int buf_len,
                                   const net::CompletionCallback& callback) {
  if (error_ != net::OK)
    return error_;

  if (pending_messages_.empty()) {
    int res = Drain(buf, buf_len);
    if (res == net::OK || res != net::ERR_IO_PENDING) {
      error_ = res;
      return res;
    }
  }

  pending_messages_.push_back(new PendingFrame(buf, buf_len, callback));
  return net::ERR_IO_PENDING;
}

void FramedWriteStreamSocket::CloseWithError(int err) {
  error_ = err;
  while (!pending_messages_.empty())
    Pop(err);
}

void FramedWriteStreamSocket::DidWrite(int result) {
  DCHECK(!pending_messages_.empty());

  if (result > 0)
    DidConsume();
  else {
    if (result == 0)
      result = net::ERR_CONNECTION_RESET;
    CloseWithError(result);
  }
}

void FramedWriteStreamSocket::DidConsume() {
  for (;;) {
    Pop(net::OK);
    if (pending_messages_.empty())
      break; // done
    PendingFrame *pending = pending_messages_.front();
    int result = Drain(pending->buf_, pending->buf_len_);
    if (result < 0) {
      if (result != net::ERR_IO_PENDING)
        CloseWithError(result);
      break;
    }
  }
}

void FramedWriteStreamSocket::Pop(int result) {
  PendingFrame *pending = pending_messages_.front();
  pending->callback_.Run(result);
  delete pending;
  pending_messages_.pop_front();
}

int FramedWriteStreamSocket::Drain(net::IOBuffer* buf, int buf_len) {
  int res = wrapped_socket_->Write(buf, buf_len,
        base::Bind(&FramedWriteStreamSocket::DidWrite,
                   base::Unretained(this)));
  if (res > 0) {
    // Pretend the whole buffer has been sent, return OK
    res = net::OK;
  }
  else if (res == 0) {
    // This can happen on ICMP error: emulates a connection reset.
    res = net::ERR_CONNECTION_RESET;
  }
  return res;
}

} // End of sippet namespace
