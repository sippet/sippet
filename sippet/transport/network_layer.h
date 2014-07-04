// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_LAYER_H_
#define SIPPET_TRANSPORT_NETWORK_LAYER_H_

#include <set>

#include "base/timer/timer.h"
#include "base/memory/ref_counted.h"
#include "base/system_monitor/system_monitor.h"
#include "base/gtest_prod_util.h"
#include "base/power_monitor/power_observer.h"
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
//       // Handle the response here. You can associate the incoming response
//       // with the initial request by using the Response::refer_to ID.
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
  public base::PowerObserver,
  public base::RefCountedThreadSafe<NetworkLayer>,
  public Channel::Delegate,
  public TransactionDelegate {
 private:
  DISALLOW_COPY_AND_ASSIGN(NetworkLayer);
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Called when a channel is opened.
    virtual void OnChannelConnected(const EndPoint &destination, int err) = 0;

    // Called when one of the opened channels is closed. Normally this function
    // is called only when a stream oriented channel is closed, but it's
    // possible to be called on datagram channels when an ICMP error (such as
    // port unreachable) is detected by the network layer.
    virtual void OnChannelClosed(const EndPoint &destination) = 0;

    // Called whenever a new request is received.
    virtual void OnIncomingRequest(
        const scoped_refptr<Request> &request) = 0;

    // Called whenever a new response is received. Incoming responses are
    // associated to initial requests by the |Response::refer_to| ID. The
    // same happens for generated responses passed to |NetworkLayer::Send|:
    // automatic responses carry the associated |Request::id|.
    virtual void OnIncomingResponse(
        const scoped_refptr<Response> &response) = 0;

    // Called when a timeout is detected while trying to send a request, or
    // while trying to send an INVITE error response.
    //
    // |request| the request associated with the transaction. For client
    // requests, it's the same request used in |NetworkLayer::Send|. For
    // server requests, it's the incoming request passed to
    // |NetworkLayer::Delegate::OnIncomingMessage|.
    virtual void OnTimedOut(const scoped_refptr<Request> &request) = 0;

    // Called when a network error is found while handling the requests or
    // responses.
    //
    // |request| the request associated with the transaction. For client
    // requests, it's the same request used in |NetworkLayer::Send|. For
    // server requests, it's the incoming request passed to
    // |NetworkLayer::Delegate::OnIncomingMessage|.
    // |error| the network error arised while handling the messages.
    virtual void OnTransportError(
        const scoped_refptr<Request> &request, int error) = 0;
  };

  // Construct a |NetworkLayer|.
  NetworkLayer(Delegate *delegate,
               const NetworkSettings &network_settings = NetworkSettings());
  virtual ~NetworkLayer();

  // This is the magic cookie "z9hG4bK" defined in RFC 3261
  static const char kMagicCookie[];

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

  // Forces a connection to a given destination. If there is no connected
  // channel to the given destination, then |net::ERR_IO_PENDING| is returned
  // and |NetworkLayer::Delegate::OnChannelConnected| is called when completed.
  // Otherwise, |net::OK| is just returned.
  int Connect(const EndPoint &destination);

  // Get the origin |EndPoint| of a given destination. This function returns
  // |net::OK| only if there is a channel available for that destination.
  int GetOriginOf(const EndPoint& destination, EndPoint *origin);

  // Send a message (request or response) using one of the opened channels. If
  // there is no channel to the destination taken from the message, then a new
  // one will be created. If a previous request has been made, and you are
  // already sending a request to the same destination, but the request has
  // not completed yet, then you will get an error.
  //
  // In case of a |Request|, case there is no |Via|, a topmost |Via| header is
  // added. Case the |Request| already contains a |Via| header, it will be
  // kept. The destination will be taken from the first |Route| header entry,
  // if exists, or |Request::request_uri| otherwise.
  //
  // In case of a |Response|, the parameters |ViaParam::received| and
  // |ViaParam::rport| available on the topmost |Via| header will be used as
  // the destination.
  //
  // |message| the message to be sent; it could be a request or a response.
  // |callback| the callback on completion of the socket Write.
  int Send(const scoped_refptr<Message> &message,
           const net::CompletionCallback& callback);

  // Add an alias to an existing channel endpoint. It is considered an error
  // to add aliases using different protocols. Return true if the alias has
  // been successfully created.
  bool AddAlias(const EndPoint &destination, const EndPoint &alias);

  // This function gets the end point of sending messages. For requests, it
  // uses the request-URI; for responses, use the topmost Via header.
  static EndPoint GetMessageEndPoint(const scoped_refptr<Message> &message);

 private:
  FRIEND_TEST_ALL_PREFIXES(NetworkLayerTest, StaticFunctions);

  // Just for testing purposes
  friend class NetworkLayerTest;

  // base::SystemMonitor::PowerObserver methods:
  virtual void OnSuspend() OVERRIDE;
  virtual void OnResume() OVERRIDE;

  struct ChannelContext {
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
    std::set<std::string> transactions_;

    ChannelContext() : refs_(0) {}
    explicit ChannelContext(Channel *channel,
                          const scoped_refptr<Request> &initial_request,
                          const net::CompletionCallback& initial_callback)
      : channel_(channel), refs_(0), initial_request_(initial_request),
        initial_callback_(initial_callback) {}
  };

  typedef std::map<Protocol, ChannelFactory*, ProtocolLess> FactoriesMap;
  typedef std::map<EndPoint, ChannelContext*, EndPointLess> ChannelsMap;

  typedef std::map<std::string, scoped_refptr<ClientTransaction> >
      ClientTransactionsMap;
  typedef std::map<std::string, scoped_refptr<ServerTransaction> >
      ServerTransactionsMap;

  NetworkSettings network_settings_;
  AliasesMap aliases_map_;
  Delegate *delegate_;
  FactoriesMap factories_;
  ChannelsMap channels_;
  ClientTransactionsMap client_transactions_;
  ServerTransactionsMap server_transactions_;
  base::WeakPtrFactory<NetworkLayer> weak_factory_;
  bool suspended_;

  int SendRequest(scoped_refptr<Request> &request,
      const net::CompletionCallback& callback);
  int SendResponse(const scoped_refptr<Response> &message,
      const net::CompletionCallback& callback);

  // Manage the number of channel references to start/stop the idle timeout
  void RequestChannelInternal(ChannelContext *channel_context);
  void ReleaseChannelInternal(ChannelContext *channel_context);

  // Create transactions, associating to referencing tables
  ClientTransaction *CreateClientTransaction(
      const scoped_refptr<Request> &request,
      ChannelContext *channel_context);
  ServerTransaction *CreateServerTransaction(
      const scoped_refptr<Request> &request,
      ChannelContext *channel_context);
  void DestroyClientTransaction(
      const scoped_refptr<ClientTransaction> &client_transaction);
  void DestroyServerTransaction(
      const scoped_refptr<ServerTransaction> &server_transaction);

  // Create channel contexts, associating to referencing tables
  int CreateChannelContext(
      const EndPoint &destination,
      const scoped_refptr<Request> &request,
      const net::CompletionCallback &callback,
      ChannelContext **created_channel_context);
  void DestroyChannelContext(ChannelContext *channel_context);

  // Set of utility functions used internally
  std::string CreateBranch();
  void StampClientTopmostVia(
      scoped_refptr<Request> &request,
      const scoped_refptr<Channel> &channel);
  void StampServerTopmostVia(
      scoped_refptr<Request> &request,
      const scoped_refptr<Channel> &channel);
  static std::string ClientTransactionId(
      const scoped_refptr<Request> &request);
  static std::string ClientTransactionId(
      const scoped_refptr<Response> &response);
  static std::string ServerTransactionId(
      const scoped_refptr<Request> &request);
  static std::string ServerTransactionId(
      const scoped_refptr<Response> &response);

  // Recover channel and transaction contexts from referencing tables
  ChannelContext *GetChannelContext(const EndPoint &destination);
  scoped_refptr<ClientTransaction> GetClientTransaction(
                        const scoped_refptr<Message> &message);
  scoped_refptr<ServerTransaction> GetServerTransaction(
                        const scoped_refptr<Message> &message);
  scoped_refptr<ClientTransaction> GetClientTransaction(
                        const std::string &transaction_id);
  scoped_refptr<ServerTransaction> GetServerTransaction(
                        const std::string &transaction_id);

  // Handle new incoming requests (not retransmissions). Server transactions
  // are created in advance while receiving new requests
  void HandleIncomingRequest(const scoped_refptr<Channel> &channel,
                             const scoped_refptr<Request> &request);
  
  // Handle responses not matching any of the existing client transactions.
  // Just pass the response to the delegate; it will be the case for 200 OK
  // retransmissions after the INVITE transaction has been terminated, and
  // the UAC will require to send the ACK directly.
  void HandleIncomingResponse(const scoped_refptr<Channel> &channel,
                              const scoped_refptr<Response> &response);

  // sippet::Channel::Delegate methods:
  virtual void OnChannelConnected(const scoped_refptr<Channel>&, int) OVERRIDE;
  virtual void OnIncomingMessage(const scoped_refptr<Channel> &,
                                 const scoped_refptr<Message> &) OVERRIDE;
  virtual void OnChannelClosed(const scoped_refptr<Channel> &, int) OVERRIDE;

  // sippet::TransactionDelegate methods:
  virtual void OnIncomingResponse(const scoped_refptr<Response> &) OVERRIDE;
  virtual void OnTimedOut(const scoped_refptr<Request> &request) OVERRIDE;
  virtual void OnTransportError(
      const scoped_refptr<Request> &request, int error) OVERRIDE;
  virtual void OnTransactionTerminated(const std::string &) OVERRIDE;

  // Timer callbacks
  void OnIdleChannelTimedOut(const EndPoint &endpoint);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_H_
