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

namespace sippet {

class Connection;
class ConnectionFactory;


class NetworkLayer :
  public base::SystemMonitor::PowerObserver,
  public base::RefCountedThreadSafe<NetworkLayer> {
 public:
  class Delegate {
   public:
    virtual void OnIncomingMessage(Message *incoming_message) = 0;
  };

  // Construct a NetworkLayer with an existing NetworkSession which
  // contains a valid ProxyService.
  NetworkLayer(
    net::ClientSocketFactory* client_socket_factory,
    const net::SSLConfig &ssl_config,
    const scoped_refptr<net::URLRequestContextGetter>& request_context_getter);
  virtual ~NetworkLayer();

  // Register a ConnectionFactory, responsible for opening client connections
  // and listening to local addresses. Registered managers are not owned
  // and won't be deleted on network layer destruction.
  void RegisterConnectionFactory(const Atom<Protocol> &protocol,
                                 ConnectionFactory *factory);

  // Starts listening on a local address, delegating to one of the existing
  // registered ConnectionFactory.  Returns a network error code.
  int Listen(const EndPoint &local);

  // Stops listening on a local address.
  void StopListen(const EndPoint &local);

  // Requests a connection for an endpoint.
  //
  // There are five possible results from calling this function:
  // 1) RequestConnection returns OK, reusing an existing connection.
  // 2) RequestConnection returns OK with a newly created connection.
  // 3) RequestConnection returns ERR_IO_PENDING.  The connection request
  // will be added to a wait list until a connection is available to reuse
  // or a new connection finishes connecting.
  // 4) An error occurred early on, so RequestConnection returns an error
  // code.
  // 5) A recoverable error occurred while setting up the connection.  An
  // error code is returned.
  //
  // If ERR_IO_PENDING is returned, then the callback will be used to notify the
  // client of completion.
  int RequestConnection(const EndPoint &destination,
                        const net::CompletionCallback &callback);

  // Called to release a connection once it is no longer needed.  If the
  // connection still has an established connection, then it will be added
  // to the set of idle connections to be used to satisfy future
  // |RequestConnection| calls. Otherwise, if there's no more pending callbacks,
  // the connection is destroyed.
  void ReleaseConnection(const EndPoint &destination,
                         const net::CompletionCallback &callback);

  // Send a message (request or response) using a requested connection. If
  // there is no connection to the given |destination|, or if it is not
  // available yet (still pending), then |ERR_FAILED| will be issued.  In
  // case of a |Request|, the topmost Via 'protocol' and 'sent-by' fields are
  // overwritten by the network layer; any available via parameters are
  // preserved.  If there's no Via in a |Request|, |ERR_INVALID_ARGUMENT| is
  // returned. In case of a |Response|, the indicated |destination| will be
  // stamped in the topmost Via as the 'received' and 'rport' parameters.
  // |message| the message to be sent; it could be a request or a response.
  // |destination| the destination to be used when sending the message.
  // |callback| the callback on completion of the socket Write.
  int Send(scoped_refptr<Message> message,
           const EndPoint &destination,
           const net::CompletionCallback& callback);

  // This function is intended to be used by the |ConnectionFactory|, after
  // accepting and creating a new incoming connection.
  void HandleIncomingConnection(const EndPoint &destination,
                                Connection *incoming_connection);

  // This function is intended to be used by the |Connection|, after receiving
  // a new incoming message.
  void HandleIncomingMessage(const EndPoint &destination,
                             Message *incoming_message);

  // base::SystemMonitor::PowerObserver methods:
  virtual void OnSuspend() OVERRIDE;
  virtual void OnResume() OVERRIDE;

 private:
  struct PendingConnection {
    // Holds the connection instance
    scoped_refptr<Connection> connection_;
    // Used to join connect requests
    std::vector<net::CompletionCallback> callbacks_;

    PendingConnection(Connection *connection,
                      const net::CompletionCallback &callback)
      : connection_(connection) { callbacks_.push_back(callback); }
  };

  struct OpenedConnection {
    // Holds the connection instance
    scoped_refptr<Connection> connection_;
    // Used to count number of current uses
    int refs_;
    // Used to keep connection opened so they can be reused.
    base::OneShotTimer<OpenedConnection> timer_;

    explicit OpenedConnection(Connection *connection)
      : connection_(connection), refs_(0) {}
  };

  typedef std::map<Atom<Protocol>, ConnectionFactory*, AtomLess<Protocol> >
    FactoriesMap;
  typedef std::map<EndPoint, scoped_refptr<Channel>, EndPointLess>
    ChannelsMap;
  typedef std::map<EndPoint, PendingConnection, EndPointLess>
    PendingConnectionsMap;

  base::Lock critical_section_;
  AliasesMap aliases_map_;
  Delegate *delegate_;
  FactoriesMap factories_;
  std::vector<EndPoint> listening_;
  ChannelsMap connections_;
  PendingConnectionsMap pending_connections_;
  bool suspended_;
  net::NetLog *netlog_;

  void OnConnectionTimedOut(const EndPoint &endpoint);
  void OnConnectionOpened(const EndPoint &endpoint, int err);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_H_
