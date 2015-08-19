// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_datagram_channel.h"

#include "base/rand_util.h"
#include "net/base/net_errors.h"
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
    next_state_(STATE_NONE) {
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

int ChromeDatagramChannel::ReconnectIgnoringLastError() {
  // If we're going to support DTLS, this will need to be changed
  VLOG(1) << "Trying to reconnect a raw UDP channel";
  return net::ERR_UNEXPECTED;
}

int ChromeDatagramChannel::ReconnectWithCertificate(
    net::X509Certificate* client_cert) {
  // If we're going to support DTLS, this will need to be changed
  VLOG(1) << "Trying to add certificate to a raw UDP channel";
  return net::ERR_ADD_USER_CERT_FAILED;
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
        datagram_reader_.reset(new ChromeDatagramReader(socket.get()));
        datagram_writer_.reset(new ChromeDatagramWriter(socket.get()));
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
  if (is_connected_ && datagram_writer_.get()) {
    scoped_refptr<net::StringIOBuffer> string_buffer =
        new net::StringIOBuffer(message->ToString());
    return datagram_writer_->Write(
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
  if (datagram_writer_.get()) {
    datagram_writer_->CloseWithError(err);
  }
}

void ChromeDatagramChannel::DetachDelegate() {
  delegate_ = nullptr;
}

void ChromeDatagramChannel::CloseTransportSocket() {
  if (socket_.get())
    socket_->Close();
  socket_.reset();
  datagram_reader_.reset();
  datagram_writer_.reset();
  is_connected_ = false;
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void ChromeDatagramChannel::PostDoRead() {
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeDatagramChannel::DoRead,
                 weak_ptr_factory_.GetWeakPtr()));
}

void ChromeDatagramChannel::DoRead() {
  DCHECK(datagram_reader_.get());
  int result = datagram_reader_->Read(
      base::Bind(&ChromeDatagramChannel::OnReadComplete,
                 weak_ptr_factory_.GetWeakPtr()));
  if (net::ERR_IO_PENDING == result)
    return;
  OnReadComplete(result);
}

void ChromeDatagramChannel::OnReadComplete(int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);
  if (net::OK == result) {
    PostDoRead();
    delegate_->OnIncomingMessage(this, datagram_reader_->GetIncomingMessage());
  } else if (result < 0) {
    RunUserChannelClosed(result);
    // |this| may be deleted after this call.
  }
}

} // namespace sippet
