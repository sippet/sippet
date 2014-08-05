// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_datagram_channel.h"

#include "base/rand_util.h"
#include "net/base/net_errors.h"
#include "net/base/net_log.h"
#include "net/base/ip_endpoint.h"
#include "net/socket/client_socket_handle.h"
#include "net/socket/client_socket_factory.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/udp/datagram_client_socket.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "sippet/message/message.h"

namespace sippet {

// This is actually the theoretical max MTU of an UDP packet. The size without
// fragmentation is actually around 1500 bytes.
const size_t kReadBufSize = 64U * 1024U;

ChromeDatagramChannel::ChromeDatagramChannel(const EndPoint& destination,
      Channel::Delegate *delegate,
      net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter)
  : destination_(destination),
    delegate_(delegate),
    client_socket_factory_(client_socket_factory),
    weak_ptr_factory_(this),
    is_connected_(false),
    host_resolver_(
        request_context_getter->GetURLRequestContext()->host_resolver()),
    bound_net_log_(
        net::BoundNetLog::Make(
            request_context_getter->GetURLRequestContext()->net_log(),
            net::NetLog::SOURCE_SOCKET)),
    read_state_(READ_IDLE),
    next_state_(STATE_NONE),
    read_buf_(new net::IOBufferWithSize(kReadBufSize)) {
  DCHECK(!destination_.IsEmpty());
  DCHECK(client_socket_factory_);
  DCHECK(delegate_);
}

ChromeDatagramChannel::~ChromeDatagramChannel() {
}

int ChromeDatagramChannel::origin(EndPoint *origin) const {
  if (STATE_NONE == next_state_ && socket_.get()) {
    net::IPEndPoint ip_endpoint;
    int rv = socket_->GetLocalAddress(&ip_endpoint);
    if (rv != net::OK)
      return rv;
    *origin = EndPoint(net::HostPortPair::FromIPEndPoint(ip_endpoint),
        Protocol::UDP);
    return net::OK;
  }
  NOTREACHED() << "not connected";
  return net::ERR_SOCKET_NOT_CONNECTED;
}

const EndPoint& ChromeDatagramChannel::destination() const {
  return destination_;
}

bool ChromeDatagramChannel::is_secure() const {
  return false;
}

bool ChromeDatagramChannel::is_connected() const {
  return is_connected_;
}

bool ChromeDatagramChannel::is_stream() const {
  return false;
}

void ChromeDatagramChannel::Connect() {
  DCHECK(!socket_.get());
  DCHECK_EQ(STATE_NONE, next_state_);

  next_state_ = STATE_RESOLVE_HOST;
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeDatagramChannel::OnIOComplete,
                  weak_ptr_factory_.GetWeakPtr(), net::OK));
}

void ChromeDatagramChannel::RunUserConnectCallback(int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);
  if (net::OK == result)
    PostDoRead();
  if (delegate_)
    delegate_->OnChannelConnected(this, result);
}

void ChromeDatagramChannel::RunUserChannelClosed(int status) {
  DCHECK_LE(status, net::OK);
  is_connected_ = false;
  if (delegate_)
    delegate_->OnChannelClosed(this, status);
}

void ChromeDatagramChannel::OnIOComplete(int result) {
  DCHECK_NE(STATE_NONE, next_state_);
  int rv = DoLoop(result);
  if (rv != net::ERR_IO_PENDING)
    RunUserConnectCallback(rv);
}

int ChromeDatagramChannel::DoLoop(int last_io_result) {
  DCHECK_NE(next_state_, STATE_NONE);
  int rv = last_io_result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_RESOLVE_HOST:
        DCHECK_EQ(net::OK, rv);
        rv = DoResolveHost();
        break;
      case STATE_RESOLVE_HOST_COMPLETE:
        rv = DoResolveHostComplete(rv);
        break;
      default:
        NOTREACHED() << "bad state";
        rv = net::ERR_UNEXPECTED;
        break;
    }
  } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);
  return rv;
}

int ChromeDatagramChannel::DoResolveHost() {
  next_state_ = STATE_RESOLVE_HOST_COMPLETE;
  net::HostResolver::RequestInfo host_request_info(destination_.hostport());
  return host_resolver_.Resolve(
      host_request_info,
      net::DEFAULT_PRIORITY,
      &addresses_,
      base::Bind(&ChromeDatagramChannel::OnIOComplete, base::Unretained(this)),
      bound_net_log_);
}

int ChromeDatagramChannel::DoResolveHostComplete(int result) {
  DCHECK(!socket_.get());

  if (net::OK != result)
    return result;

  net::NetLog::Source no_source;
  scoped_ptr<net::DatagramClientSocket> socket =
      client_socket_factory_->CreateDatagramClientSocket(
          net::DatagramSocket::RANDOM_BIND, base::Bind(&base::RandInt),
          bound_net_log_.net_log(), no_source);

  int rv;
  if (!socket.get()) {
    LOG(WARNING) << "Failed to create socket.";
    rv = net::ERR_UNEXPECTED;
  } else {
    for (net::AddressList::iterator i = addresses_.begin(),
         ie = addresses_.end(); i != ie; i++) {
      rv = socket->Connect(*i);
      if (rv == net::OK) {
        is_connected_ = true;
        datagram_socket_.reset(new FramedWriteStreamSocket(socket.get()));
        break;
      }
    }
    if (rv != net::OK) {
      VLOG(1) << "Failed to connect socket: " << rv;
      socket.reset();
    }
  }

  socket_.swap(socket);
  return rv;
}

