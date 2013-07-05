// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_LAYER_H_
#define SIPPET_TRANSPORT_NETWORK_LAYER_H_

#include "base/system_monitor/system_monitor.h"
#include "base/timer.h"
#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"
#include "net/base/completion_callback.h"
#include "net/base/client_socket_factory.h"
#include "net/base/net_log.h"
#include "sippet/message/protocol.h"
#include "sippet/message/message.h"
#include "sippet/transport/end_point.h"
#include "sippet/transport/aliases_map.h"
#include "sippet/uri/uri.h"

namespace sippet {

class Channel;
class ChannelFactory;
class Transaction;
class TransactionFactory;

// The |NetworkLayer| is the main message dispatcher of sippet. It receives
// messages from network and sends them to a delegate object, and is the
// responsible for delivering messages to network destinations. It holds the
// SIP transactions, and creates them on demand. This class is intended to be
// used single-threaded.
//
// Example usage:
//   int main() {
//     MyDelegate my_delegate;
//     DefaultTransactionFactory transaction_factory;
//     UdpChannelFactory udp_channel_factory(...);
//     TcpChannelFactory tcp_channel_factory(...);
//     NetworkLayer network_layer(&my_delegate, &transaction_factory);
//     network_layer.RegisterChannelFactory(&udp_channel_factory);
//     network_layer.RegisterChannelFactory(&tcp_channel_factory);
//     my_delegate.set_network_layer(&network_layer);
//     my_delegate.SendInitialRequest();
//     ...
//   }
//
// You must wait for the initial request to be sent to make sure the network
// layer has successfully opened a channel to your intended destination:
//   scoped_refptr<Request> MyDelegate::CreateInitialRequest() {
//     // build a REGISTER request or some other particular request of your
//     // choice, accordingly to your service provider.
//   }
//
//   void MyDelegate::SendInitialRequest(NetworkLayer &network_layer) {
//     initial_request_.reset(CreateInitialRequest());
//     service_endpoint_ = EndPoint::FromMessage(initial_request_);
//     network_layer_.Send(request.PassAs<Message>(),
//                         base::Bind(&MyDelegate::OnMessageSent, this, true));
//   }
//
//   void MyDelegate::OnMessageSent(bool is_initial_request, int err) {
//     if (is_initial_request && err == net::OK) {
//       // Now you have an established connection with the initial request
//       // service_endpoint, and you can control it's lifetime by calling:
//       network_layer.RequestChannel(service_endpoint_);
//       requested_channel_ = true;
//     }
//     else {
//       // Some error happened, and you can inform the user.
//     }
//   }
//
//   void MyDelegate::OnIncomingMessage(Message *incoming_message) {
//     if (isa<Response>(incoming_message)) {
//       // Handle the response here.
//     }
//   }
//
//   void MyDelegate::~MyDelegate() {
//     // If you have requested the channel, you can't forget to release it:
//     if (requested_channel_)
//         network_layer.ReleaseChannel(service_endpoint_);
//   }
//   
class NetworkLayer :
  public base::SystemMonitor::PowerObserver,
  public base::RefCountedThreadSafe<NetworkLayer>,
  public Channel::Delegate,
  public Transaction::Delegate; {
 public:
  class Delegate {
   public:
    // Called when one of the opened channels is closed. Normally this function
    // is called only when a stream oriented channel is closed, but it's
    // possible to be called on datagram channels when an ICMP error (such as
    // port unreachable) is detected by the network layer.
    virtual void OnChannelClosed(const EndPoint &destination, int err) = 0;

    // Called whenever a new message is received.
    virtual void OnIncomingMessage(Message *incoming_message) = 0;
  };

  // Construct a |NetworkLayer| with an existing |TransactionFactory|.
  NetworkLayer(Delegate *delegate, TransactionFactory *transaction_factory);
  virtual ~NetworkLayer();

  // Register a ChannelFactory, responsible for opening client channels.
  // Registered managers are not owned and won't be deleted on |NetworkLayer|
  // destruction.
  void RegisterChannelFactory(const Protocol &protocol,
                              ChannelFactory *factory);

  // Requests the use of a channel for a given destination. This will make the
  // channel to live longer than the individual transactions and normal
  // timeouts. It should be called after some initial transaction completion,
  // as the channels are created on demand when sending the messages. When
  // trying to request the use of a non existing channel, it will return
  // false.
  bool RequestChannel(const EndPoint &destination);

  // Called to release a channel once it is no longer needed. If the channel
  // still has an established connection, then it will be marked as idle and
  // kept for a while until an idle timeout is fired, allowing it to be reused
  // by other calls to |Send|.
  void ReleaseChannel(const EndPoint &destination);

  // Send a message (request or response) using one of the opened channels. If
  // there is no channel to the destination taken from the message, then a new
  // one will be created. If a previous request has been made, and you are
  // already sending a request to the same destination, but the request has
  // not completed yet, then you will get an error; it is required to complete
  // an initial request (such as a REGISTER) to be able to send subsequent
  // messages to the same destination.
  //
  // In case of a |Request|, a Via header is added to the top of available
  // ones, and the destination will be taken from the |Request::request_uri|.
  // In case of a |Response|, the stamped received and rport parameters on the
  // topmost Via header will be used as the destination.
  //
  // |message| the message to be sent; it could be a request or a response.
  // |destination| the destination to be used when sending the message.
  // |callback| the callback on completion of the socket Write.
  int Send(const scoped_refptr<Message> &message,
           const net::CompletionCallback& callback);

  // This function is intended to be used by the |Channel|, after receiving
  // a new incoming message. The message will be owned by the |NetworkLayer|
  // while routing the message.
  void HandleIncomingMessage(const EndPoint &destination,
                             const scoped_refptr<Message> &incoming_message);

  // Add an alias to an existing channel endpoint. It is considered an error
  // to add aliases using different protocols.
  void AddAlias(const EndPoint &destination, const EndPoint &alias);

 private:
  // base::SystemMonitor::PowerObserver methods:
  virtual void OnSuspend() OVERRIDE;
  virtual void OnResume() OVERRIDE;

  struct ChannelEntry {
    // Holds the channel instance.
    scoped_refptr<Channel> channel_;
    // Used to count number of current uses.
    int refs_;
    // Used to keep the channel opened so they can be reused.
    base::OneShotTimer<OpenedConnection> timer_;
    // Keep the first message to be sent to the channel once opened.
    scoped_refptr<Message> initial_message_;
    // Keep any transactions using this channel.
    std::list<Transaction*> transactions_;

    explicit ChannelEntry(Channel *channel)
      : channel_(channel), refs_(0) {}
  };

  typedef std::map<Protocol, ChannelFactory*, ProtocolLess> FactoriesMap;
  typedef std::map<EndPoint, ChannelEntry, EndPointLess> ChannelsMap;
  typedef std::map<std::string, Transaction*, EndPointLess> TransactionsMap;

  base::Lock critical_section_;
  AliasesMap aliases_map_;
  Delegate *delegate_;
  FactoriesMap factories_;
  ChannelsMap channels_;
  TransactionsMap transactions_;
  bool suspended_;
  net::NetLog *netlog_;

  // sippet::Channel::Delegate methods:
  virtual void OnChannelConnected(const scoped_refptr<Channel> &channel);
  virtual void OnIncomingMessage(const scoped_refptr<Message> &message);
  virtual void OnChannelClosed(const scoped_refptr<Channel> &channel,
                               int error);

  // sippet::Transaction::Delegate methods:
  virtual void OnPassMessage(const scoped_refptr<Message> &);
  virtual void OnTransactionTerminated(
      const scoped_refptr<Transaction> &transaction);

  // Timer callbacks
  void OnIdleChannelTimedOut(const EndPoint &endpoint);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_H_
