// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_reader.h"

#include "base/message_loop/message_loop.h"
#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"
#include "net/socket/stream_socket.h"
#include "sippet/message/message.h"

namespace sippet {

// This number will couple with quite long SIP messages
const size_t ChromeStreamReader::kReadBufSize = 64U * 1024U;

ChromeStreamReader::ChromeStreamReader(net::StreamSocket* socket_to_wrap)
    : wrapped_socket_(socket_to_wrap),
      next_state_(STATE_NONE),
      read_buf_(new net::IOBufferWithSize(kReadBufSize)),
      io_callback_(base::Bind(&ChromeStreamReader::OnIOComplete,
          base::Unretained(this))) {
  DCHECK(socket_to_wrap);
  drainable_read_buf_ = new net::DrainableIOBuffer(read_buf_, read_buf_->size());
}

ChromeStreamReader::~ChromeStreamReader() {
}

int ChromeStreamReader::Read(const net::CompletionCallback& callback) {
  DCHECK_EQ(STATE_NONE, next_state_);
  callback_ = callback;
  next_state_ = STATE_BEFORE_RECEIVE_DATA;
  int rv = DoLoop(net::OK);
  if (rv == net::ERR_IO_PENDING)
    callback_ = callback;
  return rv;
}

scoped_refptr<Message> ChromeStreamReader::GetIncomingMessage() {
  scoped_refptr<Message> message(current_message_);
  current_message_ = NULL;
  return message;
}

void ChromeStreamReader::DoCallback(int result) {
  DCHECK_NE(result, net::ERR_IO_PENDING);
  DCHECK(!callback_.is_null());

  // Since Run may result in Read being called, clear user_callback_ up front.
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(result);
}

void ChromeStreamReader::OnIOComplete(int result) {
  int rv = DoLoop(result);
  if (rv != net::ERR_IO_PENDING)
    DoCallback(rv);
}

int ChromeStreamReader::DoLoop(int result) {
  DCHECK(next_state_ != STATE_NONE);

  int rv = result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_BEFORE_RECEIVE_DATA:
        DCHECK_EQ(net::OK, rv);
        rv = DoBeforeReceiveData();
        break;
      case STATE_RECEIVE_DATA:
        DCHECK_EQ(net::OK, rv);
        rv = DoReceiveData();
        break;
      case STATE_RECEIVE_DATA_COMPLETE:
        rv = DoReceiveDataComplete(rv);
        break;
      case STATE_READ_HEADERS:
        DCHECK_EQ(net::OK, rv);
        rv = DoReadHeaders();
        break;
      case STATE_READ_HEADERS_COMPLETE:
        DCHECK_EQ(net::OK, rv);
        rv = DoReadHeadersComplete();
        break;
      case STATE_READ_BODY:
        DCHECK_EQ(net::OK, rv);
        rv = DoReadBody();
        break;
      case STATE_READ_BODY_COMPLETE:
        DCHECK_EQ(net::OK, rv);
        rv = DoReadBodyComplete();
        break;
    }
  } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);

  return rv;
}

int ChromeStreamReader::DoBeforeReceiveData() {
  next_state_ = STATE_RECEIVE_DATA;
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
  return net::OK;
}

int ChromeStreamReader::DoReceiveData() {
  next_state_ = STATE_RECEIVE_DATA_COMPLETE;
  if (read_end_ - drainable_read_buf_->data() > 0) {
    // There are more data to be read, just continue
    return net::OK;
  } else {
    return wrapped_socket_->Read(drainable_read_buf_.get(),
        drainable_read_buf_->size(), io_callback_);
  }
}

int ChromeStreamReader::DoReceiveDataComplete(int result) {
  if (result < 0)
    return result;
  next_state_ = !current_message_.get() ? STATE_READ_HEADERS : STATE_READ_BODY;
  if (result > 0) {
    // Move the end buffer mark accordingly to the number of bytes read.
    read_end_ = drainable_read_buf_->data() + static_cast<size_t>(result);
    if (drainable_read_buf_->data() > read_buf_->data()) {
      // Move the begin buffer to the buffer start.
      drainable_read_buf_->SetOffset(0);
    }
  }
  return net::OK;
}

int ChromeStreamReader::DoReadHeaders() {
  // Eliminate all blanks from the message start. They're used as keep-alive.
  while (drainable_read_buf_->data() != read_end_) {
    switch (drainable_read_buf_->data()[0]) {
      case '\r': case '\n':
        drainable_read_buf_->DidConsume(1);
        continue;
    }
    break;
  }
  if (drainable_read_buf_->data() == read_end_) {
    // The reading buffer was full of empty lines, read more...
    return ReadMore();
  }
  base::StringPiece string_piece(drainable_read_buf_->data(),
      read_end_ - drainable_read_buf_->data());
  size_t end_size = 4;
  size_t end = string_piece.find("\r\n\r\n");
  if (end == base::StringPiece::npos) {
    // CRLF is the standard, but we're accepting just LF
    end = string_piece.find("\n\n");
    end_size = 2;
  }
  if (end == base::StringPiece::npos) {
    // Read more...
    return ReadMore();
  }
  std::string header(drainable_read_buf_->data(), end + end_size);
  drainable_read_buf_->DidConsume(end + end_size);
  current_message_ = Message::Parse(header);
  if (!current_message_) {
    // Close connection: bad protocol
    return net::ERR_INVALID_RESPONSE;
  }
  next_state_ = STATE_READ_HEADERS_COMPLETE;
  return net::OK;
}

int ChromeStreamReader::DoReadHeadersComplete() {
  DCHECK(current_message_);

  // If there's no Content-Length, then we accept as if the content is empty
  ContentLength *content_length = current_message_->get<ContentLength>();
  if (content_length && content_length->value() > 0) {
    if (content_length->value() > kReadBufSize) {
      // Close the connection immediately: the server is trying to send a
      // too large content. Maximum size allowed is 64kb.
      VLOG(1) << "Trying to receive a too large message content: "
              << content_length->value() << ", max = " << kReadBufSize;
      return net::ERR_MSG_TOO_BIG;
    }
    next_state_ = STATE_READ_BODY;
  } else {
    // Jump directly to the final state
    next_state_ = STATE_READ_BODY_COMPLETE;
  }

  return net::OK;
}
    
int ChromeStreamReader::DoReadBody() {
  ContentLength *content_length = current_message_->get<ContentLength>();
  DCHECK(content_length);
  if (content_length->value() >
      static_cast<unsigned>(read_end_ - drainable_read_buf_->data())) {
    // Read more...
    return ReadMore();
  }
  std::string content(drainable_read_buf_->data(),
                      content_length->value());
  drainable_read_buf_->DidConsume(content_length->value());
  current_message_->set_content(content);
  next_state_ = STATE_READ_BODY_COMPLETE;
  return net::OK;
}

int ChromeStreamReader::DoReadBodyComplete() {
  // This will make the callback to be called
  next_state_ = STATE_NONE;
  return net::OK;
}

int ChromeStreamReader::ReadMore() {
  if (read_end_ == read_buf_->data() + read_buf_->size()) {
    // Close the connection: the server is trying to send a message (header
    // or content) that exceeds the maximum size allowed (64kb).
    return net::ERR_MSG_TOO_BIG;
  }
  next_state_ = STATE_BEFORE_RECEIVE_DATA;
  return net::OK;
}

} // namespace sippet

