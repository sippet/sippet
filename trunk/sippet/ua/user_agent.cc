// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/user_agent.h"
#include "sippet/uri/uri.h"
#include "net/base/net_errors.h"
#include "crypto/random.h"
#include "base/strings/string_number_conversions.h"
#include <sstream>

namespace sippet {
namespace ua {

UserAgent::UserAgent() {
}

void UserAgent::AppendHandler(Delegate *delegate) {
  handlers_.push_back(delegate);
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const GURL &request_uri,
    const GURL &from_uri,
    const GURL &to_uri) {
  scoped_refptr<Request> request(
    new Request(method, request_uri));
  scoped_ptr<To> to(new To(to_uri));
  request->push_back(to.PassAs<Header>());
  
  // Add the From header and a local tag (32-bit random string)
  scoped_ptr<From> from(new From(from_uri));
  from->set_tag(CreateTag());
  request->push_back(from.PassAs<Header>());
  
  // The Call-ID is formed by a 32-bit random string
  scoped_ptr<CallId> call_id(new CallId(CreateCallId()));
  request->push_back(call_id.PassAs<Header>());
  
  // Cseq always contain the request method and the local sequence
  scoped_ptr<Cseq> cseq(new Cseq(local_sequence_++, method));
  request->push_back(cseq.PassAs<Header>());

  // Max-Forwards header field is always 70.
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  request->push_back(max_forwards.PassAs<Header>());

  // Contact: TODO

  NOTIMPLEMENTED();
  return request;
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const scoped_refptr<Dialog> &dialog) {
  GURL request_uri;
  scoped_refptr<Request> request;
  if (dialog->route_set().empty()) {
    request = CreateRequest(method, dialog->remote_target(),
        dialog->local_uri(), dialog->remote_uri());
  }
  else {
    // TODO
  }
  request->get<CallId>()->set_value(dialog->call_id());
  request->get<Cseq>()->set_sequence(dialog->GetNewLocalSequence());
  request->get<From>()->set_tag(dialog->local_tag());
  if (!dialog->remote_tag().empty())
    request->get<To>()->set_tag(dialog->remote_tag());
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

std::string UserAgent::Create32BitRandomString() {
  char tag[4];
  crypto::RandBytes(tag, sizeof(tag));
  return base::HexEncode(tag, sizeof(tag));
}

std::string UserAgent::CreateTag() {
  return Create32BitRandomString();
}

std::string UserAgent::CreateCallId() {
  return Create32BitRandomString();
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
