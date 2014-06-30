// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/user_agent.h"
#include "sippet/ua/dialog.h"
#include "sippet/uri/uri.h"
#include "sippet/base/tags.h"
#include "sippet/base/sequences.h"
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
    const GURL &request_uri,
    const GURL &from_uri,
    const GURL &to_uri,
    unsigned local_sequence) {
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
  
  // Cseq always contain the request method and a new (random) local sequence
  if (local_sequence == 0)
    local_sequence = Create16BitRandomInteger();
  scoped_ptr<Cseq> cseq(new Cseq(local_sequence, method));
  request->push_back(cseq.PassAs<Header>());

  // Max-Forwards header field is always 70.
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  request->push_back(max_forwards.PassAs<Header>());

  // Contact: TODO


  NOTIMPLEMENTED();
  return request;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  // Create an UAS dialog
  if (isa<Response>(message)) {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    switch (response->response_code()/100) {
      case 1:
      case 2: {
        scoped_refptr<Request> request(response->refer_to());
        scoped_refptr<Dialog> dialog(
            Dialog::CreateServerDialog(request, response));
        dialogs_.insert(std::make_pair(dialog->id(), dialog));
        break;
      }
    }
  }
  return network_layer_->Send(message, callback);
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
  // Create an UAC dialog
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
