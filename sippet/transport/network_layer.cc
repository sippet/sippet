// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_layer.h"

#include "base/stl_util.h"
#include "base/string_util.h"
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
                           const NetworkSettings &network_settings,
                           BranchFactory *branch_factory)
  : delegate_(delegate),
    transaction_factory_(transaction_factory),
    network_settings_(network_settings),
    suspended_(false),
    branch_factory_(branch_factory),
    ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)) {
  DCHECK(delegate);
  DCHECK(transaction_factory);
  DCHECK(branch_factory);
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
  ChannelContext *channel_context = GetChannelContext(destination);
  if (channel_context)
    RequestChannelInternal(channel_context);
  return channel_context ? true : false;
}

void NetworkLayer::ReleaseChannel(const EndPoint &destination) {
  ChannelContext *channel_context = GetChannelContext(destination);
  if (channel_context)
    ReleaseChannelInternal(channel_context);
}

int NetworkLayer::Send(const scoped_refptr<Message> &message,
                       const net::CompletionCallback& callback) {
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    return SendRequest(request, callback);
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    return SendResponse(response, callback);
  }
}

bool NetworkLayer::AddAlias(const EndPoint &destination, const EndPoint &alias) {
  ChannelContext *channel_context = GetChannelContext(destination);
  if (channel_context)
    aliases_map_.AddAlias(destination, alias);
  return channel_context ? true : false;
}

void NetworkLayer::OnSuspend() {
  if (!suspended_) {
    for (ChannelsMap::iterator i = channels_.begin(), ie = channels_.end();
         i != ie;) {
      scoped_refptr<Channel> channel = (i++)->second->channel_;
      OnChannelClosed(channel, net::ERR_ABORTED);
    }
    suspended_ = true;
  }
}

void NetworkLayer::OnResume() {
  if (suspended_) {
    // TODO
    suspended_ = false;
  }
}

int NetworkLayer::SendRequest(scoped_refptr<Request> &request,
                              const net::CompletionCallback& callback) {
  EndPoint destination(GetMessageEndPoint(request));
  if (destination.IsEmpty()) {
    DVLOG(1) << "invalid Request-URI";
    return net::ERR_INVALID_ARGUMENT;
  }

  ChannelContext *channel_context = GetChannelContext(destination);
  if (channel_context) {
    if (!channel_context->channel_->is_connected()) {
      DVLOG(1) << "Cannot send a request yet";
      return net::ERR_SOCKET_NOT_CONNECTED;
    }
    StampClientTopmostVia(request, channel_context->channel_);
    // Send ACKs out of transactions
    if (Method::ACK != request->method()) {
      // The created transaction will handle the response processing.
      // Requests don't need to be passed to client transactions.
      ignore_result(CreateClientTransaction(request, channel_context));
    }
    return channel_context->channel_->Send(request, callback);
  }
  else {
    if (Method::ACK == request->method()) {
      // ACK requests can't open connections, therefore they will be rejected.
      DVLOG(1) << "ACK requests can't open connections";
      return net::ERR_ABORTED;
    }
    int result = CreateChannelContext(
      destination, request, callback, &channel_context);
    if (result != net::OK)
      return result;
    channel_context->channel_->Connect();
    // Now wait for the asynchronous connect. When using UDP,
    // the connect event will occur in the next event loop.
    return net::ERR_IO_PENDING;
  }
}

int NetworkLayer::SendResponse(const scoped_refptr<Response> &response,
                               const net::CompletionCallback& callback) {
  scoped_refptr<ServerTransaction> server_transaction =
    GetServerTransaction(response);
  if (server_transaction) {
    server_transaction->Send(response);
  }
  else {
    // When there's no server transaction available, tries to send the
    // response directly through an available channel.
    EndPoint destination(GetMessageEndPoint(response));
    if (destination.IsEmpty()) {
      DVLOG(1) << "Impossible to route without Via";
      return net::ERR_INVALID_ARGUMENT;
    }
    ChannelContext *channel_context = GetChannelContext(destination);
    if (!channel_context) {
      DVLOG(1) << "No channel can send the message";
      return net::ERR_SOCKET_NOT_CONNECTED;
    }
    channel_context->channel_->Send(response, callback);
  }

  return net::OK;
}

