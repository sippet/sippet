// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
#define SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_

#include "sippet/transport/network_layer.h"
#include "sippet/transport/transaction_factory.h"
#include "sippet/transport/channel_factory.h"
#include "sippet/message/message.h"

#include "base/string_util.h"
#include "base/memory/linked_ptr.h"
#include "net/socket/socket_test_util.h"
#include "net/base/mock_cert_verifier.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

class MockEvent {
 private:
  class OnChannelClosedImpl : public NetworkLayer::Delegate {
   public:
    OnChannelClosedImpl(const EndPoint &destination);
    OnChannelClosedImpl(const EndPoint &destination, int error);
    void OnChannelClosed(const EndPoint& destination, int error) OVERRIDE;
    void OnIncomingMessage(Message *message) OVERRIDE;
   private:
    EndPoint destination_;
    bool has_error_;
    int error_;
  };

  class OnIncomingMessageImpl : public NetworkLayer::Delegate {
   public:
    OnIncomingMessageImpl(const char *regular_expressions);
    void OnChannelClosed(const EndPoint& destination, int error) OVERRIDE;
    void OnIncomingMessage(Message *message) OVERRIDE;
   private:
    const char *regular_expressions_;
  };

 public:
  MockEvent(const MockEvent &other);
  ~MockEvent();

  MockEvent &operator=(const MockEvent &other);

  void OnChannelClosed(const EndPoint& destination, int error);
  void OnIncomingMessage(Message *message);

  base::Time time_stamp() const { return time_stamp_; }
  void set_time_stamp(const base::Time & t) { time_stamp_ = t; }

private:
  friend MockEvent ExpectCloseChannel(const char *);
  friend MockEvent ExpectCloseChannel(const char *, int);
  friend MockEvent ExpectIncomingMessage(const char *);

  MockEvent(NetworkLayer::Delegate *delegate);

  linked_ptr<NetworkLayer::Delegate> delegate_;
  base::Time time_stamp_; // The time stamp at which the operation occurred.
};

MockEvent ExpectCloseChannel(const char *destination);
MockEvent ExpectCloseChannel(const char *destination, int error);
MockEvent ExpectIncomingMessage(const char *regular_expressions);

// NetworkLayer::Delegate which checks callbacks based on static tables of
// mock events.
class StaticNetworkLayerDelegate : public NetworkLayer::Delegate {
 public:
  StaticNetworkLayerDelegate();
  StaticNetworkLayerDelegate(MockEvent* events, size_t events_count);
  virtual ~StaticNetworkLayerDelegate();

  const MockEvent& PeekEvent() const;
  const MockEvent& PeekEvent(size_t index) const;

  size_t events_index() const { return events_index_; }
  size_t events_count() const { return events_count_; }

  bool at_events_end() const { return events_index_ >= events_count_; }

  MockEvent GetNextEvent();

 private:
  virtual void OnChannelClosed(const EndPoint&, int) OVERRIDE;
  virtual void OnIncomingMessage(Message*) OVERRIDE;

  MockEvent *events_;
  size_t events_index_;
  size_t events_count_;
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
              Channel::Delegate *delegate,
              const EndPoint &destination);
  virtual ~MockChannel();

  static const int kBufferSize;

  // sippet::Channel methods:
  virtual const EndPoint& origin() const OVERRIDE;
  virtual const EndPoint& destination() const OVERRIDE;
  virtual bool is_secure() const OVERRIDE;
  virtual bool is_connected() const OVERRIDE;
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

