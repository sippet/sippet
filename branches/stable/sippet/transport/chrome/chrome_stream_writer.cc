// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_writer.h"

#include "base/stl_util.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/socket/socket.h"

namespace sippet {

ChromeStreamWriter::PendingBlock::PendingBlock(
        net::DrainableIOBuffer *io_buffer,
        const net::CompletionCallback& callback)
  : io_buffer_(io_buffer), callback_(callback) {
}

ChromeStreamWriter::PendingBlock::~PendingBlock() {
}

ChromeStreamWriter::ChromeStreamWriter(
    net::Socket *socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      weak_factory_(this),
      error_(net::OK) {
}

ChromeStreamWriter::~ChromeStreamWriter() {
  STLDeleteElements(&pending_messages_);
}

int ChromeStreamWriter::Write(
    net::IOBuffer* buf, int buf_len,
    const net::CompletionCallback& callback) {
  if (error_ != net::OK)
    return error_;

  scoped_refptr<net::DrainableIOBuffer> io_buffer(
      new net::DrainableIOBuffer(buf, buf_len));
  if (pending_messages_.empty()) {
    int res = Drain(io_buffer);
    if (res == net::OK || res != net::ERR_IO_PENDING) {
      error_ = res;
      return res;
    }
  }

  pending_messages_.push_back(new PendingBlock(io_buffer, callback));
  return net::ERR_IO_PENDING;
}

void ChromeStreamWriter::CloseWithError(int err) {
  error_ = err;
  while (!pending_messages_.empty())
    Pop(err);
}

void ChromeStreamWriter::DidWrite(int result) {
  DCHECK(!pending_messages_.empty());

  if (result > 0)
    DidConsume(result);
  else {
    if (result == 0)
      result = net::ERR_CONNECTION_RESET;
    CloseWithError(result);
  }
}

void ChromeStreamWriter::DidConsume(int result) {
  PendingBlock *pending = pending_messages_.front();
  pending->io_buffer_->DidConsume(result);
  for (;;) {
    if (pending->io_buffer_->BytesRemaining() == 0) {
      Pop(net::OK);
      if (pending_messages_.empty())
        break; // done
      pending = pending_messages_.front();
      continue;
    }
    else {
      result = Drain(pending->io_buffer_);
      if (result < 0) {
        if (result != net::ERR_IO_PENDING)
          CloseWithError(result);
        break;
      }
    }
  }
}

void ChromeStreamWriter::Pop(int result) {
  PendingBlock *pending = pending_messages_.front();
  pending->callback_.Run(result);
  delete pending;
  pending_messages_.pop_front();
}

int ChromeStreamWriter::Drain(net::DrainableIOBuffer* buf) {
  int res;
  do {
    res = wrapped_socket_->Write(
        buf, buf->BytesRemaining(),
        base::Bind(&ChromeStreamWriter::DidWrite,
                   base::Unretained(this)));
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