void NetworkLayer::RequestChannelInternal(ChannelContext *channel_context) {
  DCHECK(channel_context);

  // While adding an use, don't forget to stop the timer
  channel_context->refs_++;
  if (channel_context->timer_.IsRunning())
    channel_context->timer_.Stop();
}

void NetworkLayer::ReleaseChannelInternal(ChannelContext *channel_context) {
  DCHECK(channel_context);

  channel_context->refs_--;
  // When all references reach zero, start the timer.
  if (channel_context->refs_ == 0) {
    channel_context->timer_.Start(FROM_HERE,
      base::TimeDelta::FromSeconds(network_settings_.reuse_lifetime()),
      base::Bind(&NetworkLayer::OnIdleChannelTimedOut, weak_factory_.GetWeakPtr(),
        channel_context->channel_->destination()));
  }
}

ClientTransaction *NetworkLayer::CreateClientTransaction(
          const scoped_refptr<Request> &request,
          ChannelContext *channel_context) {
  scoped_refptr<ClientTransaction> client_transaction =
    transaction_factory_->CreateClientTransaction(request->method(),
      ClientTransactionId(request),
      channel_context->channel_,
      this);
  client_transactions_[client_transaction->id()] = client_transaction;
  channel_context->transactions_.insert(client_transaction->id());
  RequestChannelInternal(channel_context);
  client_transaction->Start(request);
  return client_transaction;
}

ServerTransaction *NetworkLayer::CreateServerTransaction(
          const scoped_refptr<Request> &request,
          ChannelContext *channel_context) {
  scoped_refptr<ServerTransaction> server_transaction =
    transaction_factory_->CreateServerTransaction(request->method(),
      ServerTransactionId(request),
      channel_context->channel_,
      this);
  server_transactions_[server_transaction->id()] = server_transaction;
  channel_context->transactions_.insert(server_transaction->id());
  RequestChannelInternal(channel_context);
  server_transaction->Start(request);
  return server_transaction;
}

void NetworkLayer::DestroyClientTransaction(
                const scoped_refptr<ClientTransaction> &client_transaction) {
  client_transactions_.erase(client_transaction->id());
  ChannelContext *channel_context =
    GetChannelContext(client_transaction->channel()->destination());
  if (channel_context) {
    channel_context->transactions_.erase(client_transaction->id());
    ReleaseChannelInternal(channel_context);
  }
  client_transaction->Close();
}
void NetworkLayer::DestroyServerTransaction(
                const scoped_refptr<ServerTransaction> &server_transaction) {
  server_transactions_.erase(server_transaction->id());
  ChannelContext *channel_context =
    GetChannelContext(server_transaction->channel()->destination());
  if (channel_context) {
    channel_context->transactions_.erase(server_transaction->id());
    ReleaseChannelInternal(channel_context);
  }
  server_transaction->Close();
}

int NetworkLayer::CreateChannelContext(
          const EndPoint &destination,
          const scoped_refptr<Request> &request,
          const net::CompletionCallback &callback,
          ChannelContext **created_channel_context) {
  DCHECK(created_channel_context);

  // Find the factory and create the channel.
  scoped_refptr<Channel> channel;
  FactoriesMap::iterator factories_it =
    factories_.find(destination.protocol());
  if (factories_it == factories_.end())
    return net::ERR_ADDRESS_UNREACHABLE;
  int result = factories_it->second->CreateChannel(
    destination, this, &channel);
  if (result != net::OK)
    return result;

  *created_channel_context = new ChannelContext(channel, request, callback);
  channels_[destination] = *created_channel_context;
  return net::OK;
}

