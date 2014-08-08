// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_MESSAGE_READER_H_
#define SIPPET_TRANSPORT_CHROME_MESSAGE_READER_H_

#include "base/memory/scoped_ptr.h"
#include "net/base/completion_callback.h"

namespace sippet {

class Message;

class MessageReader {
 public:
  MessageReader();
  virtual ~MessageReader();

  int Read(const net::CompletionCallback& callback);
  scoped_refptr<Message> GetIncomingMessage();

  bool is_idle() const {
    return next_state_ == STATE_NONE;
  }

  bool HasMessage() {
    return current_message_.get();
  }

 protected:
  // Perform the underlying I/O operation.
  virtual int DoIORead(const net::CompletionCallback& callback) = 0;

  // Points to the first unconsumed byte.
  virtual char *data() = 0;

  // Returns the max size of the internal read buffer.
  virtual size_t max_size() = 0;

  // Returns the number of unconsumed bytes.
  virtual int BytesRemaining() const = 0;

  // DidConsume() changes the |data| pointer so that |data| always points
  // to the first unconsumed byte.
  virtual void DidConsume(int bytes) = 0;

 private:
  enum State {
    STATE_NONE,
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

  State next_state_;
  scoped_refptr<Message> current_message_;
  net::CompletionCallback callback_;
  net::CompletionCallback io_callback_;

  DISALLOW_COPY_AND_ASSIGN(MessageReader);
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_CHROME_MESSAGE_READER_H_
