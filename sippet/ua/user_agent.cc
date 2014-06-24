// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/user_agent.h"
#include "net/base/net_errors.h"

namespace sippet {
namespace ua {

UserAgent::UserAgent() {
}

void UserAgent::AppendHandler(Delegate *delegate) {
  handlers_.push_back(delegate);
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const GURL &from_uri,
    const GURL &to_uri) {
  GURL request_uri; // TODO
  scoped_refptr<Request> request(
    new Request(method, request_uri));
  scoped_ptr<To> to(new To(to_uri));
  request->push_back(to.PassAs<Header>());
  scoped_ptr<From> from(new From(from_uri));
  request->push_back(from.PassAs<Header>());
  scoped_ptr<Cseq> cseq(new Cseq(local_sequence_, method)); // TODO
  request->push_back(cseq.PassAs<Header>());
  NOTIMPLEMENTED();
  return request;
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const scoped_refptr<Dialog> &dialog) {
  scoped_refptr<Request> request(
    CreateRequest(method, dialog->local_uri(), dialog->remote_uri()));
  request->get<Cseq>()->set_sequence(dialog->local_sequence()); // TODO
  NOTIMPLEMENTED();
  return request;
}

scoped_refptr<Response> UserAgent::CreateResponse(
    int response_code,
    const scoped_refptr<Request> &request) {
  NOTIMPLEMENTED();
  return 0;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  return network_layer_->Send(message, callback);
}

void UserAgent::OnChannelConnected(const EndPoint &destination) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnChannelConnected(destination);
  }
}

void UserAgent::OnChannelClosed(const EndPoint &destination, int err) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnChannelClosed(destination, err);
  }
}

void UserAgent::OnIncomingRequest(
    const scoped_refptr<Request> &request) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingRequest(request, 0); // TODO: dialog matching
  }
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingResponse(response, 0); // TODO: dialog matching
  }
}

void UserAgent::OnTimedOut(const std::string &id) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTimedOut(id);
  }
}

void UserAgent::OnTransportError(const std::string &id, int err) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTransportError(id, err);
  }
}

} // End of ua namespace
} // End of sippet namespace