int ChromeDatagramChannel::Send(const scoped_refptr<Message> &message,
                                const net::CompletionCallback& callback) {
  if (is_connected_ && datagram_socket_.get()) {
    scoped_refptr<net::StringIOBuffer> string_buffer =
        new net::StringIOBuffer(message->ToString());
    return datagram_socket_->Write(
        string_buffer.get(),
        string_buffer->size(),
        callback);
  }
  NOTREACHED();
  return net::ERR_SOCKET_NOT_CONNECTED;
}

void ChromeDatagramChannel::Close() {
  CloseTransportSocket();
}

void ChromeDatagramChannel::CloseWithError(int err) {
  if (datagram_socket_.get()) {
    datagram_socket_->CloseWithError(err);
  }
}

void ChromeDatagramChannel::DetachDelegate() {
  delegate_ = NULL;
}

void ChromeDatagramChannel::CloseTransportSocket() {
  if (socket_.get())
    socket_->Close();
  socket_.reset();
  datagram_socket_.reset();
  is_connected_ = false;
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void ChromeDatagramChannel::PostDoRead() {
  DCHECK(is_connected());
  DCHECK_EQ(read_state_, READ_IDLE);
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeDatagramChannel::DoRead,
                 weak_ptr_factory_.GetWeakPtr()));
  read_state_ = READ_POSTED;
}

void ChromeDatagramChannel::DoRead() {
  DCHECK(is_connected());
  DCHECK_EQ(read_state_, READ_POSTED);

  int status =
      socket_->Read(
          read_buf_.get(), read_buf_->size(),
          base::Bind(&ChromeDatagramChannel::ProcessReadDone,
                     weak_ptr_factory_.GetWeakPtr()));
  read_state_ = READ_PENDING;
  if (status != net::ERR_IO_PENDING) {
    ProcessReadDone(status);
  }
}

void ChromeDatagramChannel::ProcessReadDone(int status) {
  DCHECK_NE(status, net::ERR_IO_PENDING);
  DCHECK_EQ(read_state_, READ_PENDING);

  read_state_ = READ_IDLE;
  if (status > 0) {
    int rv = ProcessReceivedData(status);
    if (rv != net::OK) {
      RunUserChannelClosed(rv);
    } else {
      PostDoRead();
    }
  } else if (status == 0) {
    // Other side closed the connection.
    RunUserChannelClosed(net::OK);
  } else {  // status < 0
    RunUserChannelClosed(status);
  }
}

int ChromeDatagramChannel::ProcessReceivedData(int read_bytes) {
  DCHECK(is_connected());
  DCHECK_EQ(read_state_, READ_IDLE);

  // The datagram processing is simpler: interpret just one
  // frame and discard any extra data 
  char *read_start = read_buf_->data();
  char *read_end = read_buf_->data() + read_bytes;

  // Eliminate all blanks from the message start.
  while (read_start != read_end) {
    switch (read_start[0]) {
      case '\r': case '\n':
        read_start += 1;
        continue;
    }
    break;
  }
  if (read_start == read_end) {
    // It was just a keep-alive, ignore.
    return net::OK;
  }

  base::StringPiece string_piece(read_start, read_end - read_start);
  size_t end_size = 4;
  size_t end = string_piece.find("\r\n\r\n");
  if (end == base::StringPiece::npos) {
    // CRLF is the standard, but we're accepting just LF
    end = string_piece.find("\n\n");
    end_size = 2;
  }
  if (end == base::StringPiece::npos) {
    // End of message wasn't found, discarding...
    VLOG(1) << "Discarded incoming datagram: end of SIP header not found";
    return net::ERR_INVALID_RESPONSE;
  }
  std::string header(read_start, end + end_size);
  read_start += end + end_size;
  scoped_refptr<Message> message = Message::Parse(header);
  if (!message) {
    // Close connection: bad protocol
    return net::ERR_INVALID_RESPONSE;
  }

  // If there's no Content-Length, then we accept as if the content is empty
  ContentLength *content_length = message->get<ContentLength>();
  if (content_length && content_length->value() > 0) {
    if (content_length->value() > kReadBufSize) {
      // Close the connection immediately: the server is trying to send a
      // too large content. Maximum size allowed is 64kb.
      VLOG(1) << "Trying to receive a too large message content: "
              << content_length->value() << ", max = " << kReadBufSize;
      return net::ERR_MSG_TOO_BIG;
    }
    if (content_length->value() >
        static_cast<unsigned>(read_end - read_start)) {
      return net::ERR_MSG_TOO_BIG;
    }
    std::string content(read_start, content_length->value());
    message->set_content(content);
  }
    
  if (delegate_)
    delegate_->OnIncomingMessage(this, message);
  return net::OK;
}

} // namespace sippet
