// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_layer.h"

#include "base/stl_util.h"
#include "net/base/net_errors.h"
#include "sippet/message/headers/via.h"
#include "sippet/transport/channel.h"
#include "sippet/transport/channel_factory.h"

namespace sippet {

NetworkLayer::NetworkLayer(Delegate *delegate,
                           TransactionFactory *transaction_factory,
                           const NetworkSettings &network_settings)
  : delegate_(delegate),
    transaction_factory_(transaction_factory),
    network_settings_(network_settings),
    suspended_(false) {
  DCHECK(delegate);
  DCHECK(transaction_factory);
}

NetworkLayer::~NetworkLayer() {
}

void NetworkLayer::RegisterChannelFactory(const Protocol &protocol,
                                          ChannelFactory *channel_factory) {
  DCHECK(channel_factory);
  factories_.insert(std::make_pair(protocol, channel_factory));
}

bool NetworkLayer::RequestChannel(const EndPoint &destination) {
  bool success = true;
  ChannelsMap::iterator conn_it;
  if ((conn_it = channels_.find(destination)) == channels_.end()) {
    success = false;
  }
  else {
    // While adding an use, don't forget to stop the timer
    conn_it->second.refs_++;
    conn_it->second.timer_.Stop();
  }
  return success;
}

void NetworkLayer::ReleaseChannel(const EndPoint &destination) {
  ChannelsMap::iterator conn_it = channels_.find(destination);
  if (conn_it != channels_.end()) {
    conn_it->second.refs_--;
    // When all references reach zero, start the timer.
    if (conn_it->second.refs_ == 0) {
      conn_it->second.timer_.Start(FROM_HERE,
        base::TimeDelta::FromSeconds(network_settings_.reuse_lifetime()),
        base::Bind(&NetworkLayer::OnIdleChannelTimedOut, this, destination));
    }
  }
}

int NetworkLayer::Send(const scoped_refptr<Message> &message,
                       const net::CompletionCallback& callback) {
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);

    // TODO: Use the Request-URI as the destination EndPoint.

    // If there's an allocated channel for that EndPoint, check if the channel
    // is open; if so and it's not an ACK, create a transaction and reuse it,
    // otherwise return error. If it's an ACK, send it directly through the
    // channel.

    // Otherwise, create a new channel to that destination and connect
    // asynchronously. Save the callback.
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);

    // TODO: Use the topmost Via as the EndPoint destination, using 'received'
    // and 'rport' if available.

    // If there's a server transaction for that EndPoint, route the message to
    // it. Otherwise, check if there's an available channel for that
    // destination; if so, dispatch the message directly, otherwise return
    // error.
  }
  return net::OK;
}

bool NetworkLayer::AddAlias(const EndPoint &destination, const EndPoint &alias) {
  bool success = false;
  ChannelsMap::iterator conn_it = channels_.find(destination);
  if (conn_it != channels_.end()) {
    aliases_map_.AddAlias(destination, alias);
    success = true;
  }
  return success;
}

void NetworkLayer::OnSuspend() {
  if (!suspended_) {
    // TODO: Close all connections as if all connections have been aborted.
    suspended_ = true;
  }
}

void NetworkLayer::OnResume() {
  if (suspended_) {
    // TODO
    suspended_ = false;
  }
}

void NetworkLayer::OnChannelConnected(const scoped_refptr<Channel> &channel) {
  // TODO: Dispatch the enqueued message, execute the callback if appropriated
  // and don't forget to cleanup the callback afterwards.
}

void NetworkLayer::OnIncomingMessage(const scoped_refptr<Message> &message) {
  // TODO: if request, create a new server transaction and pass to it.
  // Otherwise, check if it pertains to one of the existing transactions;
  // if not, then pass directly to the delegate. If it's a request, stamp the
  // remote address in the topmost Via.
}

void NetworkLayer::OnChannelClosed(const scoped_refptr<Channel> &channel,
                                   int error) {
  // TODO: Remove all available aliases to the connected destination, pass
  // the error to connected transactions, and finally send the callback to
  // the delegate.
}

void NetworkLayer::OnPassMessage(const scoped_refptr<Message> &) {
  // TODO: Just pass the message to the delegate.
}

void NetworkLayer::OnTransactionTerminated(
    const scoped_refptr<Transaction> &transaction) {
  // TODO: Remove the transaction from the existing channel list and from the
  // transaction map.
}

void NetworkLayer::OnIdleChannelTimedOut(const EndPoint &endpoint) {
  // TODO: Remove the channel from the channel map and pass to delegate.
}

} // End of sippet namespace
