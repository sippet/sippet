// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
#define SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_

#include <vector>
#include "sippet/transport/network_layer.h"
#include "sippet/transport/transaction_factory.h"
#include "sippet/transport/channel_factory.h"
#include "sippet/message/message.h"

#include "base/strings/string_util.h"
#include "base/memory/linked_ptr.h"
#include "net/socket/socket_test_util.h"
#include "net/cert/mock_cert_verifier.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

bool MatchMessage(const scoped_refptr<Message> &message,
                  const char *regular_expressions);

class MockBranchFactory : public BranchFactory {
 public:
  MockBranchFactory(const char *branches[], size_t branches_count);
  ~MockBranchFactory() override;
  std::string CreateBranch() override;
 private:
  const char **branches_;
  size_t branches_count_;
  size_t branches_index_;

  DISALLOW_COPY_AND_ASSIGN(MockBranchFactory);
};

class MockEvent {
 public:
  class Expect : public NetworkLayer::Delegate {
   public:
    ~Expect() override {}
    virtual void Start(const scoped_refptr<Request>&) = 0;
    virtual void Send(const scoped_refptr<Response>&) = 0;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) = 0;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) = 0;
    virtual void Close() = 0;
  };

  MockEvent(Expect *expect, std::string *transaction_id = nullptr);
  MockEvent(const MockEvent &other);
  ~MockEvent();

  MockEvent &operator=(const MockEvent &other) {
    expect_ = other.expect_;
    time_stamp_ = other.time_stamp_;
    return *this;
  }

  base::Time time_stamp() const { return time_stamp_; }
  void set_time_stamp(const base::Time & t) { time_stamp_ = t; }

  std::string transaction_id() const {
    if (transaction_id_)
      return *transaction_id_;
    return "";
  }
  void set_transaction_id(const std::string &transaction_id) {
    if (transaction_id_)
      *transaction_id_ = transaction_id;
  }

  // NetworkLayer::Delegate events:
  void OnChannelConnected(const EndPoint &destination, int error) {
    expect_->OnChannelConnected(destination, error);
  }
  void OnChannelClosed(const EndPoint& destination) {
    expect_->OnChannelClosed(destination);
  }
  void OnIncomingRequest(const scoped_refptr<Request> &request) {
    expect_->OnIncomingRequest(request);
  }
  void OnIncomingResponse(const scoped_refptr<Response> &response) {
    expect_->OnIncomingResponse(response);
  }
  void OnTimedOut(const scoped_refptr<Request> &request) {
    expect_->OnTimedOut(request);
  }
  void OnTransportError(const scoped_refptr<Request> &request, int error) {
    expect_->OnTransportError(request, error);
  }

  // Transaction layer events:
  void Start(const scoped_refptr<Request>& starting_request) {
    expect_->Start(starting_request);
  }
  void Send(const scoped_refptr<Response>& response) {
    expect_->Send(response);
  }
  void HandleIncomingResponse(
            const scoped_refptr<Response>& incoming_response) {
     expect_->HandleIncomingResponse(incoming_response);
  }
  void HandleIncomingRequest(
            const scoped_refptr<Request>& incoming_request) {
    expect_->HandleIncomingRequest(incoming_request);
  }
  void Close() {
    expect_->Close();
  }
 private:
  linked_ptr<Expect> expect_;
  base::Time time_stamp_; // The time stamp at which the operation occurred.
  std::string *transaction_id_;
};

class DataProvider {
 public:
  DataProvider() {}
  virtual ~DataProvider() {}

  virtual MockEvent& PeekEvent() = 0;
  virtual MockEvent& PeekEvent(size_t index) = 0;

  virtual size_t events_index() const = 0;
  virtual size_t events_count() const = 0;

  virtual bool at_events_end() const = 0;

  virtual MockEvent &GetNextEvent() = 0;

  virtual void set_transaction_id(const std::string &transaction_id) = 0;

  virtual void OnChannelConnected(const EndPoint& destination, int err) = 0;
  virtual void OnChannelClosed(const EndPoint& destination) = 0;
  virtual void OnIncomingMessage(Message *message) = 0;
  virtual void Start(const scoped_refptr<Request> &starting_request) = 0;
  virtual void Send(const std::string &transaction_id,
                    const scoped_refptr<Response> &response) = 0;
  virtual void HandleIncomingResponse(
                    const std::string &transaction_id,
                    const scoped_refptr<Response> &response) = 0;
  virtual void HandleIncomingRequest(
                    const std::string &transaction_id,
                    const scoped_refptr<Request> &request) = 0;
  virtual void Close(const std::string &transaction_id) = 0;
 private:
  DISALLOW_COPY_AND_ASSIGN(DataProvider);
};

class StaticDataProvider : public DataProvider {
 public:
  StaticDataProvider();
  StaticDataProvider(MockEvent *events,
                     size_t events_count);
  ~StaticDataProvider() override;

  MockEvent& PeekEvent() override;
  MockEvent& PeekEvent(size_t index) override;

  size_t events_index() const override;
  size_t events_count() const override;

  bool at_events_end() const override;