void NetworkLayer::DestroyChannelContext(ChannelContext *channel_context) {
  DCHECK(channel_context);

  channels_.erase(channel_context->channel_->destination());

  // The following code works as a 'cascade on delete'
  // for existing transactions still using the channel.
  for (std::set<std::string>::iterator i =
         channel_context->transactions_.begin(),
       ie = channel_context->transactions_.end();
       i != ie;) {
    OnTransactionTerminated(*i++);
  }

  delete channel_context;
}

std::string NetworkLayer::CreateBranch() {
  return branch_factory_->CreateBranch();
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
  EndPoint destination(channel->destination());
  Message::iterator topmost_via = request->find_first<Via>();
  if (topmost_via == request->end()) {
    // When there's no Via header, we create one
    // using the channel destination and empty branch
    scoped_ptr<Via> via(new Via);
    net::HostPortPair hostport(destination.host(), destination.port());
    via->push_back(ViaParam(destination.protocol(), hostport));
    request->push_front(via.PassAs<Header>());
  }
  else {
    Via *via = dyn_cast<Via>(topmost_via);
    if (via->front().sent_by().host() != destination.host())
      via->front().set_received(destination.host());
    if (via->front().sent_by().port() != destination.port())
      via->front().set_rport(destination.port());
  }
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

EndPoint NetworkLayer::GetMessageEndPoint(
    const scoped_refptr<Message> &message) {
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    return EndPoint::FromGURL(request->request_uri());
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    Message::iterator topmost_via = response->find_first<Via>();
    if (topmost_via == response->end())
      return EndPoint();
    Via *via = dyn_cast<Via>(topmost_via);
    EndPoint result(via->front().sent_by(), via->front().protocol());
    if (via->front().HasReceived())
      result.set_host(via->front().received());
    if (via->front().HasRport())
      result.set_port(via->front().rport());
    return result;
  }
}

NetworkLayer::ChannelContext *NetworkLayer::GetChannelContext(
    const EndPoint &destination) {
  ChannelsMap::iterator channel_it;
  channel_it = channels_.find(destination);
  if (channel_it == channels_.end())
    return 0;
  return channel_it->second;
}

scoped_refptr<ClientTransaction> NetworkLayer::GetClientTransaction(
                                      const scoped_refptr<Message> &message) {
  DCHECK(isa<Response>(message));

  scoped_refptr<Response> response = dyn_cast<Response>(message);
  std::string transaction_id(ClientTransactionId(response));
  return GetClientTransaction(transaction_id);
}

scoped_refptr<ServerTransaction> NetworkLayer::GetServerTransaction(
                                      const scoped_refptr<Message> &message) {
  std::string transaction_id;
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    transaction_id = ServerTransactionId(request);
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    transaction_id = ServerTransactionId(response);
  }
  return GetServerTransaction(transaction_id);
}

scoped_refptr<ClientTransaction> NetworkLayer::GetClientTransaction(
                      const std::string &transaction_id) {
  ClientTransactionsMap::iterator client_transactions_it =
    client_transactions_.find(transaction_id);
  if (client_transactions_it == client_transactions_.end())
    return 0;
  return client_transactions_it->second;
}
scoped_refptr<ServerTransaction> NetworkLayer::GetServerTransaction(
                      const std::string &transaction_id) {
  ServerTransactionsMap::iterator server_transactions_it =
    server_transactions_.find(transaction_id);
  if (server_transactions_it == server_transactions_.end())
    return 0;
  return server_transactions_it->second;
}