class MockTransactionEvent {
 private:
  class Expect {
   public:
    virtual ~Expect() {}
    virtual void Start(const scoped_refptr<Request>&) = 0;
    virtual void Send(const scoped_refptr<Response>&,
                      const net::CompletionCallback&) = 0;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) = 0;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) = 0;
    virtual void Close() = 0;
  };

  class ExpectStart {
   public:
    ExpectStart(const char *regular_expressions);
    virtual ~ExpectStart();
    virtual void Start(const scoped_refptr<Request>&) OVERRIDE;
    virtual void Send(const scoped_refptr<Response>&,
                      const net::CompletionCallback&) OVERRIDE;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) OVERRIDE;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) OVERRIDE;
    virtual void Close() OVERRIDE;
   private:
    const char *regular_expressions_;
  };

  class ExpectIncomingResponse {
   public:
    ExpectIncomingResponse(const char *regular_expressions);
    virtual ~ExpectIncomingResponse();
    virtual void Start(const scoped_refptr<Request>&) OVERRIDE;
    virtual void Send(const scoped_refptr<Response>&,
                      const net::CompletionCallback&) OVERRIDE;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) OVERRIDE;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) OVERRIDE;
    virtual void Close() OVERRIDE;
   private:
    const char *regular_expressions_;
  };

  class ExpectIncomingRequest {
   public:
    ExpectIncomingRequest(const char *regular_expressions);
    virtual ~ExpectIncomingRequest();
    virtual void Start(const scoped_refptr<Request>&) OVERRIDE;
    virtual void Send(const scoped_refptr<Response>&,
                      const net::CompletionCallback&) OVERRIDE;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Response>&) OVERRIDE;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) OVERRIDE;
    virtual void Close() OVERRIDE;
   private:
    const char *regular_expressions_;
  };

  class ExpectClose {
   public:
    ExpectClose();
    virtual ~ExpectClose();
    virtual void Start(const scoped_refptr<Request>&) OVERRIDE;
    virtual void Send(const scoped_refptr<Response>&,
                      const net::CompletionCallback&) OVERRIDE;
    virtual void HandleIncomingResponse(
                      const scoped_refptr<Response>&) OVERRIDE;
    virtual void HandleIncomingRequest(
                      const scoped_refptr<Request>&) OVERRIDE;
    virtual void Close() OVERRIDE;
  };

 public:
  MockTransactionEvent(const MockTransactionEvent &other);
  ~MockTransactionEvent();

  MockTransactionEvent &operator=(const MockTransactionEvent &other);

  base::Time time_stamp() const { return time_stamp_; }
  void set_time_stamp(const base::Time & t) { time_stamp_ = t; }

  void Start(const scoped_refptr<Request>& starting_request);
  void Send(const scoped_refptr<Response>& response,
            const net::CompletionCallback& callback);
  void HandleIncomingResponse(
            const scoped_refptr<Response>& incoming_response);
  void HandleIncomingRequest(
            const scoped_refptr<Request>& incoming_request);
  void Close();

 private:
  friend MockTransactionEvent ExpectStartTransaction(const char *);
  friend MockTransactionEvent ExpectTransactionSend(const char *);
  friend MockTransactionEvent ExpectIncomingResponse(const char *);
  friend MockTransactionEvent ExpectIncomingRequest(const char *);
  friend MockTransactionEvent ExpectTransactionClose();

  MockTransactionEvent(Expect *expect);

  linked_ptr<Expect> expect_;
  base::Time time_stamp_;
};

MockTransactionEvent ExpectStartTransaction(const char *regular_expressions);
MockTransactionEvent ExpectTransactionSend(const char *regular_expressions);
MockTransactionEvent ExpectIncomingResponse(const char *regular_expressions);
MockTransactionEvent ExpectIncomingRequest(const char *regular_expressions);
MockTransactionEvent ExpectTransactionClose();

class StaticTransactionData {
 public:
  StaticTransactionData(MockTransactionEvent *events,
                        size_t events_count);
  virtual ~StaticTransactionData();

  const MockTransactionEvent& PeekEvent() const;
  const MockTransactionEvent& PeekEvent(size_t index) const;

  size_t events_index() const { return events_index_; }
  size_t events_count() const { return events_count_; }

  bool at_events_end() const { return events_index_ >= events_count_; }

  MockTransactionEvent GetNextEvent();
 private:
  MockTransactionEvent *events_;
  size_t events_index_;
  size_t events_count_;
};

class MockClientTransaction : public ClientTransaction {
 public:
  MockClientTransaction(StaticTransactionData *static_data);
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

  void TimedOut();

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
  StaticTransactionData *static_data_;
};

class MockServerTransaction : public ServerTransaction {
 public:
  MockServerTransaction(StaticTransactionData *static_data);
  virtual ~MockServerTransaction();

  void set_id(const std::string id) {
    transaction_id_ = id;
  }
  void set_channel(const scoped_refptr<Channel> &channel) {
    channel_ = channel;
  }
  void set_delegate(TransactionDelegate *delegate) {
    delegate_ = delegate;
  }

  // sippet::ServerTransaction methods:
  virtual const std::string& id() const OVERRIDE;
  virtual scoped_refptr<Channel> channel() const OVERRIDE;
  virtual void Start(const scoped_refptr<Request> &incoming_request) OVERRIDE;
  virtual void Send(const scoped_refptr<Response> &response,
                    const net::CompletionCallback& callback) OVERRIDE;
  virtual void HandleIncomingRequest(
                    const scoped_refptr<Request> &request) OVERRIDE;
  virtual void Close() OVERRIDE;
 private:
  std::string transaction_id_;
  scoped_refptr<Channel> channel_;
  TransactionDelegate *delegate_;
  StaticTransactionData *static_data_;
};

class MockTransactionFactory : public TransactionFactory {
 public:
  MockTransactionFactory(StaticTransactionData *static_data);
  virtual ~MockTransactionFactory();

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
  StaticTransactionData *static_data_;
};

} // End of empty namespace

#endif // SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
