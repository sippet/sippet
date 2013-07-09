// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_layer.h"

#include "base/stl_util.h"
#include "base/string_util.h"
#include "base/rand_util.h"
#include "base/base64.h"
#include "net/base/net_errors.h"
#include "sippet/message/headers/via.h"
#include "sippet/message/headers/cseq.h"
#include "sippet/transport/channel.h"
#include "sippet/transport/channel_factory.h"
#include "sippet/transport/transaction_factory.h"

namespace sippet {

const char NetworkLayer::kMagicCookie[] = "z9hG4bK";

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
  STLDeleteContainerPairSecondPointers(channels_.begin(), channels_.end());
}

void NetworkLayer::RegisterChannelFactory(const Protocol &protocol,
                                          ChannelFactory *channel_factory) {
  DCHECK(channel_factory);
  factories_.insert(std::make_pair(protocol, channel_factory));
}

bool NetworkLayer::RequestChannel(const EndPoint &destination) {
  bool success = true;
  ChannelsMap::iterator chan_it;
  if ((chan_it = channels_.find(destination)) == channels_.end())
    success = false;
  else
    RequestChannelInternal(chan_it->second);
  return success;
}

void NetworkLayer::ReleaseChannel(const EndPoint &destination) {
  ChannelsMap::iterator chan_it = channels_.find(destination);
  if (chan_it != channels_.end())
    ReleaseChannelInternal(chan_it->second);
}

int NetworkLayer::Send(const scoped_refptr<Message> &message,
                       const net::CompletionCallback& callback) {
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);

    EndPoint destination(EndPoint::FromGURL(request->request_uri()));
    if (destination.IsEmpty()) {
      DVLOG(1) << "invalid Request-URI";
      return net::ERR_INVALID_ARGUMENT;
    }

    ChannelsMap::iterator chan_it = channels_.find(destination);
    if (chan_it != channels_.end()) {
      if (!chan_it->second->channel_->is_connected()) {
        DVLOG(1) << "Cannot send a request yet";
        return net::ERR_SOCKET_NOT_CONNECTED;
      }
      // Send ACKs out of transactions
      if (Method::ACK != request->method())
        CreateClientTransaction(request, chan_it->second);
      return chan_it->second->channel_->Send(request, callback);
    }
    else {
      scoped_refptr<Channel> channel;
      int result = CreateChannel(destination, &channel);
      if (result != net::OK)
        return result;
      ChannelEntry *entry = new ChannelEntry(channel, request, callback);
      channels_[destination] = entry;
      channel->Connect();
      // Now wait for the asynchronous connect. When using UDP,
      // the connect event will occur in the next event loop.
    }
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);

    std::string transaction_id(ServerTransactionId(response));
    ServerTransactionsMap::iterator server_transactions_it =
      server_transactions_.find(transaction_id);
    if (server_transactions_it != server_transactions_.end()) {
      server_transactions_it->second->transaction_->Send(response, callback);
    }
    else {
      // Send the response directly
      Message::iterator topmost_via = response->find_first<Via>();
      if (topmost_via == response->end()) {
        DVLOG(1) << "Impossible to route without Via";
        return net::ERR_INVALID_ARGUMENT;
      }
      Via *via = dyn_cast<Via>(topmost_via);
      EndPoint destination(via->front().sent_by(), via->front().protocol());
      if (via->front().HasReceived())
        destination.set_host(via->front().received());
      if (via->front().HasRport())
        destination.set_port(via->front().rport());
      ChannelsMap::iterator chan_it = channels_.find(destination);
      if (chan_it == channels_.end()) {
        DVLOG(1) << "No channel can send the message";
        return net::ERR_SOCKET_NOT_CONNECTED;
      }
      chan_it->second->channel_->Send(response, callback);
    }
  }
  return net::OK;
}

