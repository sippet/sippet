// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_datagram_reader.h"

#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"
#include "net/socket/socket.h"

namespace sippet {

// This number will couple with quite long SIP messages
static const size_t kReadBufSize = 64U * 1024U;

ChromeDatagramReader::ChromeDatagramReader(
    net::Socket* socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      read_buf_(new net::IOBufferWithSize(kReadBufSize)),
      read_complete_(base::Bind(&ChromeDatagramReader::OnReceiveDataComplete,
          base::Unretained(this))) {
  DCHECK(socket_to_wrap);
  read_start_ = read_end_ = read_buf_->data();
}

ChromeDatagramReader::~ChromeDatagramReader() {
}

int ChromeDatagramReader::DoIORead(
    const net::CompletionCallback& callback) {
  if (HasMessage()) {
    VLOG(1) << "Discarded incoming datagram: truncated body";
    return net::ERR_INVALID_RESPONSE;
  } else if (BytesRemaining() > 0 && read_start_ == read_buf_->data()) {
    VLOG(1) << "Discarded incoming datagram: truncated header";
    return net::ERR_INVALID_RESPONSE;
  }
  int result = wrapped_socket_->Read(read_buf_.get(), read_buf_->size(),
      read_complete_);
  if (result > net::OK) {
    ReceivedData(result);
    return net::OK;
  } else if (result == net::OK) {
    return net::ERR_CONNECTION_CLOSED; // ICMP errors
  } else if (net::ERR_IO_PENDING == result) {
    callback_ = callback;
  }
  return result;
}

void ChromeDatagramReader::OnReceiveDataComplete(int result) {
  DCHECK_NE(result, net::ERR_IO_PENDING);
  if (result < net::OK) {
    DoCallback(result);
  } else if (result == net::OK) {
    DoCallback(net::ERR_CONNECTION_CLOSED);
  } else {
    ReceivedData(result);
    DoCallback(net::OK);
  }
}

void ChromeDatagramReader::ReceivedData(size_t bytes) {
  DCHECK_GT(bytes, 0U);
  read_start_ = read_buf_->data();
  read_end_ = read_buf_->data() + bytes;
}

void ChromeDatagramReader::DoCallback(int result) {
  DCHECK_NE(result, net::ERR_IO_PENDING);
  DCHECK(!callback_.is_null());
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(result);
}

char *ChromeDatagramReader::data() {
  return read_start_;
}

size_t ChromeDatagramReader::max_size() {
  return kReadBufSize;
}

int ChromeDatagramReader::BytesRemaining() const {
  return read_end_ - read_start_;
}

void ChromeDatagramReader::DidConsume(int bytes) {
  DCHECK_LE(bytes, BytesRemaining());
  read_start_ += bytes;
}

} // namespace sippet

