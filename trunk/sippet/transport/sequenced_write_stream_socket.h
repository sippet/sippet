// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_SEQUENCED_WRITE_STREAM_SOCKET_H_
#define SIPPET_TRANSPORT_SEQUENCED_WRITE_STREAM_SOCKET_H_

#include <deque>
#include "base/memory/weak_ptr.h"
#include "net/base/net_log.h"
#include "net/socket/stream_socket.h"

namespace base {
class TimeDelta;
}

namespace net {
class IPEndPoint;
class DrainableIOBuffer;
}

namespace sippet {

// This is a wrapper for stream sockets. Data frames are buffered
// locally when the wrapped socket returns asynchronously for Write().
// Each enqueued frame will be notified after write completion.
//
// There are no bounds on the local buffer size. Use carefully.
class NET_EXPORT_PRIVATE SequencedWriteStreamSocket : public net::StreamSocket {
 public:
  SequencedWriteStreamSocket(net::StreamSocket* socket_to_wrap);
  virtual ~SequencedWriteStreamSocket();

  // Socket interface
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE;
  virtual bool SetReceiveBufferSize(int32 size) OVERRIDE;
  virtual bool SetSendBufferSize(int32 size) OVERRIDE;

  // StreamSocket interface
  virtual int Connect(const net::CompletionCallback& callback) OVERRIDE;
  virtual void Disconnect() OVERRIDE;
  virtual bool IsConnected() const OVERRIDE;
  virtual bool IsConnectedAndIdle() const OVERRIDE;
  virtual int GetPeerAddress(net::IPEndPoint* address) const OVERRIDE;
  virtual int GetLocalAddress(net::IPEndPoint* address) const OVERRIDE;
  virtual const net::BoundNetLog& NetLog() const OVERRIDE;
  virtual void SetSubresourceSpeculation() OVERRIDE;
  virtual void SetOmniboxSpeculation() OVERRIDE;
  virtual bool WasEverUsed() const OVERRIDE;
  virtual bool UsingTCPFastOpen() const OVERRIDE;
  virtual int64 NumBytesRead() const OVERRIDE;
  virtual base::TimeDelta GetConnectTimeMicros() const OVERRIDE;
  virtual bool WasNpnNegotiated() const OVERRIDE;
  virtual net::NextProto GetNegotiatedProtocol() const OVERRIDE;
  virtual bool GetSSLInfo(net::SSLInfo* ssl_info) OVERRIDE;

 private:
  scoped_ptr<net::StreamSocket> wrapped_socket_;
  base::WeakPtrFactory<SequencedWriteStreamSocket> weak_factory_;
  int error_;

  struct PendingBlock {
    PendingBlock(net::DrainableIOBuffer *io_buffer,
                 const net::CompletionCallback& callback)
      : io_buffer_(io_buffer), callback_(callback) {}
    scoped_refptr<net::DrainableIOBuffer> io_buffer_;
    net::CompletionCallback callback_;
  };

  std::deque<PendingBlock*> pending_messages_;

  void CloseWithError(int err);
  void DidWrite(int result);
  void DidConsume(int result);
  void Pop(int result);
  int Drain(net::DrainableIOBuffer* buf);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_SEQUENCED_WRITE_STREAM_SOCKET_H_