bool NetworkLayer::AddAlias(const EndPoint &destination, const EndPoint &alias) {
  bool success = false;
  ChannelsMap::iterator chan_it = channels_.find(destination);
  if (chan_it != channels_.end()) {
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

void NetworkLayer::RequestChannelInternal(ChannelEntry *entry) {
  // While adding an use, don't forget to stop the timer
  entry->refs_++;
  if (entry->timer_.IsRunning())
    entry->timer_.Stop();
}

void NetworkLayer::ReleaseChannelInternal(ChannelEntry *entry) {
  entry->refs_--;
  // When all references reach zero, start the timer.
  if (entry->refs_ == 0) {
    entry->timer_.Start(FROM_HERE,
      base::TimeDelta::FromSeconds(network_settings_.reuse_lifetime()),
      base::Bind(&NetworkLayer::OnIdleChannelTimedOut, this,
        entry->channel_->destination()));
  }
}

int NetworkLayer::CreateChannel(const EndPoint &destination,
                                scoped_refptr<Channel> *channel) {
  FactoriesMap::iterator factories_it =
    factories_.find(destination.protocol());
  if (factories_it == factories_.end())
    return net::ERR_ADDRESS_UNREACHABLE;
  return factories_it->second->CreateChannel(
    destination, this, channel);
}

void NetworkLayer::CreateClientTransaction(
          const scoped_refptr<Request> &request,
          ChannelEntry *channel_entry) {
  scoped_refptr<ClientTransaction> client_transaction =
    transaction_factory_->CreateClientTransaction(request->method(),
      ClientTransactionId(request),
      channel_entry->channel_,
      this);
  ClientTransactionEntry *client_transaction_entry =
    new ClientTransactionEntry(client_transaction, channel_entry->channel_);
  client_transactions_[client_transaction->id()] = client_transaction_entry;
  channel_entry->transactions_.push_back(client_transaction->id());
  RequestChannelInternal(channel_entry);
  client_transaction->Start(request);
}

void NetworkLayer::CreateServerTransaction(
          const scoped_refptr<Request> &request,
          ChannelEntry *channel_entry) {
  scoped_refptr<ServerTransaction> server_transaction =
    transaction_factory_->CreateServerTransaction(request->method(),
      ServerTransactionId(request),
      channel_entry->channel_,
      this);
  ServerTransactionEntry *server_transaction_entry =
    new ServerTransactionEntry(server_transaction, channel_entry->channel_);
  server_transactions_[server_transaction->id()] = server_transaction_entry;
  channel_entry->transactions_.push_back(server_transaction->id());
  RequestChannelInternal(channel_entry);
  server_transaction->Start(request);
}

std::string NetworkLayer::CreateBranch() {
  // Base64 will generate a shorter string than hex
  uint64 sixteen_bytes[2] = { base::RandUint64(), base::RandUint64() };
  // An input of 15 bytes will generate 20 characters on output
  base::StringPiece part(reinterpret_cast<char*>(sixteen_bytes),
    sizeof(sixteen_bytes)-1);
  std::string random_string;
  base::Base64Encode(part, &random_string);
  return kMagicCookie + random_string;
}

void NetworkLayer::StampClientTopmostVia(scoped_refptr<Request> &request,
        const scoped_refptr<Channel> &channel) {
  EndPoint origin(channel->origin());
  scoped_ptr<Via> via(new Via);
  net::HostPortPair hostport(origin.host(), origin.port());
  via->push_back(ViaParam(origin.protocol(), hostport));
  via->back().set_branch(CreateBranch());
  request->push_front(via.PassAs<Header>());
}

void NetworkLayer::StampServerTopmostVia(scoped_refptr<Request> &request,
        const scoped_refptr<Channel> &channel) {
  // At this point, the request would have been rejected if there's
  // no topmost Via.
  Message::iterator topmost_via = request->find_first<Via>();
  DCHECK(topmost_via != request->end());
  EndPoint destination(channel->destination());
  Via *via = dyn_cast<Via>(topmost_via);
  if (via->front().sent_by().host() != destination.host())
    via->front().set_received(destination.host());
  if (via->front().sent_by().port() != destination.port())
    via->front().set_rport(destination.port());
}

std::string NetworkLayer::ClientTransactionId(
              const scoped_refptr<Request> &request) {
  Message::const_iterator topmost_via = request->find_first<Via>();
  DCHECK(topmost_via != request->end());
  const Via *via = dyn_cast<Via>(topmost_via);
  std::string id;
  id += "c:"; // Protect against clashes with server transactions
  id += via->front().branch();
  id += ":";
  id += request->method().str();
  return id;
}

std::string NetworkLayer::ClientTransactionId(
              const scoped_refptr<Response> &response) {
  Message::const_iterator topmost_via = response->find_first<Via>();
  Message::const_iterator cseq_it = response->find_first<Cseq>();
  DCHECK(topmost_via != response->end());
  DCHECK(cseq_it != response->end());
  const Via *via = dyn_cast<Via>(topmost_via);
  const Cseq *cseq = dyn_cast<Cseq>(cseq_it);
  std::string id;
  id += "c:";
  id += via->front().branch();
  id += ":";
  id += cseq->method().str();
  return id;
}

std::string NetworkLayer::ServerTransactionId(
              const scoped_refptr<Request> &request) {
  Message::const_iterator topmost_via = request->find_first<Via>();
  if (topmost_via != request->end()) {
    const Via *via = dyn_cast<Via>(topmost_via);
    if (StartsWithASCII(via->front().branch(), kMagicCookie, false)) {
      std::string id;
      id += "s:"; // Protect against clashes with server transactions
      id += via->front().branch();
      id += ":";
      id += via->front().sent_by().ToString();
      id += ":";
      Method method(request->method());
      if (method == Method::ACK)
        method = Method::INVITE;
      id += method.str();
      return id;
    }
  }
  Message::const_iterator to_it = request->find_first<To>();
  Message::const_iterator from_it = request->find_first<From>();
  Message::const_iterator callid_it = request->find_first<CallId>();
  Message::const_iterator cseq_it = request->find_first<Cseq>();
  // These headers are mandatory:
  DCHECK(to_it != request->end() && from_it != request->end()
         && callid_it != request->end() && cseq_it != request->end());
  // This is the fallback compatibility with ancient RFC 2543 implementations.
  // We're not considering the Request-URI as there's no way to relate the
  // subsequent responses to the transaction afterwards. There's a possibility
  // to exist clashes, but in practice they will be very rare.
  std::string id;
  id += "s:";
  id += dyn_cast<To>(to_it)->tag();
  id += ":";
  id += dyn_cast<From>(from_it)->tag();
  id += ":";
  id += dyn_cast<CallId>(callid_it)->value();
  id += ":";
  id += base::IntToString(dyn_cast<Cseq>(cseq_it)->sequence());
  id += ":";
  Method method(request->method());
  if (method == Method::ACK)
    method = Method::INVITE;
  id += method.str();
  id += ":";
  if (topmost_via != request->end()) {
    const Via *via = dyn_cast<Via>(topmost_via);
    id += via->front().sent_by().ToString();
    id += ":";
    id += via->front().branch();
  }
  return id;
}

std::string NetworkLayer::ServerTransactionId(
              const scoped_refptr<Response> &response) {
  Message::const_iterator topmost_via = response->find_first<Via>();
  Message::const_iterator cseq_it = response->find_first<Cseq>();
  DCHECK(cseq_it != response->end());
  if (topmost_via != response->end()) {
    const Via *via = dyn_cast<Via>(topmost_via);
    if (StartsWithASCII(via->front().branch(), kMagicCookie, false)) {
      std::string id;
      id += "s:"; // Protect against clashes with server transactions
      id += via->front().branch();
      id += ":";
      id += via->front().sent_by().ToString();
      id += ":";
      // Remember ACKs normally doesn't get answers from UAS's
      id += dyn_cast<Cseq>(cseq_it)->method().str();
      return id;
    }
  }
  Message::const_iterator to_it = response->find_first<To>();
  Message::const_iterator from_it = response->find_first<From>();
  Message::const_iterator callid_it = response->find_first<CallId>();
  // These headers are mandatory:
  DCHECK(to_it != response->end() && from_it != response->end()
         && callid_it != response->end());
  // This is the fallback compatibility with ancient RFC 2543 implementations
  std::string id;
  id += "s:";
  id += dyn_cast<To>(to_it)->tag();
  id += ":";
  id += dyn_cast<From>(from_it)->tag();
  id += ":";
  id += dyn_cast<CallId>(callid_it)->value();
  id += ":";
  id += base::IntToString(dyn_cast<Cseq>(cseq_it)->sequence());
  id += ":";
  Method method(dyn_cast<Cseq>(cseq_it)->method());
  if (method == Method::ACK)
    method = Method::INVITE;
  id += method.str();
  id += ":";
  if (topmost_via != response->end()) {
    const Via *via = dyn_cast<Via>(topmost_via);
    id += via->front().sent_by().ToString();
    id += ":";
    id += via->front().branch();
  }
  return id;
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

void NetworkLayer::OnTransactionTerminated(const std::string &transaction_id) {
  // TODO: Remove the transaction from the existing channel list and from the
  // transaction map.
}

void NetworkLayer::OnIdleChannelTimedOut(const EndPoint &endpoint) {
  // TODO: Remove the channel from the channel map and pass to delegate.
}

} // End of sippet namespace
