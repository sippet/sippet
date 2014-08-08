// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/message_reader.h"

#include "base/message_loop/message_loop.h"
#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"
#include "net/socket/stream_socket.h"
#include "sippet/message/message.h"

namespace sippet {

MessageReader::MessageReader()
    : next_state_(STATE_NONE),
      io_callback_(base::Bind(&MessageReader::OnIOComplete,
          base::Unretained(this))) {
}

MessageReader::~MessageReader() {
}

int MessageReader::Read(const net::CompletionCallback& callback) {
  DCHECK_EQ(STATE_NONE, next_state_);
  callback_ = callback;
  next_state_ = STATE_RECEIVE_DATA;
  int rv = DoLoop(net::OK);
  if (rv == net::ERR_IO_PENDING)
    callback_ = callback;
  return rv;
}

scoped_refptr<Message> MessageReader::GetIncomingMessage() {
  scoped_refptr<Message> message(current_message_);
  current_message_ = NULL;
  return message;
}

void MessageReader::DoCallback(int result) {
  DCHECK_NE(result, net::ERR_IO_PENDING);
  DCHECK(!callback_.is_null());

  // Since Run may result in Read being called, clear user_callback_ up front.
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(result);
}

void MessageReader::OnIOComplete(int result) {
  int rv = DoLoop(result);
  if (rv != net::ERR_IO_PENDING)
    DoCallback(rv);
}

int MessageReader::DoLoop(int result) {
  DCHECK(next_state_ != STATE_NONE);

  int rv = result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
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

int MessageReader::DoReceiveData() {
  next_state_ = STATE_RECEIVE_DATA_COMPLETE;
  if (BytesRemaining() > 0) {
    // There are more data to be read, just continue
    return net::OK;
  } else {
    return DoIORead(io_callback_);
  }
}

int MessageReader::DoReceiveDataComplete(int result) {
  if (result < 0)
    return result;
  next_state_ = !current_message_.get() ? STATE_READ_HEADERS : STATE_READ_BODY;
  return net::OK;
}

int MessageReader::DoReadHeaders() {
  // Eliminate all blanks from the message start. They're used as keep-alive.
  while (BytesRemaining() > 0) {
    switch (data()[0]) {
      case '\r': case '\n':
        DidConsume(1);
        continue;
    }
    break;
  }
  if (BytesRemaining() == 0) {
    // The reading buffer was full of empty lines, read more...
    return ReadMore();
  }
  base::StringPiece string_piece(data(), BytesRemaining());
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
  std::string header(data(), end + end_size);
  DidConsume(end + end_size);
  current_message_ = Message::Parse(header);
  if (!current_message_) {
    // Close connection: bad protocol
    return net::ERR_INVALID_RESPONSE; // XXX: what if it's a request?
  }
  next_state_ = STATE_READ_HEADERS_COMPLETE;
  return net::OK;
}

int MessageReader::DoReadHeadersComplete() {
  DCHECK(current_message_);

  // If there's no Content-Length, then we accept as if the content is empty
  ContentLength *content_length = current_message_->get<ContentLength>();
  if (content_length && content_length->value() > 0) {
    if (content_length->value() > max_size()) {
      // Close the connection immediately: the server is trying to send a
      // too large content. Maximum size allowed is 64kb.
      VLOG(1) << "Trying to receive a too large message content: "
              << content_length->value()
              << ", max = " << max_size();
      return net::ERR_MSG_TOO_BIG;
    }
    next_state_ = STATE_READ_BODY;
  } else {
    // Jump directly to the final state
    next_state_ = STATE_READ_BODY_COMPLETE;
  }

  return net::OK;
}
    
int MessageReader::DoReadBody() {
  ContentLength *content_length = current_message_->get<ContentLength>();
  DCHECK(content_length);
  if (content_length->value() > static_cast<unsigned>(BytesRemaining())) {
    // Read more...
    return ReadMore();
  }
  std::string content(data(), content_length->value());
  DidConsume(content_length->value());
  current_message_->set_content(content);
  next_state_ = STATE_READ_BODY_COMPLETE;
  return net::OK;
}

int MessageReader::DoReadBodyComplete() {
  // This will make the callback to be called
  next_state_ = STATE_NONE;
  return net::OK;
}

int MessageReader::ReadMore() {
  if (BytesRemaining() == max_size()) {
    // Close the connection: the server is trying to send a message (header
    // or content) that exceeds the maximum size allowed.
    return net::ERR_MSG_TOO_BIG;
  }
  next_state_ = STATE_RECEIVE_DATA;
  return net::OK;
}

} // namespace sippet

