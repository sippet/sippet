// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/transport_test_util.h"

#include <string>

#include "base/strings/string_util.h"
#include "base/strings/string_tokenizer.h"
#include "base/rand_util.h"
#include "net/base/url_util.h"
#include "url/gurl.h"
#include "url/url_canon_ip.h"

#include "third_party/icu/source/i18n/unicode/regex.h"

namespace sippet {

namespace {

bool ParseIPLiteralToBytes(const base::StringPiece& ip_literal,
                           std::vector<uint8_t>* bytes) {
  // |ip_literal| could be either an IPv4 or an IPv6 literal. If it contains
  // a colon however, it must be an IPv6 address.
  if (ip_literal.find(':') != base::StringPiece::npos) {
    // GURL expects IPv6 hostnames to be surrounded with brackets.
    std::string host_brackets = "[";
    ip_literal.AppendToString(&host_brackets);
    host_brackets.push_back(']');
    url::Component host_comp(0, host_brackets.size());

    // Try parsing the hostname as an IPv6 literal.
    bytes->resize(16);  // 128 bits.
    return url::IPv6AddressToNumber(host_brackets.data(), host_comp,
                                    bytes->data());
  }

  // Otherwise the string is an IPv4 address.
  bytes->resize(4);  // 32 bits.
  url::Component host_comp(0, ip_literal.size());
  int num_components;
  url::CanonHostInfo::Family family = url::IPv4AddressToNumber(
      ip_literal.data(), host_comp, bytes->data(), &num_components);
  return family == url::CanonHostInfo::IPV4;
}

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
  std::vector<uint8_t> bytes;
  if (!ParseIPLiteralToBytes(host, &bytes)) {
    LOG(WARNING) << "Not a supported IP literal: " << host;
    return false;
  }
  net::IPAddress ip_number(bytes);
  *addrlist = net::AddressList();
  addrlist->push_back(net::IPEndPoint(ip_number, port));
  return true;
}

class ExpectNothing : public MockEvent::Expect {
 public:
  void OnChannelConnected(const EndPoint &destination, int error) override {
    EXPECT_TRUE(false) << "Not expected a channel connect at this time";
  }
  void OnChannelClosed(const EndPoint& destination) override {
    EXPECT_TRUE(false) << "Not expected a channel close at this time";
  }
  void OnIncomingRequest(const scoped_refptr<Request> &) override {
    EXPECT_TRUE(false) << "Not expected to get an incoming request this time";
  }
  void OnIncomingResponse(const scoped_refptr<Response> &) override {
    EXPECT_TRUE(false) << "Not expected to get an incoming response this time";
  }
  void OnTimedOut(const scoped_refptr<Request> &request) override {
    EXPECT_TRUE(false) << "Not expected a timeout at this time";
  }
  void OnTransportError(
      const scoped_refptr<Request> &request, int error) override {
    EXPECT_TRUE(false) << "Not expected a transport error at this time";
  }
  void Start(const scoped_refptr<Request>&) override {
    EXPECT_TRUE(false) << "Not expected transaction start at this time";
  }
  void Send(const scoped_refptr<Response>&) override {
    EXPECT_TRUE(false) << "Not expected to send a response at this time";
  }
  void HandleIncomingResponse(const scoped_refptr<Response>&) override {
    EXPECT_TRUE(false) << "Not expected to handle incoming response";
  }
  void HandleIncomingRequest(const scoped_refptr<Request>&) override {
    EXPECT_TRUE(false) << "Not expected to handle incoming request";
  }
  void Close() override {
    EXPECT_TRUE(false) << "Not expected to close transaction at this time";
  }
};

class ChannelConnectedImpl : public ExpectNothing {
 public:
  explicit ChannelConnectedImpl(const EndPoint &destination)
    : has_error_(false), destination_(destination) {}
  ChannelConnectedImpl(const EndPoint &destination, int error)
    : has_error_(true), destination_(destination), error_(error) {}
  ~ChannelConnectedImpl() override {}
  void OnChannelConnected(const EndPoint& destination, int error) override {
    EXPECT_EQ(destination_, destination);
    if (has_error_)
      EXPECT_EQ(error_, error);
  }
 private:
  bool has_error_;
  EndPoint destination_;
  int error_;
};

class ChannelClosedImpl : public ExpectNothing {
 public:
  explicit ChannelClosedImpl(const EndPoint &destination)
    : destination_(destination) {}
  ~ChannelClosedImpl() override {}
  void OnChannelClosed(const EndPoint& destination) override {
    EXPECT_EQ(destination_, destination);
  }
 private:
  EndPoint destination_;
};

class IncomingMessageImpl : public ExpectNothing {
 public:
  explicit IncomingMessageImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  ~IncomingMessageImpl() override {}
  void OnIncomingRequest(
      const scoped_refptr<Request>& request) override {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
  void OnIncomingResponse(
      const scoped_refptr<Response>& response) override {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectStartImpl : public ExpectNothing {
 public:
  explicit ExpectStartImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  ~ExpectStartImpl() override {}
  void Start(const scoped_refptr<Request>& request) override {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectSendImpl : public ExpectNothing {
 public:
  explicit ExpectSendImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  ~ExpectSendImpl() override {}
  void Send(const scoped_refptr<Response>& response) override {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectIncomingResponseImpl : public ExpectNothing {
 public:
  explicit ExpectIncomingResponseImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  ~ExpectIncomingResponseImpl() override {}
  void HandleIncomingResponse(
          const scoped_refptr<Response>& response) override {
    DCHECK(response);
    MatchMessage(response, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectIncomingRequestImpl : public ExpectNothing {
 public:
  explicit ExpectIncomingRequestImpl(const char *regular_expressions)
    : regular_expressions_(regular_expressions) {}
  ~ExpectIncomingRequestImpl() override {}
  void HandleIncomingRequest(
          const scoped_refptr<Request>& request) override {
    DCHECK(request);
    MatchMessage(request, regular_expressions_);
  }
 private:
  const char *regular_expressions_;
};

class ExpectCloseImpl : public ExpectNothing {
 public:
  ExpectCloseImpl() {}
  ~ExpectCloseImpl() override {}
  void Close() override {
    DVLOG(1) << "Transaction closed successfully";
  }
};

}  // namespace

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
  base::CStringTokenizer t(regular_expressions,
      regular_expressions + strlen(regular_expressions), "\n");
  while (t.GetNext()) {
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher matcher(icu::UnicodeString::fromUTF8(t.token()),
        0, status);
    DCHECK(U_SUCCESS(status));
    matcher.reset(input);
    if (!matcher.find()) {
      EXPECT_TRUE(false)
        << "Failed to match pattern '" << t.token() << "', line " << line;
      result = false;
    }
    ++line;
  }
  return result;
}

MockBranchFactory::MockBranchFactory(
          const char *branches[], size_t branches_count)
  : branches_(branches), branches_count_(branches_count), branches_index_(0) {}

MockBranchFactory::~MockBranchFactory() {}

std::string MockBranchFactory::CreateBranch() {
  DCHECK(branches_index_ < branches_count_);
  return branches_[branches_index_++];
}

MockEvent::MockEvent(Expect *expect, std::string *transaction_id)
  : expect_(expect), transaction_id_(transaction_id) {
}

MockEvent::MockEvent(const MockEvent &other)
  : expect_(other.expect_), time_stamp_(other.time_stamp_),
    transaction_id_(other.transaction_id_) {
}

MockEvent::~MockEvent() {}

StaticDataProvider::StaticDataProvider()
  : events_(nullptr), events_index_(0), events_count_(0) {}

StaticDataProvider::StaticDataProvider(MockEvent *events,
                                       size_t events_count)
  : events_(events), events_index_(0), events_count_(events_count) {}

StaticDataProvider::~StaticDataProvider() {}

MockEvent& StaticDataProvider::PeekEvent() {
  return PeekEvent(events_index_);
}

MockEvent& StaticDataProvider::PeekEvent(size_t index) {
  DCHECK_LT(index, events_count_);
  return events_[index];
}

size_t StaticDataProvider::events_index() const {
  return events_index_;
}

size_t StaticDataProvider::events_count() const {
  return events_count_;
}

bool StaticDataProvider::at_events_end() const {
  return events_index_ >= events_count_;
}

MockEvent &StaticDataProvider::GetNextEvent() {
  DCHECK(!at_events_end());
  events_[events_index_].set_time_stamp(base::Time::Now());
  return events_[events_index_++];
}

void StaticDataProvider::set_transaction_id(
        const std::string &transaction_id) {
  PeekEvent().set_transaction_id(transaction_id);
}

void StaticDataProvider::OnChannelConnected(
        const EndPoint& destination, int error) {
  GetNextEvent().OnChannelConnected(destination, error);
}

void StaticDataProvider::OnChannelClosed(
        const EndPoint& destination) {
  GetNextEvent().OnChannelClosed(destination);
}

void StaticDataProvider::OnIncomingMessage(Message *message) {
  if (isa<Request>(message)) {
    GetNextEvent().OnIncomingRequest(dyn_cast<Request>(message));
  } else {
    GetNextEvent().OnIncomingResponse(dyn_cast<Response>(message));
  }
}

void StaticDataProvider::Start(
        const scoped_refptr<Request>& starting_request) {
  GetNextEvent().Start(starting_request);
}

void StaticDataProvider::Send(
        const std::string &transaction_id,
        const scoped_refptr<Response>& response) {
  if (!PeekEvent().transaction_id().empty())
    EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
  GetNextEvent().Send(response);
}

void StaticDataProvider::HandleIncomingResponse(
        const std::string &transaction_id,
        const scoped_refptr<Response>& response) {
  if (!PeekEvent().transaction_id().empty())
    EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
  GetNextEvent().HandleIncomingResponse(response);
}

void StaticDataProvider::HandleIncomingRequest(
        const std::string &transaction_id,
        const scoped_refptr<Request>& request) {
  if (!PeekEvent().transaction_id().empty())
    EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
  GetNextEvent().HandleIncomingRequest(request);
}

void StaticDataProvider::Close(const std::string &transaction_id) {
  if (!PeekEvent().transaction_id().empty())
    EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
  GetNextEvent().Close();
}

MockEvent ExpectConnectChannel(const char *destination) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new ChannelConnectedImpl(endpoint));
}

MockEvent ExpectConnectChannel(const char *destination, int error) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new ChannelConnectedImpl(endpoint, error));
}

MockEvent ExpectCloseChannel(const char *destination) {
  EndPoint endpoint(EndPoint::FromString(destination));
  return MockEvent(new ChannelClosedImpl(endpoint));
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
  : data_provider_(data_provider) {
}

StaticNetworkLayerDelegate::~StaticNetworkLayerDelegate() {
}

void StaticNetworkLayerDelegate::OnChannelConnected(
    const EndPoint& destination, int error) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnChannelConnected(destination, error);
}

void StaticNetworkLayerDelegate::OnChannelClosed(const EndPoint& destination) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnChannelClosed(destination);
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

void StaticNetworkLayerDelegate::OnTimedOut(
    const scoped_refptr<Request> &request) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnTimedOut(request);
}

void StaticNetworkLayerDelegate::OnTransportError(
    const scoped_refptr<Request> &request, int error) {
  DCHECK(data_provider_ && !data_provider_->at_events_end());
  data_provider_->GetNextEvent().OnTransportError(request, error);
}

UDPChannelAdapter::UDPChannelAdapter(net::ClientSocketFactory *socket_factory,
                                     net::NetLog *net_log)
  : socket_factory_(socket_factory), net_log_(net_log) {
}

UDPChannelAdapter::~UDPChannelAdapter() {
}

bool UDPChannelAdapter::is_secure() const {
  return false;
}

int UDPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  net::AddressList addrlist;
  if (!ParseHostPortPair(destination, &addrlist))
    return net::ERR_UNEXPECTED;

  net::NetLogSource no_source;
  socket_ = socket_factory_->CreateDatagramClientSocket(
    net::DatagramSocket::DEFAULT_BIND, base::Bind(&base::RandInt),
    net_log_, no_source);
  if (!socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  } else {
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
  : socket_factory_(socket_factory), net_log_(net_log) {
}

TCPChannelAdapter::~TCPChannelAdapter() {
}

bool TCPChannelAdapter::is_secure() const {
  return false;
}

int TCPChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  net::AddressList addrlist;
  if (!ParseHostPortPair(destination, &addrlist))
    return net::ERR_UNEXPECTED;
  net::NetLogSource no_source;
  socket_ = socket_factory_->CreateTransportClientSocket(
    addrlist, nullptr, net_log_, no_source);
  if (!socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  } else {
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
    weak_ptr_factory_(this) {
  cert_verifier_->set_default_result(net::OK);
  context_.cert_verifier = cert_verifier_.get();
}

TLSChannelAdapter::~TLSChannelAdapter() {
}

bool TLSChannelAdapter::is_secure() const {
  return true;
}

int TLSChannelAdapter::Connect(
        const net::HostPortPair &destination,
        const net::CompletionCallback& callback) {
  if (!ParseHostPortPair(destination, &addrlist_))
    return net::ERR_UNEXPECTED;
  net::NetLogSource no_source;
  std::unique_ptr<net::StreamSocket> inner_socket;
  tcp_socket_ = socket_factory_->CreateTransportClientSocket(
    addrlist_, nullptr, net_log_, no_source);
  if (!tcp_socket_.get()) {
    LOG(WARNING) << "Failed to create socket.";
    return net::ERR_UNEXPECTED;
  }
  int result = tcp_socket_->Connect(
    base::Bind(&TLSChannelAdapter::OnConnected, weak_ptr_factory_.GetWeakPtr()));
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

    std::unique_ptr<net::ClientSocketHandle> connection(new net::ClientSocketHandle);
    connection->SetSocket(std::move(tcp_socket_));

    net::SSLConfig config;
    ssl_socket_ = socket_factory_->CreateSSLClientSocket(
      std::move(connection), host_and_port, config, context_);
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
    : delegate_(delegate),
      destination_(destination),
      origin_(net::HostPortPair("192.0.2.33", 123),
              destination.protocol()),
      is_connected_(false),
      is_stream_(is_stream),
      channel_adapter_(channel_adapter),
      weak_ptr_factory_(this) {}

MockChannel::~MockChannel() {
}

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
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
      base::Bind(&MockChannel::OnConnected, this, net::OK));
  }
}

int MockChannel::ReconnectIgnoringLastError() {
  return net::ERR_NOT_IMPLEMENTED;
}

int MockChannel::ReconnectWithCertificate(net::X509Certificate* client_cert,
      net::SSLPrivateKey* private_key) {
  return net::ERR_NOT_IMPLEMENTED;
}

int MockChannel::Send(const scoped_refptr<Message>& message,
                      const net::CompletionCallback& callback) {
  DCHECK(is_connected());
  std::string buffer(message->ToString());
  scoped_refptr<net::IOBuffer> io_buffer(new net::IOBuffer(buffer.size()));
  memcpy(io_buffer->data(), buffer.data(), buffer.size());
  int result = channel_adapter_->Write(io_buffer.get(),
      buffer.size(), callback);
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

void MockChannel::SetKeepAlive(int seconds) {
  // do nothing
}

void MockChannel::Read() {
  int result;
  for (;;) {
    result = channel_adapter_->Read(io_buffer_.get(), io_buffer_->size(),
      base::Bind(&MockChannel::OnRead, weak_ptr_factory_.GetWeakPtr()));
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
  if (result < net::OK) {
    delegate_->OnChannelClosed(this, result);
  } else {
    HandleMessage(result);
    Read();
  }
}

MockChannelFactory::MockChannelFactory(
    net::ClientSocketFactory *socket_factory)
  : socket_factory_(socket_factory) {
  DCHECK(socket_factory);
}

MockChannelFactory::~MockChannelFactory() {
}

int MockChannelFactory::CreateChannel(const EndPoint &destination,
                                      Channel::Delegate *delegate,
                                      scoped_refptr<Channel> *channel) {
  bool is_stream = true;
  MockChannelAdapter *channel_adapter = 0;
  if (destination.protocol() == Protocol::UDP) {
    channel_adapter = new UDPChannelAdapter(socket_factory_,
        net_log_.net_log());
    is_stream = false;
  } else if (destination.protocol() == Protocol::TCP) {
    channel_adapter = new TCPChannelAdapter(socket_factory_,
        net_log_.net_log());
  } else if (destination.protocol() == Protocol::TLS) {
    channel_adapter = new TCPChannelAdapter(socket_factory_,
        net_log_.net_log());
  }
  if (channel_adapter) {
    *channel = new MockChannel(channel_adapter, is_stream,
        delegate, destination);
    return net::OK;
  }
  NOTREACHED();
  return net::ERR_UNEXPECTED;
}

MockClientTransaction::MockClientTransaction(
    DataProvider *data_provider)
  : data_provider_(data_provider) {
}

MockClientTransaction::~MockClientTransaction() {
}

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
    DataProvider *data_provider) : data_provider_(data_provider) {
}

MockServerTransaction::~MockServerTransaction() {
}

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
  : data_provider_(data_provider) {
}

MockTransactionFactory::~MockTransactionFactory() {
}

MockClientTransaction*
      MockTransactionFactory::client_transaction(size_t index) const {
  DCHECK(index < client_transactions_.size());
  return client_transactions_[index].get();
}

MockServerTransaction*
      MockTransactionFactory::server_transaction(size_t index) const {
  DCHECK(index < server_transactions_.size());
  return server_transactions_[index].get();
}

ClientTransaction *MockTransactionFactory::CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
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
    TimeDeltaFactory *time_delta_factory,
    TransactionDelegate *delegate) {
  MockServerTransaction *server_transaction =
    new MockServerTransaction(data_provider_);
  server_transaction->set_id(transaction_id);
  server_transaction->set_channel(channel);
  server_transaction->set_delegate(delegate);
  server_transactions_.push_back(server_transaction);
  return server_transaction;
}

}  // namespace sippet
