// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_LAYER_H_
#define SIPPET_TRANSPORT_NETWORK_LAYER_H_

#include <list>
#include "base/timer.h"
#include "base/memory/ref_counted.h"
#include "base/system_monitor/system_monitor.h"
#include "net/base/completion_callback.h"
#include "sippet/message/protocol.h"
#include "sippet/message/message.h"
#include "sippet/transport/channel.h"
#include "sippet/transport/end_point.h"
#include "sippet/transport/client_transaction.h"
#include "sippet/transport/server_transaction.h"
#include "sippet/transport/transaction_delegate.h"
#include "sippet/transport/aliases_map.h"
#include "sippet/transport/network_settings.h"
#include "sippet/uri/uri.h"

namespace sippet {

class ChannelFactory;
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
  public TransactionDelegate {
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
  NetworkLayer(Delegate *delegate, TransactionFactory *transaction_factory,
               const NetworkSettings &network_settings = NetworkSettings());
  virtual ~NetworkLayer();

  // Register a ChannelFactory, responsible for opening client channels.
  // Registered managers are not owned and won't be deleted on |NetworkLayer|
  // destruction.
  void RegisterChannelFactory(const Protocol &protocol,
                              ChannelFactory *channel_factory);

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

  // Add an alias to an existing channel endpoint. It is considered an error
  // to add aliases using different protocols. Return true if the alias has
  // been successfully created.
  bool AddAlias(const EndPoint &destination, const EndPoint &alias);

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
    base::OneShotTimer<NetworkLayer> timer_;
    // Keep the request used to open the channel.
    scoped_refptr<Request> initial_request_;
    // Keep the first callback to be called after connected and sent.
    net::CompletionCallback initial_callback_;
    // Keep references to transactions using this channel.
    std::list<std::string> transactions_;

    ChannelEntry() : refs_(0) {}
    explicit ChannelEntry(Channel *channel,
                          const scoped_refptr<Request> &initial_request,
                          const net::CompletionCallback& initial_callback)
      : channel_(channel), refs_(0), initial_request_(initial_request),
        initial_callback_(initial_callback) {}
  };

  struct ClientTransactionEntry {
    // Points to the transaction.
    scoped_refptr<ClientTransaction> transaction_;
    // Points to the using channel instance.
    Channel* channel_;

    explicit ClientTransactionEntry(ClientTransaction* transaction,
                                    Channel* channel)
      : transaction_(transaction), channel_(channel) {}
  };

  struct ServerTransactionEntry {
    // Points to the transaction.
    scoped_refptr<ServerTransaction> transaction_;
    // Points to the using channel instance.
    Channel* channel_;

    explicit ServerTransactionEntry(ServerTransaction* transaction,
                                    Channel* channel)
      : transaction_(transaction), channel_(channel) {}
  };

  typedef std::map<Protocol, ChannelFactory*, ProtocolLess> FactoriesMap;
  typedef std::map<EndPoint, ChannelEntry*, EndPointLess> ChannelsMap;

  typedef std::map<std::string, ClientTransactionEntry*>
    ClientTransactionsMap;
  typedef std::map<std::string, ServerTransactionEntry*>
    ServerTransactionsMap;

  TransactionFactory *transaction_factory_;
  NetworkSettings network_settings_;
  AliasesMap aliases_map_;
  Delegate *delegate_;
  FactoriesMap factories_;
  ChannelsMap channels_;
  ClientTransactionsMap client_transactions_;
  ServerTransactionsMap server_transactions_;
  bool suspended_;

  static const char kMagicCookie[];

  void RequestChannelInternal(ChannelEntry *entry);
  void ReleaseChannelInternal(ChannelEntry *entry);
  int CreateChannel(const EndPoint &destination,
                    scoped_refptr<Channel> *channel);
  void CreateClientTransaction(const scoped_refptr<Request> &request,
                               ChannelEntry *channel_entry);
  void CreateServerTransaction(const scoped_refptr<Request> &request,
                               ChannelEntry *channel_entry);
  static std::string CreateBranch();
  void StampClientTopmostVia(scoped_refptr<Request> &request,
                             const scoped_refptr<Channel> &channel);
  void StampServerTopmostVia(scoped_refptr<Request> &request,
                             const scoped_refptr<Channel> &channel);
  static std::string ClientTransactionId(
                        const scoped_refptr<Request> &request);
  static std::string ClientTransactionId(
                        const scoped_refptr<Response> &response);
  static std::string ServerTransactionId(
                        const scoped_refptr<Request> &request);
  static std::string ServerTransactionId(
                        const scoped_refptr<Response> &response);

  // sippet::Channel::Delegate methods:
  virtual void OnChannelConnected(const scoped_refptr<Channel> &channel);
  virtual void OnIncomingMessage(const scoped_refptr<Message> &message);
  virtual void OnChannelClosed(const scoped_refptr<Channel> &channel,
                               int error);

  // sippet::TransactionDelegate methods:
  virtual void OnPassMessage(const scoped_refptr<Message> &);
  virtual void OnTransactionTerminated(const std::string &transaction_id);

  // Timer callbacks
  void OnIdleChannelTimedOut(const EndPoint &endpoint);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_H_