  MockEvent &GetNextEvent() override;

  void set_transaction_id(const std::string &transaction_id) override;

  void OnChannelConnected(const EndPoint& destination, int error) override;
  void OnChannelClosed(const EndPoint& destination) override;
  void OnIncomingMessage(Message *message) override;
  void Start(const scoped_refptr<Request>& starting_request) override;
  void Send(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) override;
  void HandleIncomingResponse(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) override;
  void HandleIncomingRequest(
          const std::string &transaction_id,
          const scoped_refptr<Request>& request) override;
  void Close(const std::string &transaction_id) override;

 private:
  MockEvent *events_;
  size_t events_index_;
  size_t events_count_;
};

// NetworkLayer::Delegate events:
MockEvent ExpectConnectChannel(const char *destination);
MockEvent ExpectConnectChannel(const char *destination, int err);
MockEvent ExpectCloseChannel(const char *destination);
MockEvent ExpectIncomingMessage(const char *regular_expressions);

// Transaction events:
MockEvent ExpectStartTransaction(const char *regular_expressions,
                                 std::string *transaction_id = nullptr);
MockEvent ExpectTransactionSend(const char *regular_expressions,
                                std::string *transaction_id = nullptr);
MockEvent ExpectIncomingResponse(const char *regular_expressions,
                                 std::string *transaction_id = nullptr);
MockEvent ExpectIncomingRequest(const char *regular_expressions,
                                std::string *transaction_id = nullptr);
MockEvent ExpectTransactionClose(std::string *transaction_id = nullptr);

// NetworkLayer::Delegate which checks callbacks based on static tables of
// mock events.
class StaticNetworkLayerDelegate :
  public NetworkLayer::Delegate {
 public:
  StaticNetworkLayerDelegate(DataProvider *data_provider);
  ~StaticNetworkLayerDelegate() override;

 private:
  DataProvider *data_provider_;

  void OnChannelConnected(
      const EndPoint &destination, int err) override;
  void OnChannelClosed(const EndPoint &destination) override;
  void OnIncomingRequest(
      const scoped_refptr<Request> &request) override;
  void OnIncomingResponse(
      const scoped_refptr<Response> &response) override;
  void OnTimedOut(const scoped_refptr<Request> &request) override;
  void OnTransportError(
      const scoped_refptr<Request> &request, int error) override;
};

class MockChannelAdapter {
 public:
  virtual ~MockChannelAdapter() {}

  // Whether this is a secure channel.
  virtual bool is_secure() const = 0;

  // Initialize this socket as a client socket connected to the given
  // |destination|. Returns a network error code.
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) = 0;

  // Reads data, up to |buf_len| bytes, from the socket.  The number of bytes
  // read is returned, or an error is returned upon failure.
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) = 0;

  // Writes data, up to |buf_len| bytes, to the socket.
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) = 0;
};

class UDPChannelAdapter : public MockChannelAdapter {
 public:
  UDPChannelAdapter(net::ClientSocketFactory *socket_factory,
                    net::NetLog *net_log);
  ~UDPChannelAdapter() override;

  // sippet::MockChannelAdapter methods:
  bool is_secure() const override;
  int Connect(const net::HostPortPair &destination,
              const net::CompletionCallback& callback) override;
  int Read(net::IOBuffer* buf, int buf_len,
           const net::CompletionCallback& callback) override;
  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback) override;
 private:
  net::ClientSocketFactory *socket_factory_;
  scoped_ptr<net::DatagramClientSocket> socket_;
  net::NetLog *net_log_;
};
             
class TCPChannelAdapter : public MockChannelAdapter {
 public:
  TCPChannelAdapter(net::ClientSocketFactory *socket_factory,
                    net::NetLog *net_log);
  ~TCPChannelAdapter() override;

  // sippet::MockChannelAdapter methods:
  bool is_secure() const override;
  int Connect(const net::HostPortPair &destination,
              const net::CompletionCallback& callback) override;
  int Read(net::IOBuffer* buf, int buf_len,
           const net::CompletionCallback& callback) override;
  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback) override;
 private:
  net::ClientSocketFactory *socket_factory_;
  scoped_ptr<net::StreamSocket> socket_;
  net::NetLog *net_log_;
};
             
class TLSChannelAdapter : public MockChannelAdapter {
 public:
  TLSChannelAdapter(net::ClientSocketFactory *socket_factory,
                    net::NetLog *net_log);
  ~TLSChannelAdapter() override;

  // sippet::MockChannelAdapter methods:
  bool is_secure() const override;
  int Connect(const net::HostPortPair &destination,
              const net::CompletionCallback& callback) override;
  int Read(net::IOBuffer* buf, int buf_len,
           const net::CompletionCallback& callback) override;
  int Write(net::IOBuffer* buf, int buf_len,
            const net::CompletionCallback& callback) override;
 private:
  net::ClientSocketFactory *socket_factory_;
  scoped_ptr<net::StreamSocket> tcp_socket_;
  scoped_ptr<net::SSLClientSocket> ssl_socket_;
  net::CompletionCallback connect_callback_;
  net::NetLog *net_log_;
  net::AddressList addrlist_;
  scoped_ptr<net::MockCertVerifier> cert_verifier_;
  net::SSLClientSocketContext context_;
  base::WeakPtrFactory<TLSChannelAdapter> weak_factory_;

