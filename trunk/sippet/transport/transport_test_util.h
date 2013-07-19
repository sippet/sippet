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

#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

class MockEvent {
 private:
  class OnChannelClosedImpl : public NetworkLayer::Delegate {
   public:
    OnChannelClosedImpl(const EndPoint &destination) : has_error_(false),
        destination_(destination) {}

    OnChannelClosedImpl(const EndPoint &destination, int error) :
        has_error_(true), error_(error), destination_(destination) {}

    void OnChannelClosed(const EndPoint& destination, int error) {
      EXPECT_EQ(destination_, destination);
      if (has_error_)
        EXPECT_EQ(error_, error);
    }

    void OnIncomingMessage(Message *message) {
      EXPECT_TRUE(false) << "Not expected to get an incoming message this time";
    }

   private:
    EndPoint destination_;
    bool has_error_;
    int error_;
  };

  class OnIncomingMessageImpl : public NetworkLayer::Delegate {
   public:
    OnIncomingMessageImpl(const char *regular_expressions) :
        regular_expressions_(regular_expressions) {}

    void OnChannelClosed(const EndPoint& destination, int error) {
      EXPECT_TRUE(false) << "Not expected to get a network error this time";
    }

    void OnIncomingMessage(Message *message) {
      // TODO: break down the regular expressions and check the message
    }

   private:
    const char *regular_expressions_;
  };

 public:
  MockEvent(const MockEvent &other) : delegate_(other.delegate_),
      time_stamp_(other.time_stamp_) {}
  ~MockEvent() {}

  MockEvent &operator=(const MockEvent &other) {
    delegate_ = other.delegate_;
    time_stamp_ = other.time_stamp_;
    return *this;
  }

  static MockEvent ChannelClosed(const EndPoint &destination) {
    return MockEvent(new MockEvent::OnChannelClosedImpl(destination));
  }
  static MockEvent ChannelClosed(const EndPoint &destination, int error) {
    return MockEvent(new MockEvent::OnChannelClosedImpl(destination, error));
  }
  static MockEvent ChannelClosed(const char *destination) {
    EndPoint endpoint(EndPoint::FromString(destination));
    return MockEvent(new MockEvent::OnChannelClosedImpl(endpoint));
  }
  static MockEvent ChannelClosed(const char *destination, int error) {
    EndPoint endpoint(EndPoint::FromString(destination));
    return MockEvent(new MockEvent::OnChannelClosedImpl(endpoint, error));
  }
  static MockEvent IncomingMessage(const char *regular_expressions) {
    return MockEvent(new MockEvent::OnIncomingMessageImpl(regular_expressions));
  }

  void OnChannelClosed(const EndPoint& destination, int error) {
    delegate_->OnChannelClosed(destination, error);
  }

  void OnIncomingMessage(Message *message) {
    delegate_->OnIncomingMessage(message);
  }

  base::Time time_stamp() const { return time_stamp_; }
  void set_time_stamp(const base::Time & t) { time_stamp_ = t; }

private:
  MockEvent(NetworkLayer::Delegate *delegate) :
    delegate_(delegate) {}

  linked_ptr<NetworkLayer::Delegate> delegate_;
  base::Time time_stamp_; // The time stamp at which the operation occurred.
};

// NetworkLayer::Delegate which checks callbacks based on static tables of
// mock events.
class StaticNetworkLayerDelegate : public NetworkLayer::Delegate {
 public:
  StaticNetworkLayerDelegate() : events_(NULL), events_count_(0),
      events_index_(0) {}

  StaticNetworkLayerDelegate(MockEvent* events, size_t events_count) :
      events_(events), events_count_(events_count), events_index_(0) {}

  virtual ~StaticNetworkLayerDelegate() {}

  const MockEvent& PeekEvent() const { return PeekEvent(events_index_); }
  const MockEvent& PeekEvent(size_t index) const {
    DCHECK_LT(index, events_count_);
    return events_[index];
  }

  size_t events_index() const { return events_index_; }
  size_t events_count() const { return events_count_; }

  bool at_events_end() const { return events_index_ >= events_count_; }

  MockEvent GetNextEvent() {
    DCHECK(!at_events_end());
    events_[events_index_].set_time_stamp(base::Time::Now());
    return events_[events_index_++];
  }

 private:
  virtual void OnChannelClosed(const EndPoint& destination, int error) {
    DCHECK(!at_events_end());
    GetNextEvent().OnChannelClosed(destination, error);
  }

  virtual void OnIncomingMessage(Message *message) {
    DCHECK(!at_events_end());
    GetNextEvent().OnIncomingMessage(message);
  }

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
  UDPChannelAdapter(net::MockClientSocketFactory *socket_factory)
    : socket_factory_(socket_factory) {}

  virtual bool is_secure() const OVERRIDE { return false; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE {
    return net::OK;
  }
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Read(buf, buf_len, callback);
  }
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Write(buf, buf_len, callback);
  }
 private:
  net::MockClientSocketFactory *socket_factory_;
  net::DatagramClientSocket *socket_;
};
             
