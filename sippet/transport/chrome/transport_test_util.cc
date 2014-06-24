// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/transport_test_util.h"

#include "base/strings/string_util.h"
#include "base/rand_util.h"

#include "third_party/icu/source/i18n/unicode/regex.h"

namespace sippet {

namespace {

const net::SSLConfig kDefaultSSLConfig;

bool ParseHostPortPair(const net::HostPortPair &destination,
                       net::AddressList *addrlist) {
  std::string host;
  int port;
  std::string destination_string(destination.ToString());
  if (!net::ParseHostAndPort(destination_string, &host, &port)) {
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

class ExpectNothing : public MockEvent::Expect {
 public:
  virtual void OnChannelConnected(const EndPoint &destination) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected a channel connect at this time";
  }
  virtual void OnChannelClosed(
                    const EndPoint& destination, int error) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected a channel close at this time";
  }
  virtual void OnIncomingRequest(const scoped_refptr<Request> &) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to get an incoming request this time";
  }
  virtual void OnIncomingResponse(const scoped_refptr<Response> &) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to get an incoming response this time";
  }
  virtual void OnTimedOut(const std::string &id) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected a timeout at this time";
  }
  virtual void OnTransportError(const std::string &id, int error) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected a transport error at this time";
  }
  virtual void Start(const scoped_refptr<Request>&) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected transaction start at this time";
  }
  virtual void Send(const scoped_refptr<Response>&) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to send a response at this time";
  }
  virtual void HandleIncomingResponse(
                    const scoped_refptr<Response>&) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to handle incoming response";
  }
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request>&) OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to handle incoming request";
  }
  virtual void Close() OVERRIDE {
    EXPECT_TRUE(false) << "Not expected to close transaction at this time";
  }
};

class ChannelClosedImpl : public ExpectNothing {
 public:
  ChannelClosedImpl(const EndPoint &destination)
    : has_error_(false), destination_(destination) {}
  ChannelClosedImpl(const EndPoint &destination, int error)
    : has_error_(true), destination_(destination), error_(error) {}
  virtual ~ChannelClosedImpl() {}
  void OnChannelClosed(const EndPoint& destination, int error) OVERRIDE {
    EXPECT_EQ(destination_, destination);
    if (has_error_)
      EXPECT_EQ(error_, error);
  }
 private:
  EndPoint destination_;
  bool has_error_;
  int error_;
};

