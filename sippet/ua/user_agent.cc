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
  // Create an UAS dialog when appropriated
  if (isa<Response>(message)) {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    int response_code = response->response_code();
    if (response_code > 100) {
      scoped_refptr<Request> request(response->refer_to());
      std::string id(response->GetDialogId());
      DialogMapType::iterator i = dialogs_.find(id);
      if (dialogs_.end() == i) {
        scoped_refptr<Dialog> dialog(
            Dialog::CreateServerDialog(request, response));
        dialogs_.insert(std::make_pair(id, dialog));
      }
      else if (response_code/100 == 2) {
        i->second->set_state(Dialog::STATE_CONFIRMED);
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

void UserAgent::OnChannelConnected(const EndPoint &destination, int err) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnChannelConnected(destination, err);
  }
}

void UserAgent::OnChannelClosed(const EndPoint &destination) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnChannelClosed(destination);
  }
}

void UserAgent::OnIncomingRequest(
    const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog;
  std::string id(request->GetDialogId());
  DialogMapType::iterator i = dialogs_.find(id);
  if (dialogs_.end() != i)
    dialog = i->second;
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingRequest(request, dialog);
  }
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  // Create an UAC dialog when appropriated
  scoped_refptr<Dialog> dialog;
  int response_code = response->response_code();
  if (response_code > 100) {
    scoped_refptr<Request> request(response->refer_to());
    std::string id(response->GetDialogId());
    DialogMapType::iterator i = dialogs_.find(id);
    if (dialogs_.end() == i) {
      dialog = Dialog::CreateClientDialog(request, response);
      dialogs_.insert(std::make_pair(id, dialog));
    }
    else if (response_code/100 == 2) {
      dialog = i->second;
      dialog->set_state(Dialog::STATE_CONFIRMED);
    }
  }
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingResponse(response, dialog);
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
