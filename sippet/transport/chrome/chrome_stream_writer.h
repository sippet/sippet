// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_STREAM_WRITER_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_STREAM_WRITER_H_

#include <deque>
#include "base/memory/weak_ptr.h"
#include "net/base/net_log.h"
#include "net/base/completion_callback.h"

namespace base {
class TimeDelta;
}

namespace net {
class IPEndPoint;
class IOBuffer;
class DrainableIOBuffer;
class Socket;
}

namespace sippet {

// This is a wrapper for stream sockets. Data frames are buffered
// locally when the wrapped socket returns asynchronously for Write().
// Each enqueued frame will be notified after write completion.
//
// There are no bounds on the local buffer size. Use carefully.
class ChromeStreamWriter {
 public:
  ChromeStreamWriter(net::Socket* socket_to_wrap);
  virtual ~ChromeStreamWriter();

  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback);

  void CloseWithError(int err);

 private:
  net::Socket* wrapped_socket_;
  int error_;

  struct PendingBlock {
    PendingBlock(net::DrainableIOBuffer* io_buffer,
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

  base::WeakPtrFactory<ChromeStreamWriter> weak_factory_;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_CHROME_STREAM_WRITER_H_
