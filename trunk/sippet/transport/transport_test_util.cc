// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/transport_test_util.h"

#include "base/string_util.h"
#include "base/rand_util.h"

#include "third_party/icu/public/i18n/unicode/regex.h"

namespace sippet {

namespace {

const net::SSLConfig kDefaultSSLConfig;

bool ParseHostPortPair(const net::HostPortPair &destination,
                       net::AddressList *addrlist) {
  std::string host;
  int port;
  std::string destination_string(destination.ToString());
  if (net::ParseHostAndPort(destination_string, &host, &port)) {
    LOG(WARNING) << "Not a supported IP literal: " << destination_string;
    return false;
  }
  addrlist->set_canonical_name(host);
  net::IPAddressNumber ip_number;
  if (!net::ParseIPLiteralToNumber(host, &ip_number)) {
    LOG(WARNING) << "Not a supported IP literal: " << host;
    return false;
  }
  *addrlist = net::AddressList();
  addrlist->push_back(net::IPEndPoint(ip_number, port));
  return true;
}

} // End of empty namespace

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
  DCHECK(message);

  // Break down the multiple regular expressions separated
  // by single line break  and match pattern against the
  // incoming message

  int line = 1;
  icu::UnicodeString input(
    icu::UnicodeString::fromUTF8(message->ToString()));
  std::vector<std::string> regexps;
  Tokenize(regular_expressions_, "\n", &regexps);
  for (std::vector<std::string>::iterator i = regexps.begin(),
       ie = regexps.end(); i != ie; ++i) {
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher matcher(icu::UnicodeString::fromUTF8(*i), 0, status);
    DCHECK(U_SUCCESS(status));
    matcher.reset(input);
    EXPECT_TRUE(matcher.find())
      << "Failed to match pattern '" << *i << "', line " << line;
    ++line;
  }
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

UDPChannelAdapter::UDPChannelAdapter(net::ClientSocketFactory *socket_factory,
                                     net::NetLog *net_log)
  : socket_factory_(socket_factory), net_log_(net_log) {}

UDPChannelAdapter::~UDPChannelAdapter() {}

int UDPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  net::AddressList addrlist;
  if (!ParseHostPortPair(destination, &addrlist))
    return net::ERR_UNEXPECTED;
  net::NetLog::Source no_source;
  socket_.reset(socket_factory_->CreateDatagramClientSocket(
    net::DatagramSocket::DEFAULT_BIND, base::Bind(&base::RandInt),
    net_log_, no_source));
  return socket_->Connect(addrlist.front());
}

int UDPChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(socket_);
  return socket_->Read(buf, buf_len, callback);
}

int UDPChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(socket_);
  return socket_->Write(buf, buf_len, callback);
}

TCPChannelAdapter::TCPChannelAdapter(net::ClientSocketFactory *socket_factory,
                                     net::NetLog *net_log)
  : socket_factory_(socket_factory), net_log_(net_log) {}

TCPChannelAdapter::~TCPChannelAdapter() {}

int TCPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  net::AddressList addrlist;
  if (!ParseHostPortPair(destination, &addrlist))
    return net::ERR_UNEXPECTED;
  net::NetLog::Source no_source;
  socket_.reset(socket_factory_->CreateTransportClientSocket(
    addrlist, net_log_, no_source));
  return socket_->Connect(callback);
}

int TCPChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(socket_);
  return socket_->Read(buf, buf_len, callback);
}

int TCPChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(socket_);
  return socket_->Write(buf, buf_len, callback);
}

TLSChannelAdapter::TLSChannelAdapter(net::ClientSocketFactory *socket_factory,
                                     net::NetLog *net_log)
  : socket_factory_(socket_factory), net_log_(net_log),
    cert_verifier_(new net::MockCertVerifier),
    ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)) {
  cert_verifier_->set_default_result(net::OK);
  context_.cert_verifier = cert_verifier_.get();
}

TLSChannelAdapter::~TLSChannelAdapter() {}

int TLSChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  if (!ParseHostPortPair(destination, &addrlist_))
    return net::ERR_UNEXPECTED;
  net::NetLog::Source no_source;
  scoped_ptr<net::StreamSocket> inner_socket;
  tcp_socket_.reset(socket_factory_->CreateTransportClientSocket(
    addrlist_, net_log_, no_source));
  int result = tcp_socket_->Connect(
    base::Bind(&TLSChannelAdapter::OnConnected, weak_factory_.GetWeakPtr()));
  if (result == net::ERR_IO_PENDING)
    connect_callback_ = callback;
  return result;
}

int TLSChannelAdapter::Read(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(ssl_socket_);
  return ssl_socket_->Read(buf, buf_len, callback);
}

