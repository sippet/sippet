// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_

#include "sippet/transport/chrome/message_reader.h"

namespace net {
class Socket;
class IOBufferWithSize;
class DrainableIOBuffer;
}

namespace sippet {

class Message;

class ChromeStreamReader
  : public MessageReader {
 public:
  ChromeStreamReader(net::Socket* socket_to_wrap);
  virtual ~ChromeStreamReader();

 private:
  virtual int DoIORead(const net::CompletionCallback& callback) OVERRIDE;
  virtual char *data() OVERRIDE;
  virtual size_t max_size() OVERRIDE;
  virtual int BytesRemaining() const OVERRIDE;
  virtual void DidConsume(int bytes) OVERRIDE;

  void ReceiveDataComplete(int result);
  void DoCallback(int result);

  net::Socket* wrapped_socket_;
  scoped_refptr<net::IOBufferWithSize> read_buf_;
  scoped_refptr<net::DrainableIOBuffer> drainable_read_buf_;
  net::CompletionCallback callback_;
  net::CompletionCallback read_complete_;
  char *read_end_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStreamReader);
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_CHROME_CHROME_STREAM_READER_H_
