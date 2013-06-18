// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_layer.h"

#include "base/stl_util.h"
#include "net/base/net_errors.h"
#include "sippet/transport/connection.h"
#include "sippet/transport/connection_factory.h"
#include "sippet/message/headers/via.h"

namespace sippet {

const int NetworkLayer::kReuseDelay = 60;

NetworkLayer::NetworkLayer(Delegate *delegate,
                           net::NetLog *netlog)
  : netlog_(netlog), delegate_(delegate) {
  DCHECK(delegate);
}

NetworkLayer::~NetworkLayer() {
  STLDeleteContainerPairSecondPointers(
    connections_.begin(), connections_.end());
}

void NetworkLayer::RegisterConnectionFactory(const Atom<Protocol> &protocol,
                                             ConnectionFactory *manager) {
  DCHECK(manager);
  factories_.insert(std::make_pair(protocol, manager));
}

int NetworkLayer::Listen(const EndPoint &endpoint) {
  base::AutoLock lock(critical_section_);

  FactoriesMap::iterator i = factories_.find(endpoint.protocol());
  if (i == factories_.end())
    return net::ERR_NOT_IMPLEMENTED;

  int res = i->second->Listen(endpoint);

  // Keep track of listening sockets (for power monitor)
  if (res == net::OK)
    listening_.push_back(endpoint);

  return res;
}

void NetworkLayer::StopListen(const EndPoint &local) {
  base::AutoLock lock(critical_section_);

  // Find appropriated factory
  FactoriesMap::iterator i = factories_.find(local.protocol());
  if (i == factories_.end()) {
    DVLOG(1) << "impossible to stop listening on "
             << local.host() << ":" << local.port();
    return;
  }

  std::vector<EndPoint>::iterator j = std::find_if(
    listening_.begin(), listening_.end(),
    std::bind1st(std::equal_to<EndPoint>(), local));

  if (j != listening_.end()) {
    i->second->StopListen(local);
    listening_.erase(j);
  }
}

int NetworkLayer::RequestConnection(const EndPoint &destination,
                                    const net::CompletionCallback &callback) {
  base::AutoLock lock(critical_section_);

  // First check if there's already an opened connection
  ConnectionsMap::iterator conn_it;
  if ((conn_it = connections_.find(destination)) != connections_.end()) {
    conn_it->second->refs_++;
    conn_it->second->timer_.Stop();
    return net::OK;
  }

  // If there's a pending connection, add the callback
  PendingConnectionsMap::iterator pending_it;
  if ((pending_it = pending_connections_.find(destination)) !=
       pending_connections_.end()) {
    pending_it->second.callbacks_.push_back(callback);
    return net::ERR_IO_PENDING;
  }

  // Create a new connection otherwise
  Atom<Protocol> protocol = destination.protocol();
  FactoriesMap::iterator man_it = factories_.find(protocol);
  if (man_it == factories_.end())
    return net::ERR_NOT_IMPLEMENTED;
  scoped_refptr<Connection> connection;
  man_it->second->CreateConnection(destination, netlog_, &connection);
  pending_connections_.insert(
    std::make_pair(destination, PendingConnection(connection, callback)));
  // Handle synchronous errors
  return connection->Connect(base::Bind(
    &NetworkLayer::OnConnectionOpened, this, destination));
}

void NetworkLayer::ReleaseConnection(const EndPoint &destination,
                                     const net::CompletionCallback &callback) {
  base::AutoLock lock(critical_section_);

  // If there's a connection to this endpoint, remove a reference.
  ConnectionsMap::iterator conn_it = connections_.find(destination);
  if (conn_it != connections_.end()) {
    conn_it->second->refs_--;
    // When all references reach zero, start the timer.
    if (conn_it->second->refs_ == 0) {
      conn_it->second->timer_.Start(FROM_HERE,
        base::TimeDelta::FromSeconds(kReuseDelay),
        base::Bind(&NetworkLayer::OnConnectionTimedOut, this, destination));
    }
  }
  else {
    // Check if there's a pending connection.
    PendingConnectionsMap::iterator pending_it =
      pending_connections_.find(destination);
    if (pending_it != pending_connections_.end()) {
      // Check if the passed in callback has been associated with the
      // found connection.
      std::vector<net::CompletionCallback>::iterator i, ie;
      for (i = pending_it->second.callbacks_.begin(),
           ie = pending_it->second.callbacks_.end(); i != ie; ++i) {
        if (i->Equals(callback))
          break;
      }
      // Remove the attached callback.
      if (i != ie) {
        i = pending_it->second.callbacks_.erase(i);
        // If this is the last callback, so the connection attempt will be
        // immediately cancelled and the connection will be destroyed.
        if (pending_it->second.callbacks_.empty()) {
          pending_it->second.connection_->Close();
          pending_connections_.erase(pending_it);
        }
      }
    }
  }
}

int NetworkLayer::Send(scoped_refptr<Message> message,
                       const EndPoint &destination,
                       const net::CompletionCallback& callback) {
  Via *via = message->get<Via>();
  if (!via || via->empty())
    return net::ERR_INVALID_ARGUMENT;

  base::AutoLock lock(critical_section_);

  ConnectionsMap::iterator i = connections_.find(destination);
  if (i == connections_.end())
    return net::ERR_NOT_IMPLEMENTED;

  ViaParam &topmost_via = via->front();
  topmost_via.set_protocol(destination.protocol());

  // If a request, set the Via protocol and sent-by.
  if (isa<Request>(message)) {
    topmost_via.set_sent_by(destination.hostport());
  }
  // If a response, stamp the received/rport of the topmost via.
  else if (isa<Response>(message)
           && !topmost_via.sent_by().Equals(destination.hostport())) {
    topmost_via.set_received(destination.host());
    topmost_via.set_rport(destination.port());
  }
  
  // Serialize the SIP message.
  std::string output;
  raw_string_ostream os(output);
  message->print(os);

  // Dispatch message to the found connection.
  int res = i->second->connection_->Send(os.str(), callback);
  if (res != net::OK && res != net::ERR_IO_PENDING) {
    // If the connection has been reset, close immediately.
    // There's no need to call CloseWithError, as the Send implementation
    // should propagate the sending error to the pending send queue.
    connections_.erase(i);
  }
  return res;
}

void NetworkLayer::HandleIncomingConnection(const EndPoint &destination,
                                            Connection *connection) {
  base::AutoLock lock(critical_section_);

  // Insert the newly created connection.
  scoped_ptr<OpenedConnection> opened_connection(new OpenedConnection(connection));
  std::pair<ConnectionsMap::iterator, bool> res = connections_.insert(
    std::make_pair(destination, opened_connection.get()));

  // It's not possible to substitute an existing connection
  // to the same destination.
  DCHECK(res.second);
  ignore_result(opened_connection.release());

  // Start the reuse timer.
  res.first->second->timer_.Start(FROM_HERE,
        base::TimeDelta::FromSeconds(kReuseDelay),
        base::Bind(&NetworkLayer::OnConnectionTimedOut, this, destination));
}

void NetworkLayer::HandleIncomingMessage(const EndPoint &destination,
                                         Message *incoming_message) {
  base::AutoLock lock(critical_section_);

  ConnectionsMap::iterator i = connections_.find(destination);
  
  // The connection factory must register the connection first.
  DCHECK(i != connections_.end());

  // Refresh connection timeout if running.
  if (i->second->timer_.IsRunning())
    i->second->timer_.Reset();

  // Check the topmost via header
  Via *via = incoming_message->get<Via>();
  if (!via || via->empty()) {
    VLOG(1) << "incoming message without Via";
    return;
  }
  
  ViaParam &topmost_via = via->front();
  if (!destination.protocol().Equals(topmost_via.protocol())) {
    VLOG(1) << "message Via protocol didn't match";
    return;
  }

  // If it's a request, stamp received and rport accordingly
  if (isa<Request>(incoming_message)) {
    if (!topmost_via.sent_by().Equals(destination.hostport())) {
      topmost_via.set_received(destination.host());
      topmost_via.set_rport(destination.port());
    }
  }

  // TODO: other checks would go here
  
  // Send incoming message upwards
  delegate_->OnIncomingMessage(incoming_message);
}

void NetworkLayer::OnSuspend() {
  base::AutoLock lock(critical_section_);

  if (!suspended_) {
    // Close all connections as if all connections have been aborted
    while (!connections_.empty()) {
      ConnectionsMap::iterator top = connections_.begin();
      top->second->connection_->CloseWithError(net::ERR_ABORTED);
      delete top->second;
      connections_.erase(top);
    }
    // Close all pending connections in the same way
    while (!pending_connections_.empty()) {
      PendingConnectionsMap::iterator top = pending_connections_.begin();
      while (!top->second.callbacks_.empty()) {
        top->second.callbacks_.back().Run(net::ERR_ABORTED);
        top->second.callbacks_.pop_back();
      }
      pending_connections_.erase(top);
    }
    // Close all listening sockets
    for (std::vector<EndPoint>::iterator i = listening_.begin(),
         ie = listening_.end(); i != ie; ++i) {
      FactoriesMap::iterator j = factories_.find(i->protocol());
      if (j != factories_.end()) {
        j->second->StopListen(*i);
      }
    }
    suspended_ = true;
  }
}

void NetworkLayer::OnResume() {
  base::AutoLock lock(critical_section_);

  if (suspended_) {
    // Restore listening sockets
    for (std::vector<EndPoint>::iterator i = listening_.begin(),
         ie = listening_.end(); i != ie; ++i) {
      FactoriesMap::iterator j = factories_.find(i->protocol());
      if (j != factories_.end()) {
        // XXX: what should we do if it doesn't work?
        j->second->Listen(*i);
      }
    }
    suspended_ = false;
  }
}

void NetworkLayer::OnConnectionTimedOut(const EndPoint &endpoint) {
  // TODO
}

void NetworkLayer::OnConnectionOpened(const EndPoint &endpoint, int err) {
  // TODO
}

} // End of sippet namespace