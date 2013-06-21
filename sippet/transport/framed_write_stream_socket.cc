// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/framed_write_stream_socket.h"

#include "base/stl_util.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"

namespace sippet {

FramedWriteStreamSocket::FramedWriteStreamSocket(
    net::StreamSocket *socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)),
      error_(net::OK) {
}

FramedWriteStreamSocket::~FramedWriteStreamSocket() {
  STLDeleteElements(&pending_messages_);
}

int FramedWriteStreamSocket::Read(net::IOBuffer* buf, int buf_len,
                                  const net::CompletionCallback& callback) {
  return wrapped_socket_->Read(buf, buf_len, callback);
}

int FramedWriteStreamSocket::Write(net::IOBuffer* buf, int buf_len,
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

  pending_messages_.push_back(new PendingFrame(io_buffer, callback));
  return net::ERR_IO_PENDING;
}

bool FramedWriteStreamSocket::SetReceiveBufferSize(int32 size) {
  return wrapped_socket_->SetReceiveBufferSize(size);
}

bool FramedWriteStreamSocket::SetSendBufferSize(int32 size) {
  return wrapped_socket_->SetSendBufferSize(size);
}

int FramedWriteStreamSocket::Connect(const net::CompletionCallback& callback) {
  return wrapped_socket_->Connect(callback);
}

void FramedWriteStreamSocket::Disconnect() {
  wrapped_socket_->Disconnect();
  CloseWithError(net::ERR_CONNECTION_RESET);
}

bool FramedWriteStreamSocket::IsConnected() const {
  return wrapped_socket_->IsConnected();
}

bool FramedWriteStreamSocket::IsConnectedAndIdle() const {
  return wrapped_socket_->IsConnectedAndIdle();
}

int FramedWriteStreamSocket::GetPeerAddress(net::IPEndPoint* address) const {
  return wrapped_socket_->GetPeerAddress(address);
}

int FramedWriteStreamSocket::GetLocalAddress(net::IPEndPoint* address) const {
  return wrapped_socket_->GetLocalAddress(address);
}

const net::BoundNetLog& FramedWriteStreamSocket::NetLog() const {
  return wrapped_socket_->NetLog();
}

void FramedWriteStreamSocket::SetSubresourceSpeculation() {
  wrapped_socket_->SetSubresourceSpeculation();
}

void FramedWriteStreamSocket::SetOmniboxSpeculation() {
  wrapped_socket_->SetOmniboxSpeculation();
}

bool FramedWriteStreamSocket::WasEverUsed() const {
  return wrapped_socket_->WasEverUsed();
}

bool FramedWriteStreamSocket::UsingTCPFastOpen() const {
  return wrapped_socket_->UsingTCPFastOpen();
}

int64 FramedWriteStreamSocket::NumBytesRead() const {
  return wrapped_socket_->NumBytesRead();
}

base::TimeDelta FramedWriteStreamSocket::GetConnectTimeMicros() const {
  return wrapped_socket_->GetConnectTimeMicros();
}

bool FramedWriteStreamSocket::WasNpnNegotiated() const {
  return wrapped_socket_->WasNpnNegotiated();
}

net::NextProto FramedWriteStreamSocket::GetNegotiatedProtocol() const {
  return wrapped_socket_->GetNegotiatedProtocol();
}

bool FramedWriteStreamSocket::GetSSLInfo(net::SSLInfo* ssl_info) {
  return wrapped_socket_->GetSSLInfo(ssl_info);
}

void FramedWriteStreamSocket::CloseWithError(int err) {
  error_ = err;
  while (!pending_messages_.empty())
    Pop(err);
}

void FramedWriteStreamSocket::DidWrite(int result) {
  DCHECK(!pending_messages_.empty());

  if (result > 0)
    DidConsume(result);
  else {
    if (result == 0)
      result = net::ERR_CONNECTION_RESET;
    CloseWithError(result);
  }
}

void FramedWriteStreamSocket::DidConsume(int result) {
  PendingFrame *pending = pending_messages_.front();
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

void FramedWriteStreamSocket::Pop(int result) {
  PendingFrame *pending = pending_messages_.front();
  pending->callback_.Run(result);
  delete pending;
  pending_messages_.pop_front();
}

int FramedWriteStreamSocket::Drain(net::DrainableIOBuffer* buf) {
  int res;
  do {
    res = wrapped_socket_->Write(
        buf, buf->BytesRemaining(),
        base::Bind(&FramedWriteStreamSocket::DidWrite,
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