void NetworkLayer::OnChannelConnected(const scoped_refptr<Channel> &channel,
                                      int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);

  ChannelsMap::iterator channel_it = channels_.find(channel->destination());
  DCHECK(channel_it != channels_.end());
  ChannelContext *channel_context = channel_it->second;
  if (result == net::OK) {
    StampClientTopmostVia(channel_context->initial_request_,
      channel_context->channel_);
    ignore_result(CreateClientTransaction(
      channel_context->initial_request_, channel_context));
    result = channel_context->channel_->Send(
      channel_context->initial_request_, channel_context->initial_callback_);
    if (result == net::OK) {
      if (!channel_context->initial_callback_.is_null()) {
        // Complete the pending send callback now
        channel_context->initial_callback_.Run(net::OK);
      }
    }
    if (result == net::OK || result == net::ERR_IO_PENDING) {
      // Clean channel initial context, the channel will be
      // responsible for the callback now.
      channel_context->initial_request_ = NULL;
      channel_context->initial_callback_.Reset();
    }
  }
  if (result != net::OK && result != net::ERR_IO_PENDING) {
    net::CompletionCallback callback(channel_context->initial_callback_);
    scoped_refptr<Channel> channel(channel_context->channel_);
    EndPoint destination(channel_context->channel_->destination());
    DestroyChannelContext(channel_context);
    channel->Close();
    callback.Run(result);
    delegate_->OnChannelClosed(destination, result);
  }
}

void NetworkLayer::OnIncomingMessage(const scoped_refptr<Channel> &channel,
                                     const scoped_refptr<Message> &message) {
  if (isa<Request>(message)) {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    StampServerTopmostVia(request, channel);
    scoped_refptr<ServerTransaction> server_transaction =
      GetServerTransaction(request);
    if (server_transaction)
      server_transaction->HandleIncomingRequest(request);
    else
      HandleIncomingRequest(channel, request);
  }
  else {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    scoped_refptr<ClientTransaction> client_transaction =
      GetClientTransaction(response);
    if (client_transaction)
      client_transaction->HandleIncomingResponse(response);
    else
      HandleIncomingResponse(channel, response);
  }
}

void NetworkLayer::HandleIncomingRequest(
                                 const scoped_refptr<Channel> &channel,
                                 const scoped_refptr<Request> &request) {
  ChannelContext *channel_context =
    GetChannelContext(channel->destination());

  DCHECK(channel_context);

  // Server transactions are created in advance
  CreateServerTransaction(request, channel_context);
  delegate_->OnIncomingMessage(request);
}

void NetworkLayer::HandleIncomingResponse(
                                 const scoped_refptr<Channel> &channel,
                                 const scoped_refptr<Response> &response) {
  ChannelContext *channel_context =
    GetChannelContext(channel->destination());

  DCHECK(channel_context);

  delegate_->OnIncomingMessage(response);
}

void NetworkLayer::OnChannelClosed(const scoped_refptr<Channel> &channel,
                                   int error) {
  ChannelContext *channel_context = GetChannelContext(channel->destination());
  DCHECK(channel_context);

  EndPoint destination(channel->destination());
  scoped_refptr<Channel> closing_channel(channel);
  DestroyChannelContext(channel_context);
  closing_channel->CloseWithError(error);
  delegate_->OnChannelClosed(destination, error);
}

void NetworkLayer::OnPassMessage(const scoped_refptr<Message> &message) {
  delegate_->OnIncomingMessage(message);
}

void NetworkLayer::OnTransactionTerminated(const std::string &transaction_id) {
  if (StartsWithASCII(transaction_id, "c:", true)) {
    scoped_refptr<ClientTransaction> client_transaction =
      GetClientTransaction(transaction_id);
    DestroyClientTransaction(client_transaction);
  }
  else {
    scoped_refptr<ServerTransaction> server_transaction =
      GetServerTransaction(transaction_id);
    DestroyServerTransaction(server_transaction);
  }
}

void NetworkLayer::OnIdleChannelTimedOut(const EndPoint &endpoint) {
  ChannelContext *channel_context = GetChannelContext(endpoint);
  OnChannelClosed(channel_context->channel_, net::ERR_TIMED_OUT);
}

} // End of sippet namespace
