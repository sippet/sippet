// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_SEQUENCED_WRITE_STREAM_SOCKET_H_
#define SIPPET_TRANSPORT_CHROME_SEQUENCED_WRITE_STREAM_SOCKET_H_

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
class SequencedWriteStreamSocket {
 public:
  SequencedWriteStreamSocket(net::StreamSocket* socket_to_wrap);
  virtual ~SequencedWriteStreamSocket();

  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback);

  void CloseWithError(int err);

 private:
  net::StreamSocket* wrapped_socket_;
  base::WeakPtrFactory<SequencedWriteStreamSocket> weak_factory_;
  int error_;

  struct PendingBlock {
    PendingBlock(net::DrainableIOBuffer *io_buffer,
                 const net::CompletionCallback& callback);
    ~PendingBlock();
    scoped_refptr<net::DrainableIOBuffer> io_buffer_;
    net::CompletionCallback callback_;
  };

  std::deque<PendingBlock*> pending_messages_;

  void DidWrite(int result);
  void DidConsume(int result);
  void Pop(int result);
  int Drain(net::DrainableIOBuffer* buf);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_SEQUENCED_WRITE_STREAM_SOCKET_H_
