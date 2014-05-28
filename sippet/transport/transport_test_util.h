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
  virtual std::string CreateBranch() OVERRIDE;
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

  MockEvent(Expect *expect, std::string *transaction_id = NULL)
    : expect_(expect), transaction_id_(transaction_id) {}
  MockEvent(const MockEvent &other)
    : expect_(other.expect_), time_stamp_(other.time_stamp_),
      transaction_id_(other.transaction_id_) {}
  ~MockEvent() {}

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
  void OnChannelClosed(const EndPoint& destination, int error) {
    expect_->OnChannelClosed(destination, error);
  }
  void OnIncomingMessage(Message *message) {
    expect_->OnIncomingMessage(message);
  }
  void OnTimedOut(const std::string &id) {
    expect_->OnTimedOut(id);
  }
  void OnTransportError(const std::string &id, int error) {
    expect_->OnTransportError(id, error);
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

  virtual void OnChannelClosed(const EndPoint& destination, int error) = 0;
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
  StaticDataProvider()
    : events_(NULL), events_count_(0), events_index_(0) {}
  StaticDataProvider(MockEvent *events,
                     size_t events_count)
    : events_(events), events_count_(events_count), events_index_(0) {}
  virtual ~StaticDataProvider() {}

  virtual MockEvent& PeekEvent() OVERRIDE {
    return PeekEvent(events_index_);
  }
  virtual MockEvent& PeekEvent(size_t index) OVERRIDE {
    DCHECK_LT(index, events_count_);
    return events_[index];
  }

  virtual size_t events_index() const OVERRIDE {
    return events_index_;
  }
  virtual size_t events_count() const OVERRIDE {
    return events_count_;
  }

  virtual bool at_events_end() const OVERRIDE {
    return events_index_ >= events_count_;
  }

  virtual MockEvent &GetNextEvent() OVERRIDE {
    DCHECK(!at_events_end());
    events_[events_index_].set_time_stamp(base::Time::Now());
    return events_[events_index_++];
  }

  virtual void set_transaction_id(const std::string &transaction_id) {
    PeekEvent().set_transaction_id(transaction_id);
  }

  virtual void OnChannelClosed(
          const EndPoint& destination, int error) OVERRIDE {
    GetNextEvent().OnChannelClosed(destination, error);
  }
  virtual void OnIncomingMessage(Message *message) OVERRIDE {
    GetNextEvent().OnIncomingMessage(message);
  }
  virtual void Start(
          const scoped_refptr<Request>& starting_request) OVERRIDE {
    GetNextEvent().Start(starting_request);
  }
  virtual void Send(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) OVERRIDE {
    if (!PeekEvent().transaction_id().empty())
      EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
    GetNextEvent().Send(response);
  }
  virtual void HandleIncomingResponse(
          const std::string &transaction_id,
          const scoped_refptr<Response>& response) OVERRIDE {
    if (!PeekEvent().transaction_id().empty())
      EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
    GetNextEvent().HandleIncomingResponse(response);
  }
  virtual void HandleIncomingRequest(
          const std::string &transaction_id,
          const scoped_refptr<Request>& request) OVERRIDE {
    if (!PeekEvent().transaction_id().empty())
      EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
    GetNextEvent().HandleIncomingRequest(request);
  }
  virtual void Close(const std::string &transaction_id) OVERRIDE {
    if (!PeekEvent().transaction_id().empty())
      EXPECT_EQ(PeekEvent().transaction_id(), transaction_id);
    GetNextEvent().Close();
  }

 private:
  MockEvent *events_;
  size_t events_index_;
  size_t events_count_;
};

// NetworkLayer::Delegate events:
MockEvent ExpectCloseChannel(const char *destination);
MockEvent ExpectCloseChannel(const char *destination, int error);
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

  virtual void OnChannelClosed(const EndPoint&, int) OVERRIDE;
  virtual void OnIncomingMessage(Message*) OVERRIDE;
  virtual void OnTimedOut(const std::string &id) OVERRIDE;
  virtual void OnTransportError(const std::string &id, int error) OVERRIDE;
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
  virtual bool is_secure() const OVERRIDE { return false; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE;
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
  virtual bool is_secure() const OVERRIDE { return false; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE;
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
  virtual bool is_secure() const OVERRIDE { return false; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE;
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE;
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
  virtual ~MockChannel();

  static const int kBufferSize;

  // sippet::Channel methods:
  virtual const EndPoint& origin() const OVERRIDE;
  virtual const EndPoint& destination() const OVERRIDE;
  virtual bool is_secure() const OVERRIDE;
  virtual bool is_connected() const OVERRIDE;
  virtual bool is_stream() const OVERRIDE;
  virtual void Connect() OVERRIDE;
  virtual int Send(const scoped_refptr<Message>& message,
                   const net::CompletionCallback& callback) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void CloseWithError(int error) OVERRIDE;
  virtual void DetachDelegate() OVERRIDE;

 private:
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
          scoped_refptr<Channel> *channel) OVERRIDE;

 private:
  net::ClientSocketFactory *socket_factory_;
  net::BoundNetLog net_log_;
};

class MockClientTransaction : public ClientTransaction {
 public:
  MockClientTransaction(DataProvider *data_provider);
  virtual ~MockClientTransaction();

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
  virtual const std::string& id() const OVERRIDE;
  virtual scoped_refptr<Channel> channel() const OVERRIDE;
  virtual void Start(const scoped_refptr<Request> &outgoing_request) OVERRIDE;
  virtual void HandleIncomingResponse(
                    const scoped_refptr<Response> &response) OVERRIDE;
  virtual void Close() OVERRIDE;
 private:
  std::string transaction_id_;
  scoped_refptr<Channel> channel_;
  TransactionDelegate *delegate_;
  DataProvider *data_provider_;
};

class MockServerTransaction : public ServerTransaction {
 public:
  MockServerTransaction(DataProvider *data_provider);
  virtual ~MockServerTransaction();

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
  virtual const std::string& id() const OVERRIDE;
  virtual scoped_refptr<Channel> channel() const OVERRIDE;
  virtual void Start(const scoped_refptr<Request> &incoming_request) OVERRIDE;
  virtual void Send(const scoped_refptr<Response> &response) OVERRIDE;
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request> &request) OVERRIDE;
  virtual void Close() OVERRIDE;
 private:
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
      TransactionDelegate *delegate) OVERRIDE;
  virtual ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) OVERRIDE;
 private:
  DataProvider *data_provider_;
  std::vector<scoped_refptr<MockClientTransaction> > client_transactions_;
  std::vector<scoped_refptr<MockServerTransaction> > server_transactions_;
};

} // End of empty namespace

#endif // SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