int TLSChannelAdapter::Write(
        net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {
  DCHECK(ssl_socket_);
  return ssl_socket_->Write(buf, buf_len, callback);
}

void TLSChannelAdapter::OnConnected(int result) {
  DCHECK(tcp_socket_);
  if (result == net::OK) {
    net::HostPortPair host_and_port(
      net::HostPortPair::FromIPEndPoint(addrlist_.front()));
    ssl_socket_.reset(socket_factory_->CreateSSLClientSocket(
      tcp_socket_.get(), host_and_port, kDefaultSSLConfig, context_));
  }
  if (!connect_callback_.is_null()) {
    connect_callback_.Run(result);
  }
}

MockChannel::MockChannel(MockChannelAdapter *channel_adapter,
                         Channel::Delegate *delegate,
                         const EndPoint &destination)
    : channel_adapter_(channel_adapter),
      delegate_(delegate),
      ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)),
      destination_(destination),
      origin_(net::HostPortPair("192.0.2.33", 123),
              destination.protocol()) {}

MockChannel::~MockChannel() {}

const int MockChannel::kBufferSize = 1500;

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

void MockChannel::Read() {
  int result;
  for (;;) {
    result = channel_adapter_->Read(io_buffer_, io_buffer_->size(),
      base::Bind(&MockChannel::OnRead, weak_factory_.GetWeakPtr()));
    if (result < net::OK)
      break;
    HandleMessage(result);
  }
  if (result != net::ERR_IO_PENDING)
    delegate_->OnChannelClosed(this, result);
}

void MockChannel::HandleMessage(int read_bytes) {
  std::string raw_message(io_buffer_->data(), read_bytes);
  scoped_refptr<Message> message = Message::Parse(raw_message);
  delegate_->OnIncomingMessage(this, message);
}

void MockChannel::OnConnected(int result) {
  is_connected_ = true;
  if (delegate_)
    delegate_->OnChannelConnected(this, result);
  io_buffer_ = new net::IOBufferWithSize(kBufferSize);
  Read();
}

void MockChannel::OnRead(int result) {
  if (result < net::OK)
    delegate_->OnChannelClosed(this, result);
  else {
    HandleMessage(result);
    Read();
  }
}

MockChannelFactory::MockChannelFactory(
    net::ClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {
  DCHECK(socket_factory);
}

MockChannelFactory::~MockChannelFactory() {}

int MockChannelFactory::CreateChannel(const EndPoint &destination,
                                      Channel::Delegate *delegate,
                                      scoped_refptr<Channel> *channel) {
  MockChannelAdapter *channel_adapter = 0;
  if (destination.protocol() == Protocol::UDP)
    channel_adapter = new UDPChannelAdapter(socket_factory_, net_log_.net_log());
  else if (destination.protocol() == Protocol::TCP)
    channel_adapter = new TCPChannelAdapter(socket_factory_, net_log_.net_log());
  else if (destination.protocol() == Protocol::TLS)
    channel_adapter = new TCPChannelAdapter(socket_factory_, net_log_.net_log());
  if (channel_adapter) {
    *channel = new MockChannel(channel_adapter, delegate, destination);
    return net::OK;
  }
  NOTREACHED();
  return net::ERR_UNEXPECTED;
}

MockClientTransaction::MockClientTransaction() {
  // TODO
}

MockClientTransaction::~MockClientTransaction() {}

void MockClientTransaction::TimedOut() {
  // TODO
}

const std::string& MockClientTransaction::id() const {
  return transaction_id_;
}

scoped_refptr<Channel> MockClientTransaction::channel() const {
  return channel_;
}

void MockClientTransaction::Start(
                    const scoped_refptr<Request> &outgoing_request) {
  // TODO
}

void MockClientTransaction::HandleIncomingResponse(
                    const scoped_refptr<Response> &response) {
  // TODO
}

void MockClientTransaction::Close() {
  // TODO
}

MockServerTransaction::MockServerTransaction() {
  // TODO
}

MockServerTransaction::~MockServerTransaction() {}

const std::string& MockServerTransaction::id() const {
  return transaction_id_;
}

scoped_refptr<Channel> MockServerTransaction::channel() const {
  return channel_;
}

void MockServerTransaction::Start(
                const scoped_refptr<Request> &incoming_request) {
  // TODO
}

void MockServerTransaction::Send(
                    const scoped_refptr<Response> &response,
                    const net::CompletionCallback& callback) {
  // TODO
}

void MockServerTransaction::HandleIncomingRequest(
                    const scoped_refptr<Request> &request) {
  // TODO
}

void MockServerTransaction::Close() {
  // TODO
}

MockTransactionFactory::MockTransactionFactory()
  : client_transaction_(new MockClientTransaction),
    server_transaction_(new MockServerTransaction) {}

MockTransactionFactory::~MockTransactionFactory() {}

MockClientTransaction *MockTransactionFactory::client_transaction() {
  return client_transaction_;
}

MockServerTransaction *MockTransactionFactory::server_transaction() {
  return server_transaction_;
}

ClientTransaction *MockTransactionFactory::CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
  client_transaction_->set_id(transaction_id);
  client_transaction_->set_channel(channel);
  client_transaction_->set_delegate(delegate);
  return client_transaction_;
}

ServerTransaction *MockTransactionFactory::CreateServerTransaction(
    const Method &method,
    const std::string &transaction_id,
    const scoped_refptr<Channel> &channel,
    TransactionDelegate *delegate) {
  server_transaction_->set_id(transaction_id);
  server_transaction_->set_channel(channel);
  server_transaction_->set_delegate(delegate);
  return server_transaction_;
}

} // End of sippet namespace
