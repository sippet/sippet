// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_FRAMED_WRITE_STREAM_SOCKET_H_
#define SIPPET_TRANSPORT_CHROME_FRAMED_WRITE_STREAM_SOCKET_H_

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
class Socket;
}

namespace sippet {

// This is a wrapper for datagram sockets. Data frames are buffered
// locally when the wrapped socket returns asynchronously for Write().
// But messages will be truncated instead of cutting them down in frame
// boundaries.
//
// There are no bounds on the local buffer size. Use carefully.
class ChromeDatagramWriter {
 public:
  ChromeDatagramWriter(net::Socket* socket_to_wrap);
  virtual ~ChromeDatagramWriter();

  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback);

  void CloseWithError(int err);

 private:
  net::Socket* wrapped_socket_;
  base::WeakPtrFactory<ChromeDatagramWriter> weak_factory_;
  int error_;

  struct PendingFrame {
    PendingFrame(net::IOBuffer* buf, int buf_len,
                 const net::CompletionCallback& callback);
    ~PendingFrame();
    scoped_refptr<net::IOBuffer> buf_;
    int buf_len_;
    net::CompletionCallback callback_;
  };

  std::deque<PendingFrame*> pending_messages_;

  void DidWrite(int result);
  void DidConsume();
  void Pop(int result);
  int Drain(net::IOBuffer* buf, int buf_len);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_FRAMED_WRITE_STREAM_SOCKET_H_
