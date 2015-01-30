// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_READER_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_READER_H_

#include "sippet/transport/chrome/message_reader.h"

namespace net {
class Socket;
class IOBufferWithSize;
}

namespace sippet {

class Message;

class ChromeDatagramReader
  : public MessageReader {
 public:
  ChromeDatagramReader(net::Socket* socket_to_wrap);
  virtual ~ChromeDatagramReader();

 private:
  virtual int DoIORead(const net::CompletionCallback& callback) override;
  virtual char *data() override;
  virtual size_t max_size() override;
  virtual int BytesRemaining() const override;
  virtual void DidConsume(int bytes) override;

  void OnReceiveDataComplete(int result);
  void ReceivedData(size_t bytes);
  void DoCallback(int result);

  net::Socket* wrapped_socket_;
  scoped_refptr<net::IOBufferWithSize> read_buf_;
  net::CompletionCallback callback_;
  net::CompletionCallback read_complete_;
  char *read_start_;
  char *read_end_;

  DISALLOW_COPY_AND_ASSIGN(ChromeDatagramReader);
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_READER_H_
