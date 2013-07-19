// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/transport_test_util.h"

namespace sippet {

MockEvent::OnChannelClosedImpl::OnChannelClosedImpl(
    const EndPoint &destination)
  : has_error_(false), destination_(destination) {}

MockEvent::OnChannelClosedImpl::OnChannelClosedImpl(
    const EndPoint &destination, int error)
  : has_error_(true), error_(error), destination_(destination) {}

void MockEvent::OnChannelClosedImpl::OnChannelClosed(
    const EndPoint& destination, int error) {
  EXPECT_EQ(destination_, destination);
  if (has_error_)
    EXPECT_EQ(error_, error);
}

void MockEvent::OnChannelClosedImpl::OnIncomingMessage(Message *message) {
  EXPECT_TRUE(false) << "Not expected to get an incoming message this time";
}

MockEvent::OnIncomingMessageImpl::OnIncomingMessageImpl(
    const char *regular_expressions)
  : regular_expressions_(regular_expressions) {}

void MockEvent::OnIncomingMessageImpl::OnChannelClosed(
    const EndPoint& destination, int error) {
  EXPECT_TRUE(false) << "Not expected to get a network error this time";
}

void MockEvent::OnIncomingMessageImpl::OnIncomingMessage(Message *message) {
  // TODO: break down the regular expressions and check the message
}

MockEvent::MockEvent(const MockEvent &other)
  : delegate_(other.delegate_), time_stamp_(other.time_stamp_) {}

MockEvent::~MockEvent() {}

MockEvent& MockEvent::operator=(const MockEvent &other) {
  delegate_ = other.delegate_;
  time_stamp_ = other.time_stamp_;
  return *this;
}

MockEvent MockEvent::ChannelClosed(const EndPoint &destination) {
  return MockEvent(new MockEvent::OnChannelClosedImpl(destination));
}

MockEvent MockEvent::ChannelClosed(const EndPoint &destination, int error) {
  return MockEvent(new MockEvent::OnChannelClosedImpl(destination, error));
}

MockEvent MockEvent::ChannelClosed(const char *destination) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new MockEvent::OnChannelClosedImpl(endpoint));
}

MockEvent MockEvent::ChannelClosed(const char *destination, int error) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new MockEvent::OnChannelClosedImpl(endpoint, error));
}

MockEvent MockEvent::IncomingMessage(const char *regular_expressions) {
  return MockEvent(new MockEvent::OnIncomingMessageImpl(regular_expressions));
}

void MockEvent::OnChannelClosed(const EndPoint& destination, int error) {
  delegate_->OnChannelClosed(destination, error);
}

void MockEvent::OnIncomingMessage(Message *message) {
  delegate_->OnIncomingMessage(message);
}

MockEvent::MockEvent(NetworkLayer::Delegate *delegate)
  : delegate_(delegate) {}

StaticNetworkLayerDelegate::StaticNetworkLayerDelegate()
  : events_(NULL), events_count_(0), events_index_(0) {}

StaticNetworkLayerDelegate::StaticNetworkLayerDelegate(
    MockEvent* events, size_t events_count)
  : events_(events), events_count_(events_count), events_index_(0) {}

StaticNetworkLayerDelegate::~StaticNetworkLayerDelegate() {}

const MockEvent& StaticNetworkLayerDelegate::PeekEvent() const {
  return PeekEvent(events_index_);
}

const MockEvent& StaticNetworkLayerDelegate::PeekEvent(size_t index) const {
  DCHECK_LT(index, events_count_);
  return events_[index];
}

MockEvent StaticNetworkLayerDelegate::GetNextEvent() {
  DCHECK(!at_events_end());
  events_[events_index_].set_time_stamp(base::Time::Now());
  return events_[events_index_++];
}

void StaticNetworkLayerDelegate::OnChannelClosed(const EndPoint& destination, int error) {
  DCHECK(!at_events_end());
  GetNextEvent().OnChannelClosed(destination, error);
}

void StaticNetworkLayerDelegate::OnIncomingMessage(Message *message) {
  DCHECK(!at_events_end());
  GetNextEvent().OnIncomingMessage(message);
}