class IncomingMessageImpl : public ExpectNothing {
 public:
  IncomingMessageImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  virtual ~IncomingMessageImpl() {}
  virtual void OnIncomingRequest(
      const scoped_refptr<Request>& request) OVERRIDE {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
  virtual void OnIncomingResponse(
      const scoped_refptr<Response>& response) OVERRIDE {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectStartImpl : public ExpectNothing {
 public:
  ExpectStartImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  virtual ~ExpectStartImpl() {}
  virtual void Start(const scoped_refptr<Request>& request) OVERRIDE {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectSendImpl : public ExpectNothing {
 public:
  ExpectSendImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  virtual ~ExpectSendImpl() {}
  virtual void Send(const scoped_refptr<Response>& response) OVERRIDE {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectIncomingResponseImpl : public ExpectNothing {
 public:
  ExpectIncomingResponseImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  virtual ~ExpectIncomingResponseImpl() {}
  virtual void HandleIncomingResponse(
                    const scoped_refptr<Response>& response) OVERRIDE {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectIncomingRequestImpl : public ExpectNothing {
 public:
  ExpectIncomingRequestImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  virtual ~ExpectIncomingRequestImpl() {}
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request>& request) OVERRIDE {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectCloseImpl : public ExpectNothing {
 public:
  ExpectCloseImpl() {}
  virtual ~ExpectCloseImpl() {}
  virtual void Close() OVERRIDE {
    DVLOG(1) << "Transaction closed successfully";
  }
};

} // End of empty namespace

bool MatchMessage(const scoped_refptr<Message> &message,
                  const char *regular_expressions) {
  DCHECK(message);
  DCHECK(regular_expressions);

  // Break down the multiple regular expressions separated
  // by single line break  and match pattern against the
  // incoming message

  int line = 1;
  bool result = true;
  icu::UnicodeString input(
    icu::UnicodeString::fromUTF8(message->ToString()));
  std::vector<std::string> regexps;
  Tokenize(regular_expressions, "\n", &regexps);
  for (std::vector<std::string>::iterator i = regexps.begin(),
       ie = regexps.end(); i != ie; ++i) {
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher matcher(icu::UnicodeString::fromUTF8(*i), 0, status);
    DCHECK(U_SUCCESS(status));
    matcher.reset(input);
    if (!matcher.find()) {
      EXPECT_TRUE(false)
        << "Failed to match pattern '" << *i << "', line " << line;
      result = false;
    }
    ++line;
  }
  return result;
}

MockBranchFactory::MockBranchFactory(
          const char *branches[], size_t branches_count)
  : branches_(branches), branches_index_(0),
    branches_count_(branches_count) {}

MockBranchFactory::~MockBranchFactory() {}

std::string MockBranchFactory::CreateBranch() {
  DCHECK(branches_index_ < branches_count_);
  return branches_[branches_index_++];
}

MockEvent ExpectCloseChannel(const char *destination) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new ChannelClosedImpl(endpoint));
}

MockEvent ExpectCloseChannel(const char *destination, int error) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new ChannelClosedImpl(endpoint, error));
}

MockEvent ExpectIncomingMessage(const char *regular_expressions) {
  return MockEvent(new IncomingMessageImpl(regular_expressions));
}

MockEvent ExpectStartTransaction(const char *regular_expressions,
                                 std::string *transaction_id) {
  return MockEvent(new ExpectStartImpl(regular_expressions), transaction_id);
}

MockEvent ExpectTransactionSend(const char *regular_expressions,
                                std::string *transaction_id) {
  return MockEvent(new ExpectSendImpl(regular_expressions), transaction_id);
}

MockEvent ExpectIncomingResponse(const char *regular_expressions,
                                 std::string *transaction_id) {
  return MockEvent(new ExpectIncomingResponseImpl(regular_expressions),
            transaction_id);
}

MockEvent ExpectIncomingRequest(const char *regular_expressions,
                                std::string *transaction_id) {
  return MockEvent(new ExpectIncomingRequestImpl(regular_expressions),
            transaction_id);
}

MockEvent ExpectTransactionClose(std::string *transaction_id) {
  return MockEvent(new ExpectCloseImpl, transaction_id);
}

StaticNetworkLayerDelegate::StaticNetworkLayerDelegate(
              DataProvider *data_provider)
  : data_provider_(data_provider) {}

StaticNetworkLayerDelegate::~StaticNetworkLayerDelegate() {}

void StaticNetworkLayerDelegate::OnChannelConnected(const EndPoint& destination) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnChannelConnected(destination);
}

void StaticNetworkLayerDelegate::OnChannelClosed(const EndPoint& destination, int error) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnChannelClosed(destination, error);
}

void StaticNetworkLayerDelegate::OnIncomingRequest(
    const scoped_refptr<Request>& request) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnIncomingRequest(request);
}

void StaticNetworkLayerDelegate::OnIncomingResponse(
    const scoped_refptr<Response>& response) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnIncomingResponse(response);
}

void StaticNetworkLayerDelegate::OnTimedOut(const std::string &id) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnTimedOut(id);
}

void StaticNetworkLayerDelegate::OnTransportError(
                                  const std::string &id, int error) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnTransportError(id, error);
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
  socket_ = socket_factory_->CreateDatagramClientSocket(
    net::DatagramSocket::DEFAULT_BIND, base::Bind(&base::RandInt),
    net_log_, no_source);
  if (!socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  }
  else {
    return socket_->Connect(addrlist.front());
  }
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
  socket_ = socket_factory_->CreateTransportClientSocket(
    addrlist, net_log_, no_source);
  if (!socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  }
  else {
    return socket_->Connect(callback);
  }
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
    weak_factory_(this) {
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
  tcp_socket_ = socket_factory_->CreateTransportClientSocket(
    addrlist_, net_log_, no_source);
  if (!tcp_socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  }
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

    scoped_ptr<net::ClientSocketHandle> connection(new net::ClientSocketHandle);
    connection->SetSocket(tcp_socket_.Pass());

    ssl_socket_ = socket_factory_->CreateSSLClientSocket(
      connection.Pass(), host_and_port, kDefaultSSLConfig, context_);
    if (!ssl_socket_.get()) {
      LOG(WARNING) << "Failed to create socket.";
    }
  }
  if (!connect_callback_.is_null()) {
    connect_callback_.Run(result);
  }
}

MockChannel::MockChannel(MockChannelAdapter *channel_adapter,
                         bool is_stream,
                         Channel::Delegate *delegate,
                         const EndPoint &destination)
    : channel_adapter_(channel_adapter),
      delegate_(delegate),
      is_connected_(false),
      is_stream_(is_stream),
      weak_factory_(this),
      destination_(destination),
      origin_(net::HostPortPair("192.0.2.33", 123),
              destination.protocol()) {}

MockChannel::~MockChannel() {}

const int MockChannel::kBufferSize = 1500;

