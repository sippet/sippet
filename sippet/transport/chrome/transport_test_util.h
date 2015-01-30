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
  virtual ~MockBranchFactory();
  virtual std::string CreateBranch() override;
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
    virtual ~Expect() {}
    virtual void Start(const scoped_refptr<Request>&) = 0;
    virtual void Send(const scoped_refptr<Response>&) = 0;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) = 0;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) = 0;
    virtual void Close() = 0;
  };

  MockEvent(Expect *expect, std::string *transaction_id = NULL);
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
  virtual ~StaticDataProvider();

  virtual MockEvent& PeekEvent() override;
  virtual MockEvent& PeekEvent(size_t index) override;

  virtual size_t events_index() const override;
  virtual size_t events_count() const override;

  virtual bool at_events_end() const override;

  virtual MockEvent &GetNextEvent() override;

  virtual void set_transaction_id(const std::string &transaction_id) override;

  virtual void OnChannelConnected(
          const EndPoint& destination, int error) override;
  virtual void OnChannelClosed(
          const EndPoint& destination) override;
  virtual void OnIncomingMessage(Message *message) override;
  virtual void Start(
          const scoped_refptr<Request>& starting_request) override;
  virtual void Send(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) override;
  virtual void HandleIncomingResponse(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) override;
  virtual void HandleIncomingRequest(
          const std::string &transaction_id,
          const scoped_refptr<Request>& request) override;
  virtual void Close(const std::string &transaction_id) override;

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
                                 std::string *transaction_id = NULL);
MockEvent ExpectTransactionSend(const char *regular_expressions,
                                std::string *transaction_id = NULL);
MockEvent ExpectIncomingResponse(const char *regular_expressions,
                                 std::string *transaction_id = NULL);
MockEvent ExpectIncomingRequest(const char *regular_expressions,
                                std::string *transaction_id = NULL);
MockEvent ExpectTransactionClose(std::string *transaction_id = NULL);

// NetworkLayer::Delegate which checks callbacks based on static tables of
// mock events.
class StaticNetworkLayerDelegate :
  public NetworkLayer::Delegate {
 public:
  StaticNetworkLayerDelegate(DataProvider *data_provider);
  virtual ~StaticNetworkLayerDelegate();

 private:
  DataProvider *data_provider_;

  virtual void OnChannelConnected(
      const EndPoint &destination, int err) override;
  virtual void OnChannelClosed(const EndPoint &destination) override;
  virtual void OnIncomingRequest(
      const scoped_refptr<Request> &request) override;
  virtual void OnIncomingResponse(
      const scoped_refptr<Response> &response) override;
  virtual void OnTimedOut(const scoped_refptr<Request> &request) override;
  virtual void OnTransportError(
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
  virtual ~UDPChannelAdapter();

  // sippet::MockChannelAdapter methods:
  virtual bool is_secure() const override;
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) override;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) override;
  virtual int Write(net::IOBuffer* buf, int buf_len,
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
  virtual ~TCPChannelAdapter();

  // sippet::MockChannelAdapter methods:
  virtual bool is_secure() const override;
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) override;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) override;
  virtual int Write(net::IOBuffer* buf, int buf_len,
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
  virtual ~TLSChannelAdapter();

  // sippet::MockChannelAdapter methods:
  virtual bool is_secure() const override;
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) override;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) override;
  virtual int Write(net::IOBuffer* buf, int buf_len,
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
  virtual int origin(EndPoint *origin) const override;
  virtual const EndPoint& destination() const override;
  virtual bool is_secure() const override;
  virtual bool is_connected() const override;
  virtual bool is_stream() const override;
  virtual void Connect() override;
  virtual int ReconnectIgnoringLastError() override;
  virtual int ReconnectWithCertificate(
      net::X509Certificate* client_cert) override;
  virtual int Send(const scoped_refptr<Message>& message,
                   const net::CompletionCallback& callback) override;
  virtual void Close() override;
  virtual void CloseWithError(int error) override;
  virtual void DetachDelegate() override;

 private:
  friend class base::RefCountedThreadSafe<MockChannel>;
  virtual ~MockChannel();

  Channel::Delegate *delegate_;
  EndPoint destination_;
  EndPoint origin_;
  bool is_connected_;
  bool is_stream_;
  net::BoundNetLog net_log_;
  scoped_ptr<MockChannelAdapter> channel_adapter_;
  scoped_ptr<net::DeterministicSocketData> data_;
  base::WeakPtrFactory<MockChannel> weak_factory_;
  scoped_refptr<net::IOBufferWithSize> io_buffer_;

  void Read();
  void HandleMessage(int read_bytes);

  void OnConnected(int result);
  void OnRead(int result);
};

class MockChannelFactory : public ChannelFactory {
 public:
  MockChannelFactory(net::ClientSocketFactory *socket_factory);
  virtual ~MockChannelFactory();

  // sippet::ChannelFactory methods:
  virtual int CreateChannel(
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
  virtual const std::string& id() const override;
  virtual scoped_refptr<Channel> channel() const override;
  virtual void Start(const scoped_refptr<Request> &outgoing_request) override;
  virtual void HandleIncomingResponse(
                    const scoped_refptr<Response> &response) override;
  virtual void Close() override;

 private:
  friend class base::RefCountedThreadSafe<MockClientTransaction>;
  virtual ~MockClientTransaction();

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
  virtual const std::string& id() const override;
  virtual scoped_refptr<Channel> channel() const override;
  virtual void Start(const scoped_refptr<Request> &incoming_request) override;
  virtual void Send(const scoped_refptr<Response> &response) override;
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request> &request) override;
  virtual void Close() override;

 private:
  friend class base::RefCountedThreadSafe<MockServerTransaction>;
  virtual ~MockServerTransaction();

  std::string transaction_id_;
  scoped_refptr<Channel> channel_;
  TransactionDelegate *delegate_;
  DataProvider *data_provider_;
  net::TestCompletionCallback callback_;
};

class MockTransactionFactory : public TransactionFactory {
 public:
  MockTransactionFactory(DataProvider *data_provider);
  virtual ~MockTransactionFactory();

  MockClientTransaction* client_transaction(size_t index) const;
  MockServerTransaction* server_transaction(size_t index) const;

  // sippet::TransactionFactory methods:
  virtual ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TimeDeltaFactory *time_delta_factory,
      TransactionDelegate *delegate) override;
  virtual ServerTransaction *CreateServerTransaction(
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