class TCPChannelAdapter : public MockChannelAdapter {
 public:
  TCPChannelAdapter(net::MockClientSocketFactory *socket_factory)
    : socket_factory_(socket_factory) {}

  virtual bool is_secure() const OVERRIDE { return false; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE {
    return net::OK;
  }
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Read(buf, buf_len, callback);
  }
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Write(buf, buf_len, callback);
  }
 private:
  net::MockClientSocketFactory *socket_factory_;
  net::StreamSocket *socket_;
};
             
class TLSChannelAdapter : public MockChannelAdapter {
 public:
  TLSChannelAdapter(net::MockClientSocketFactory *socket_factory)
    : socket_factory_(socket_factory) {}

  virtual bool is_secure() const OVERRIDE { return true; }
  virtual int Connect(const net::HostPortPair &destination,
                      const net::CompletionCallback& callback) OVERRIDE {
    return net::OK;
  }
  virtual int Read(net::IOBuffer* buf, int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Read(buf, buf_len, callback);
  }
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE {
    return socket_->Write(buf, buf_len, callback);
  }
 private:
  net::MockClientSocketFactory *socket_factory_;
  net::SSLClientSocket *socket_;
};

class MockChannel : public Channel {
 public:
  template<class Adapter>
  static MockChannel *CreateChannel(
                                net::MockClientSocketFactory *socket_factory,
                                Channel::Delegate *delegate,
                                const EndPoint &destination) {
    return new MockChannel(new Adapter(socket_factory),
                           delegate, destination);
  }

  virtual ~MockChannel() {}

  virtual const EndPoint& origin() const OVERRIDE {
    return origin_;
  }
  virtual const EndPoint& destination() const OVERRIDE {
    return destination_;
  }
  virtual bool is_secure() const OVERRIDE {
    return channel_adapter_->is_secure();
  }
  virtual bool is_connected() const OVERRIDE {
    return is_connected_;
  }

  virtual void Connect() OVERRIDE {
    DCHECK(!is_connected());
    channel_adapter_->Connect(destination_.hostport(),
      base::Bind(&MockChannel::OnConnected, this));
  }

  virtual int Send(const scoped_refptr<Message>& message,
                   const net::CompletionCallback& callback) OVERRIDE {
    DCHECK(is_connected());
    std::string buffer(message->ToString());
    scoped_refptr<net::IOBuffer> io_buffer(new net::IOBuffer(buffer.size()));
    memcpy(io_buffer->data(), buffer.data(), buffer.size());
    return channel_adapter_->Write(io_buffer, buffer.size(), callback);
  }

  virtual void Close() OVERRIDE {
    DCHECK(is_connected());
  }
  virtual void CloseWithError(int error) OVERRIDE {
    DCHECK(is_connected());
  }
  virtual void DetachDelegate() OVERRIDE {
    delegate_ = 0;
  }

 private:
  MockChannel(MockChannelAdapter *channel_adapter,
              Channel::Delegate *delegate,
              const EndPoint &destination)
    : channel_adapter_(channel_adapter),
      delegate_(delegate),
      destination_(destination),
      origin_(net::HostPortPair("192.0.2.33", 123),
              destination.protocol()) {}

  Channel::Delegate *delegate_;
  EndPoint destination_;
  EndPoint origin_;
  bool is_connected_;
  net::BoundNetLog net_log_;
  scoped_ptr<MockChannelAdapter> channel_adapter_;
  scoped_ptr<net::DeterministicSocketData> data_;

  void OnConnected(int result) {
    is_connected_ = true;
    if (delegate_)
      delegate_->OnChannelConnected(this, result);
    // TODO: start reading
  }
};

class MockChannelFactory : public ChannelFactory {
 public:
  MockChannelFactory(net::MockClientSocketFactory *socket_factory) :
      socket_factory_(socket_factory) {
    DCHECK(socket_factory);
  }
  virtual ~MockChannelFactory() {}

  virtual int CreateChannel(
          const EndPoint &destination,
          Channel::Delegate *delegate,
          scoped_refptr<Channel> *channel) {
    if (destination.protocol() == Protocol::UDP) {
      *channel = MockChannel::CreateChannel<UDPChannelAdapter>(
                    socket_factory_, delegate, destination);
      return net::OK;
    }
    else if (destination.protocol() == Protocol::TCP) {
      *channel = MockChannel::CreateChannel<TCPChannelAdapter>(
                    socket_factory_, delegate, destination);
      return net::OK;
    }
    else if (destination.protocol() == Protocol::TLS) {
      *channel = MockChannel::CreateChannel<TLSChannelAdapter>(
                    socket_factory_, delegate, destination);
      return net::OK;
    }
    NOTREACHED();
    return net::ERR_UNEXPECTED;
  }

 private:
  net::MockClientSocketFactory *socket_factory_;
};

class TransactionFactoryImpl : public TransactionFactory {
 public:
  virtual ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
    return 0;
  }

  virtual ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
    return 0;
  }
};

} // End of empty namespace

#endif // SIPPET_TRANSPORT_TRANSPORT_TEST_UTIL_H_