int MockChannel::origin(EndPoint *origin) const {
  *origin = origin_;
  return net::OK;
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

bool MockChannel::is_stream() const {
  return is_stream_;
}

void MockChannel::Connect() {
  DCHECK(!is_connected());
  int result = channel_adapter_->Connect(destination_.hostport(),
    base::Bind(&MockChannel::OnConnected, this));
  if (result == net::OK) {
    // When using UDP, the connect event will occur in the next event loop.
    base::MessageLoop::current()->PostTask(FROM_HERE,
      base::Bind(&MockChannel::OnConnected, this, net::OK));
  }
}

int MockChannel::Send(const scoped_refptr<Message>& message,
                      const net::CompletionCallback& callback) {
  DCHECK(is_connected());
  std::string buffer(message->ToString());
  scoped_refptr<net::IOBuffer> io_buffer(new net::IOBuffer(buffer.size()));
  memcpy(io_buffer->data(), buffer.data(), buffer.size());
  int result = channel_adapter_->Write(io_buffer, buffer.size(), callback);
  return (result > 0) ? net::OK : result;
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
  bool is_stream = true;
  MockChannelAdapter *channel_adapter = 0;
  if (destination.protocol() == Protocol::UDP) {
    channel_adapter = new UDPChannelAdapter(socket_factory_, net_log_.net_log());
    is_stream = false;
  }
  else if (destination.protocol() == Protocol::TCP)
    channel_adapter = new TCPChannelAdapter(socket_factory_, net_log_.net_log());
  else if (destination.protocol() == Protocol::TLS)
    channel_adapter = new TCPChannelAdapter(socket_factory_, net_log_.net_log());
  if (channel_adapter) {
    *channel = new MockChannel(channel_adapter, is_stream, delegate, destination);
    return net::OK;
  }
  NOTREACHED();
  return net::ERR_UNEXPECTED;
}

MockClientTransaction::MockClientTransaction(
    DataProvider *data_provider)
  : data_provider_(data_provider) {}

MockClientTransaction::~MockClientTransaction() {}

const std::string& MockClientTransaction::id() const {
  return transaction_id_;
}

scoped_refptr<Channel> MockClientTransaction::channel() const {
  return channel_;
}

void MockClientTransaction::Start(
                    const scoped_refptr<Request> &outgoing_request) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->set_transaction_id(transaction_id_);
  data_provider_->Start(outgoing_request);
}

void MockClientTransaction::HandleIncomingResponse(
                    const scoped_refptr<Response> &response) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->HandleIncomingResponse(transaction_id_, response);
  delegate_->OnIncomingResponse(response);
}

void MockClientTransaction::Close() {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->Close(transaction_id_);
}

MockServerTransaction::MockServerTransaction(
    DataProvider *data_provider) : data_provider_(data_provider) {}

MockServerTransaction::~MockServerTransaction() {}

const std::string& MockServerTransaction::id() const {
  return transaction_id_;
}

scoped_refptr<Channel> MockServerTransaction::channel() const {
  return channel_;
}

void MockServerTransaction::Start(
                const scoped_refptr<Request> &incoming_request) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->set_transaction_id(transaction_id_);
  data_provider_->Start(incoming_request);
}

void MockServerTransaction::Send(
                    const scoped_refptr<Response> &response) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->Send(transaction_id_, response);
  channel_->Send(response, callback_.callback());
}

void MockServerTransaction::HandleIncomingRequest(
                    const scoped_refptr<Request> &request) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->HandleIncomingRequest(transaction_id_, request);
}

void MockServerTransaction::Close() {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->Close(transaction_id_);
}

MockTransactionFactory::MockTransactionFactory(DataProvider *data_provider)
  : data_provider_(data_provider) {}

MockTransactionFactory::~MockTransactionFactory() {}

MockClientTransaction*
      MockTransactionFactory::client_transaction(size_t index) const {
  DCHECK(index < client_transactions_.size());
  return client_transactions_[index];
}

MockServerTransaction*
      MockTransactionFactory::server_transaction(size_t index) const {
  DCHECK(index < server_transactions_.size());
  return server_transactions_[index];
}

ClientTransaction *MockTransactionFactory::CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
  MockClientTransaction *client_transaction =
    new MockClientTransaction(data_provider_);
  client_transaction->set_id(transaction_id);
  client_transaction->set_channel(channel);
  client_transaction->set_delegate(delegate);
  client_transactions_.push_back(client_transaction);
  return client_transaction;
}

ServerTransaction *MockTransactionFactory::CreateServerTransaction(
    const Method &method,
    const std::string &transaction_id,
    const scoped_refptr<Channel> &channel,
    TransactionDelegate *delegate) {
  MockServerTransaction *server_transaction =
    new MockServerTransaction(data_provider_);
  server_transaction->set_id(transaction_id);
  server_transaction->set_channel(channel);
  server_transaction->set_delegate(delegate);
  server_transactions_.push_back(server_transaction);
  return server_transaction;
}

} // End of sippet namespace