UDPChannelAdapter::UDPChannelAdapter(net::MockClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {}

UDPChannelAdapter::~UDPChannelAdapter() {}

int UDPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  // TODO
  return net::OK;
}

int UDPChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Read(buf, buf_len, callback);
}

int UDPChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Write(buf, buf_len, callback);
}

TCPChannelAdapter::TCPChannelAdapter(net::MockClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {}

TCPChannelAdapter::~TCPChannelAdapter() {}

int TCPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  // TODO
  return net::OK;
}

int TCPChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Read(buf, buf_len, callback);
}

int TCPChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Write(buf, buf_len, callback);
}

TLSChannelAdapter::TLSChannelAdapter(net::MockClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {}

TLSChannelAdapter::~TLSChannelAdapter() {}

int TLSChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  // TODO
  return net::OK;
}

int TLSChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Read(buf, buf_len, callback);
}

int TLSChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  return socket_->Write(buf, buf_len, callback);
}

MockChannel::MockChannel(MockChannelAdapter *channel_adapter,
                         Channel::Delegate *delegate,
                         const EndPoint &destination)
    : channel_adapter_(channel_adapter),
      delegate_(delegate),
      destination_(destination),
      origin_(net::HostPortPair("192.0.2.33", 123),
              destination.protocol()) {}

MockChannel::~MockChannel() {}

const EndPoint& MockChannel::origin() const {
  return origin_;
}

const EndPoint& MockChannel::destination() const {
  return destination_;
}

bool MockChannel::is_secure() const {
  return channel_adapter_->is_secure();
}

bool MockChannel::is_connected() const {
  return is_connected_;
}

void MockChannel::Connect() {
  DCHECK(!is_connected());
  channel_adapter_->Connect(destination_.hostport(),
    base::Bind(&MockChannel::OnConnected, this));
}

int MockChannel::Send(const scoped_refptr<Message>& message,
                      const net::CompletionCallback& callback) {
  DCHECK(is_connected());
  std::string buffer(message->ToString());
  scoped_refptr<net::IOBuffer> io_buffer(new net::IOBuffer(buffer.size()));
  memcpy(io_buffer->data(), buffer.data(), buffer.size());
  return channel_adapter_->Write(io_buffer, buffer.size(), callback);
}

void MockChannel::Close() {
  DCHECK(is_connected());
}

void MockChannel::CloseWithError(int error) {
  DCHECK(is_connected());
}

void MockChannel::DetachDelegate() {
  delegate_ = 0;
}

void MockChannel::OnConnected(int result) {
  is_connected_ = true;
  if (delegate_)
    delegate_->OnChannelConnected(this, result);
  // TODO: start reading
}

MockChannelFactory::MockChannelFactory(
    net::MockClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {
  DCHECK(socket_factory);
}

MockChannelFactory::~MockChannelFactory() {}

int MockChannelFactory::CreateChannel(const EndPoint &destination,
                                      Channel::Delegate *delegate,
                                      scoped_refptr<Channel> *channel) {
  if (destination.protocol() == Protocol::UDP) {
    *channel = new MockChannel(new UDPChannelAdapter(socket_factory_),
                               delegate, destination);
    return net::OK;
  }
  else if (destination.protocol() == Protocol::TCP) {
    *channel = new MockChannel(new TCPChannelAdapter(socket_factory_),
                               delegate, destination);
    return net::OK;
  }
  else if (destination.protocol() == Protocol::TLS) {
    *channel = new MockChannel(new TLSChannelAdapter(socket_factory_),
                               delegate, destination);
    return net::OK;
  }
  NOTREACHED();
  return net::ERR_UNEXPECTED;
}

ClientTransaction *TransactionFactoryImpl::CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
  // TODO
  return 0;
}

ServerTransaction *TransactionFactoryImpl::CreateServerTransaction(
    const Method &method,
    const std::string &transaction_id,
    const scoped_refptr<Channel> &channel,
    TransactionDelegate *delegate) {
  // TODO
  return 0;
}

} // End of sippet namespace
