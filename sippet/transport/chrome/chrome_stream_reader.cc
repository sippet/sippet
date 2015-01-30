// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_reader.h"

#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"
#include "net/socket/socket.h"

namespace sippet {

// This number will couple with quite long SIP messages
static const size_t kReadBufSize = 64U * 1024U;

ChromeStreamReader::ChromeStreamReader(net::Socket* socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      read_buf_(new net::IOBufferWithSize(kReadBufSize)),
      read_complete_(base::Bind(&ChromeStreamReader::ReceiveDataComplete,
          base::Unretained(this))) {
  DCHECK(socket_to_wrap);
  drainable_read_buf_ = new net::DrainableIOBuffer(read_buf_.get(), read_buf_->size());
  read_end_ = drainable_read_buf_->data();
}

ChromeStreamReader::~ChromeStreamReader() {
}

int ChromeStreamReader::DoIORead(
    const net::CompletionCallback& callback) {
  if (read_end_ == read_buf_->data() + read_buf_->size()) {
    // Close the connection: the server is trying to send a message (header
    // or content) that exceeds the maximum size allowed (64kb).
    return net::ERR_MSG_TOO_BIG;
  }
  if (read_end_ - drainable_read_buf_->data() > 0) {
    // Rearrange the still pending data to the beginning of the buffer.
    size_t pending_bytes = read_end_ - drainable_read_buf_->data();
    memmove(read_buf_->data(), drainable_read_buf_->data(), pending_bytes);

    // Move the reading buffer after the pending bytes
    drainable_read_buf_->SetOffset(pending_bytes);
  } else {
    // The next read will take a clean buffer
    drainable_read_buf_->SetOffset(0);
  }
  read_end_ = drainable_read_buf_->data();
  if (read_end_ - drainable_read_buf_->data() > 0) {
    // There are more data to be read, just continue
    return net::OK;
  }
  int result = wrapped_socket_->Read(drainable_read_buf_.get(),
      drainable_read_buf_->size(), read_complete_);
  if (net::ERR_IO_PENDING == result)
    callback_ = callback;
  return result;
}

void ChromeStreamReader::ReceiveDataComplete(int result) {
  if (result < 0) {
    DoCallback(result);
  } else if (result == 0) {
    DoCallback(net::ERR_CONNECTION_CLOSED);
  } else {
    // Move the end buffer mark accordingly to the number of bytes read.
    read_end_ = drainable_read_buf_->data() + static_cast<size_t>(result);
    if (drainable_read_buf_->data() > read_buf_->data()) {
      // Move the begin buffer to the buffer start.
      drainable_read_buf_->SetOffset(0);
    }
    DoCallback(net::OK);
  }
}

void ChromeStreamReader::DoCallback(int result) {
  DCHECK_NE(result, net::ERR_IO_PENDING);
  DCHECK(!callback_.is_null());
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(result);
}

char *ChromeStreamReader::data() {
  return drainable_read_buf_->data();
}

size_t ChromeStreamReader::max_size() {
  return kReadBufSize;
}

int ChromeStreamReader::BytesRemaining() const {
  return read_end_ - drainable_read_buf_->data();
}

void ChromeStreamReader::DidConsume(int bytes) {
  drainable_read_buf_->DidConsume(bytes);
}

} // namespace sippet

