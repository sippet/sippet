// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_LAYER_H_
#define SIPPET_TRANSPORT_NETWORK_LAYER_H_

#include "base/system_monitor/system_monitor.h"
#include "base/message/protocol.h"
#include "sippet/transport/end_point.h"

namespace sippet {

class NetworkLayerDelegate;

class NetworkLayer :
  public base::SystemMonitor::PowerObserver {
 public:
  // Construct a NetworkLayer with an existing NetworkSession which
  // contains a valid ProxyService.
  explicit NetworkLayer(NetworkSession* session,
                        NetworkLayerDelegate *delegate);
  virtual ~NetworkLayer();

  // Register a TransportFactory, responsible for opening client connections
  // and listening to local addresses. Registered factories are owned pointers
  // and will be deleted on network layer destruction.
  void RegisterTransportFactory(Protocol protocol, TransportFactory *factory);

  // Opens a connection to a given remote address and protocol, using one
  // of the previously registered TransportFactory.
  int OpenConnection(const EndPoint &endpoint);

  // Starts listening on a local address, delegating to one of the existing
  // registered TransportFactory.
  int ListenTo(const EndPoint &endpoint);

  // Register a TransportConnection to be reused.
  void RegisterConnection(const EndPoint &endpoint,
                          TransportConnection *connection);

  // Remove a previously registered TransportConnection.
  void UnregisterConnection(const EndPoint &endpoint);

  // Get one of the registered connections to be used.
  scoped_refptr<TransportConnection>
    GetConnection(const EndPoint &endpoint);

  // Send a message (request or response) using one of the registered
  // TransportConnections.
  int SendMessage(scoped_refptr<Message> message);

  // Send a message (request or response) forcing the usage of a specific
  // TransportConnection.
  int SendMessage(scoped_refptr<Message> message,
                  const EndPoint &endpoint);

  // base::SystemMonitor::PowerObserver methods:
  virtual void OnSuspend() OVERRIDE;
  virtual void OnResume() OVERRIDE;

 private:
  const scoped_refptr<NetworkSession> session_;
  NetworkLayerDelegate *delegate_;
  std::map<Protocol, TransportFactory*> factories_;
  std::vector<EndPoint> listening_;
  std::map<EndPoint, scoped_refptr<TransportConnection> > connections_;
  bool suspended_;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_H_
