// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_

#include "base/memory/scoped_ptr.h"
#include "net/base/completion_callback.h"

namespace net {
class StreamSocket;
class IOBufferWithSize;
class DrainableIOBuffer;
}

namespace sippet {

class Message;

class ChromeStreamReader {
 public:
  ChromeStreamReader(net::StreamSocket* socket_to_wrap);
  virtual ~ChromeStreamReader();

  int Read(const net::CompletionCallback& callback);
  scoped_refptr<Message> GetIncomingMessage();

  bool is_idle() const {
    return next_state_ == STATE_NONE;
  }

  static const size_t kReadBufSize;

 private:
  enum State {
    STATE_NONE,
    STATE_BEFORE_RECEIVE_DATA,
    STATE_RECEIVE_DATA,
    STATE_RECEIVE_DATA_COMPLETE,
    STATE_READ_HEADERS,
    STATE_READ_HEADERS_COMPLETE,
    STATE_READ_BODY,
    STATE_READ_BODY_COMPLETE,
  };

  void DoCallback(int result);
  void OnIOComplete(int result);

  // Runs the state transition loop.
  int DoLoop(int result);

  // Read loop functions.
  int DoBeforeReceiveData();
  int DoReceiveData();
  int DoReceiveDataComplete(int result);
  int DoReadHeaders();
  int DoReadHeadersComplete();
  int DoReadBody();
  int DoReadBodyComplete();
  int ReadMore();

  net::StreamSocket* wrapped_socket_;

  State next_state_;
  scoped_refptr<net::IOBufferWithSize> read_buf_;
  scoped_refptr<net::DrainableIOBuffer> drainable_read_buf_;
  char *read_end_;
  scoped_refptr<Message> current_message_;
  net::CompletionCallback callback_;
  net::CompletionCallback io_callback_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStreamReader);
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_
