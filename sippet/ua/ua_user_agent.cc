// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/ua_user_agent.h"
#include "sippet/ua/dialog.h"
#include "sippet/uri/uri.h"
#include "sippet/base/tags.h"
#include "sippet/base/sequences.h"
#include "sippet/base/stl_extras.h"
#include "net/base/net_errors.h"

namespace sippet {
namespace ua {

UserAgent::IncomingRequestContext::IncomingRequestContext(
      const scoped_refptr<Request>& incoming_request)
  : incoming_request_(incoming_request) {
}

UserAgent::IncomingRequestContext::~IncomingRequestContext() {
}

UserAgent::OutgoingRequestContext::OutgoingRequestContext(
      const scoped_refptr<Request>& outgoing_request)
  : outgoing_request_(outgoing_request) {
}

UserAgent::OutgoingRequestContext::~OutgoingRequestContext() {
}

UserAgent::UserAgent(AuthHandlerFactory *auth_handler_factory,
                     const net::BoundNetLog &net_log)
  : auth_handler_factory_(auth_handler_factory),
    net_log_(net_log) {
}

UserAgent::~UserAgent() {
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
  
  // Add the From header and a local tag (48-bit random string)
  scoped_ptr<From> from(new From(from_uri));
  from->set_tag(CreateTag());
  request->push_back(from.PassAs<Header>());
  
  // The Call-ID is formed by a 120-bit random string
  scoped_ptr<CallId> call_id(new CallId(CreateCallId()));
  request->push_back(call_id.PassAs<Header>());
  
  // Cseq always contain the request method and a new (random) local sequence
  if (local_sequence == 0) {
    local_sequence = Create16BitRandomInteger();
    if (local_sequence == 0)
      local_sequence = 1;
  }

  scoped_ptr<Cseq> cseq(new Cseq(local_sequence, method));
  request->push_back(cseq.PassAs<Header>());

  // Max-Forwards header field is always 70.
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  request->push_back(max_forwards.PassAs<Header>());
  return request;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  if (isa<Response>(message)) {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    HandleDialogStateOnResponse(response);
  }
  return network_layer_->Send(message, callback);
}

scoped_refptr<Dialog> UserAgent::HandleDialogStateOnResponse(
    const scoped_refptr<Response> &response) {
  scoped_refptr<Dialog> dialog;
  scoped_refptr<Request> request(response->refer_to());
  int response_code = response->response_code();
  // Create dialog on response_code > 100 for INVITE requests
  if (Method::INVITE == request->method()
      && response_code > 100
      && response->get<To>()->HasTag()) {
    DialogMapType::iterator i;
    tie(dialog, i) = GetDialog(response);
    if (!dialog) {
      switch (response_code/100) {
        case 1:
        case 2:
          dialog = Dialog::Create(response);
          dialogs_.insert(std::make_pair(dialog->id(), dialog));
          break;
      }
    }
    else {
      switch (response_code/100) {
        case 1:
          break;
        case 2:
          dialog->set_state(Dialog::STATE_CONFIRMED);
          break;
        default:
          dialog->set_state(Dialog::STATE_TERMINATED);
          dialogs_.erase(i);
          break;
      }
    }
  }
  // Terminate UAC dialog on response_code 2xx for BYE requests
  else if (Method::BYE == request->method()
           && response_code/100 == 2) {
    DialogMapType::iterator i;
    tie(dialog, i) = GetDialog(response);
    if (dialog) {
      dialog->set_state(Dialog::STATE_TERMINATED);
      dialogs_.erase(i);
    }
  }
  return dialog;
}

scoped_refptr<Dialog> UserAgent::HandleDialogStateOnError(
    const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog;
  if (Message::Outgoing == request->direction()) {
    // UAC timeout or transport error
    DialogMapType::iterator i;
    tie(dialog, i) = GetDialog(request);
    if (dialog) {
      dialog->set_state(Dialog::STATE_TERMINATED);
      dialogs_.erase(i);
    }
  }
  return dialog;
}

bool UserAgent::HandleChallengeAuthentication(
    const scoped_refptr<Response> &response) {
  int response_code = response->response_code();
  if (SIP_UNAUTHORIZED != response_code
      && SIP_PROXY_AUTHENTICATION_REQUIRED != response_code)
    return false;
  scoped_refptr<Request> request(response->refer_to());
  OutgoingRequestMap::iterator i = outgoing_requests_.find(request->id());
  if (outgoing_requests_.end() == i)
    return false;
  if (!i->second.auth_controller_.get()) {
    i->second.auth_controller_ = new AuthController(&auth_cache_,
        auth_handler_factory_);
  }
  int rv = i->second.auth_controller_->HandleAuthChallenge(response, net_log_);
  if (net::OK != rv)
    return false;
  if (!i->second.auth_controller_->HaveAuthHandler())
    return false;
  if (i->second.auth_controller_->auth_info()) {
    // TODO: collect user credentials here
  }
  // TODO: handle asynchronous cases here
  rv = i->second.auth_controller_->AddAuthorizationHeaders(
      request, net::CompletionCallback(), net_log_);
  if (net::OK != rv)
    return false;
  // TODO: reissue the request + authentication tokens on the network
  rv = network_layer_->Send(request, net::CompletionCallback());
  // TODO: get the callback set in the Send method in case of errors
  return true;
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
  scoped_refptr<Dialog> dialog = GetDialog(request).first;
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingRequest(request, dialog);
  }
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  if (HandleChallengeAuthentication(response))
    return;
  scoped_refptr<Dialog> dialog = HandleDialogStateOnResponse(response);
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingResponse(response, dialog);
  }
}

void UserAgent::OnTimedOut(const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog = HandleDialogStateOnError(request);
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTimedOut(request, dialog);
  }
}

void UserAgent::OnTransportError(
    const scoped_refptr<Request> &request, int err) {
  scoped_refptr<Dialog> dialog = HandleDialogStateOnError(request);
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTransportError(request, err, dialog);
  }
}

} // End of ua namespace
} // End of sippet namespace