  void OnConnected(int result);
};

class MockChannel : public Channel {
 public:
  MockChannel(MockChannelAdapter *channel_adapter,
              bool is_stream,
              Channel::Delegate *delegate,
              const EndPoint &destination);

  static const int kBufferSize;

  // sippet::Channel methods:
  int origin(EndPoint *origin) const override;
  const EndPoint& destination() const override;
  bool is_secure() const override;
  bool is_connected() const override;
  bool is_stream() const override;
  void Connect() override;
  int ReconnectIgnoringLastError() override;
  int ReconnectWithCertificate(net::X509Certificate* client_cert) override;
  int Send(const scoped_refptr<Message>& message,
           const net::CompletionCallback& callback) override;
  void Close() override;
  void CloseWithError(int error) override;
  void DetachDelegate() override;

 private:
  friend class base::RefCountedThreadSafe<MockChannel>;
  ~MockChannel() override;

  Channel::Delegate *delegate_;
  EndPoint destination_;
  EndPoint origin_;
  bool is_connected_;
  bool is_stream_;
  net::BoundNetLog net_log_;
  scoped_ptr<MockChannelAdapter> channel_adapter_;
  scoped_ptr<net::DeterministicSocketData> data_;
  scoped_refptr<net::IOBufferWithSize> io_buffer_;
  base::WeakPtrFactory<MockChannel> weak_factory_;

  void Read();
  void HandleMessage(int read_bytes);

  void OnConnected(int result);
  void OnRead(int result);
};

class MockChannelFactory : public ChannelFactory {
 public:
  MockChannelFactory(net::ClientSocketFactory *socket_factory);
  ~MockChannelFactory();

  // sippet::ChannelFactory methods:
  int CreateChannel(
          const EndPoint &destination,
          Channel::Delegate *delegate,
          scoped_refptr<Channel> *channel) override;

 private:
  net::ClientSocketFactory *socket_factory_;
  net::BoundNetLog net_log_;
};

class MockClientTransaction : public ClientTransaction {
 public:
  MockClientTransaction(DataProvider *data_provider);

  void set_id(const std::string id) {
    transaction_id_ = id;
  }
  void set_channel(const scoped_refptr<Channel> &channel) {
    channel_ = channel;
  }
  void set_delegate(TransactionDelegate *delegate) {
    delegate_ = delegate;
  }

  void Terminate() {
    delegate_->OnTransactionTerminated(transaction_id_);
  }

  // sippet::ClientTransaction methods:
  const std::string& id() const override;
  scoped_refptr<Channel> channel() const override;
  void Start(const scoped_refptr<Request> &outgoing_request) override;
  void HandleIncomingResponse(
      const scoped_refptr<Response> &response) override;
  void Close() override;

 private:
  friend class base::RefCountedThreadSafe<MockClientTransaction>;
  ~MockClientTransaction() override;

  std::string transaction_id_;
  scoped_refptr<Channel> channel_;
  TransactionDelegate *delegate_;
  DataProvider *data_provider_;
};

class MockServerTransaction : public ServerTransaction {
 public:
  MockServerTransaction(DataProvider *data_provider);

  void set_id(const std::string &id) {
    transaction_id_ = id;
  }
  void set_channel(const scoped_refptr<Channel> &channel) {
    channel_ = channel;
  }
  void set_delegate(TransactionDelegate *delegate) {
    delegate_ = delegate;
  }

  void Terminate() {
    delegate_->OnTransactionTerminated(transaction_id_);
  }

  net::TestCompletionCallback &callback() {
    return callback_;
  }

  // sippet::ServerTransaction methods:
  const std::string& id() const override;
  scoped_refptr<Channel> channel() const override;
  void Start(const scoped_refptr<Request> &incoming_request) override;
  void Send(const scoped_refptr<Response> &response) override;
  void HandleIncomingRequest(const scoped_refptr<Request> &request) override;
  void Close() override;

 private:
  friend class base::RefCountedThreadSafe<MockServerTransaction>;
  ~MockServerTransaction() override;

  std::string transaction_id_;
  scoped_refptr<Channel> channel_;
  TransactionDelegate *delegate_;
  DataProvider *data_provider_;
  net::TestCompletionCallback callback_;
};

class MockTransactionFactory : public TransactionFactory {
 public:
  MockTransactionFactory(DataProvider *data_provider);
  ~MockTransactionFactory() override;

  MockClientTransaction* client_transaction(size_t index) const;
  MockServerTransaction* server_transaction(size_t index) const;

  // sippet::TransactionFactory methods:
  ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) override;
  ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) override;
 private:
  DataProvider *data_provider_;
  std::vector<scoped_refptr<MockClientTransaction> > client_transactions_;
  std::vector<scoped_refptr<MockServerTransaction> > server_transactions_;
};

} // End of empty namespace

#endif // SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
