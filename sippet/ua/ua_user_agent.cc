// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/ua_user_agent.h"

#include <string>

#include "base/md5.h"
#include "base/build_time.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/i18n/time_formatting.h"
#include "net/base/net_errors.h"
#include "sippet/ua/dialog.h"
#include "sippet/uri/uri.h"
#include "sippet/base/tags.h"
#include "sippet/base/sequences.h"
#include "sippet/base/stl_extras.h"
#include "sippet/ua/dialog_store.h"
#include "sippet/ua/dialog_controller.h"

namespace sippet {
namespace ua {

UserAgent::OutgoingRequestContext::OutgoingRequestContext(
      const scoped_refptr<Request>& original_request)
    : original_request_(original_request) {
  DCHECK(original_request);
}

UserAgent::OutgoingRequestContext::~OutgoingRequestContext() {
}

UserAgent::UserAgent(AuthHandlerFactory *auth_handler_factory,
    PasswordHandler::Factory *password_handler_factory,
    DialogController *dialog_controller,
    const net::BoundNetLog &net_log)
    : auth_handler_factory_(auth_handler_factory),
      net_log_(net_log),
      password_handler_factory_(password_handler_factory),
      weak_factory_(this),
      dialog_store_(new DialogStore),
      dialog_controller_(dialog_controller) {
  DCHECK(auth_handler_factory);
  DCHECK(password_handler_factory);
  DCHECK(dialog_controller_);
}

UserAgent::~UserAgent() {
  STLDeleteContainerPairSecondPointers(
      outgoing_requests_.begin(), outgoing_requests_.end());
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
  request->push_back(to.Pass());

  // Add the From header and a local tag (48-bit random string)
  scoped_ptr<From> from(new From(from_uri));
  from->set_tag(CreateTag());
  request->push_back(from.Pass());

  // The Call-ID is formed by a 120-bit random string
  scoped_ptr<CallId> call_id(new CallId(CreateCallId()));
  request->push_back(call_id.Pass());

  // Cseq always contain the request method and a new (random) local sequence
  if (local_sequence == 0) {
    local_sequence = Create16BitRandomInteger();
    if (local_sequence == 0)  // Avoiding zero for a question of aesthetics
      local_sequence = 1;
  }

  scoped_ptr<Cseq> cseq(new Cseq(local_sequence, method));
  request->push_back(cseq.Pass());

  // Max-Forwards header field is always 70.
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  request->push_back(max_forwards.Pass());

  scoped_ptr<Supported> supported(new Supported);
  supported->push_back("path");
  supported->push_back("outbound");
  request->push_back(supported.Pass());

  std::string contact_address("sip:");
  contact_address += "domain.invalid";
  scoped_ptr<Contact> contact(new Contact(GURL(contact_address)));
  // Now uses the build time to generate a single instance ID
  base::string16 build_time =
    base::TimeFormatShortDateAndTime(base::GetBuildTime());
  std::string instance = base::MD5String(base::UTF16ToUTF8(build_time));
  std::string instance_id = base::StringPrintf("\"<urn:uuid:%s-%s-%s-%s-%s>\"",
    instance.substr(0, 8).c_str(),
    instance.substr(8, 4).c_str(),
    instance.substr(12, 4).c_str(),
    instance.substr(16, 4).c_str(),
    instance.substr(20, 12).c_str());
  contact->front().param_set("+sip.instance", instance_id);
  if (Method::REGISTER == method) {
    contact->front().param_set("reg-id", "1");
  }
  request->push_back(contact.Pass());

  // Adds the Route-Set, case it exists
  if (route_set_.size() > 0) {
    SipURI first_uri(route_set_.front().spec());
    if (first_uri.is_valid()) {
      scoped_ptr<Route> route(new Route);
      if (first_uri.parameter("lr").first) {
        for (UrlListType::iterator i = route_set_.begin(),
             ie = route_set_.end(); i != ie; i++) {
          route->push_back(RouteParam(*i));
        }
      } else {
        if (route_set_.size() > 1) {
          for (UrlListType::iterator i = route_set_.begin() + 1,
               ie = route_set_.end(); i != ie; i++) {
            route->push_back(RouteParam(*i));
          }
        }
        route->push_back(RouteParam(request->request_uri()));
        // Removed some invalid parameters
        request->set_request_uri(
          GURL(first_uri.GetWithEmptyHeaders().spec()));
      }
      request->push_back(route.Pass());
    }
  }

  return request;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  if (isa<Response>(message)) {
    scoped_refptr<Response> response = dyn_cast<Response>(message);
    dialog_controller_->HandleResponse(dialog_store_.get(), response);
  } else {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    dialog_controller_->HandleRequest(dialog_store_.get(), request);
  }
  return network_layer_->Send(message, callback);
}

bool UserAgent::HandleChallengeAuthentication(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  int response_code = incoming_response->response_code();
  if (SIP_UNAUTHORIZED != response_code
      && SIP_PROXY_AUTHENTICATION_REQUIRED != response_code)
    return false;
  scoped_refptr<Request> original_request(incoming_response->refer_to());
  if (nullptr == original_request)
    return false;
  OutgoingRequestContext *outgoing_request_context;
  OutgoingRequestMap::iterator i = outgoing_requests_.find(
      original_request->id());
  if (outgoing_requests_.end() == i) {
    outgoing_request_context = new OutgoingRequestContext(original_request);
    outgoing_request_context->auth_transaction_.reset(
        new AuthTransaction(&auth_cache_, auth_handler_factory_,
            password_handler_factory_, net_log_));
    outgoing_requests_.insert(std::make_pair(original_request->id(),
        outgoing_request_context));
  } else {
    outgoing_request_context = i->second;
    original_request = outgoing_request_context->original_request_;
  }
  outgoing_request_context->last_dialog_ = dialog;
  outgoing_request_context->last_response_ = incoming_response;
  scoped_refptr<Request> outgoing_request(original_request->CloneRequest());
  if (dialog) {
    // Update the dialog sequence for each authenticated request
    Message::iterator i = outgoing_request->find_first<Cseq>();
    if (outgoing_request->end() != i) {
      dialog->set_local_sequence(dyn_cast<Cseq>(i)->sequence());
    }
  }
  outgoing_request_context->outgoing_requests_.push_back(outgoing_request);
  AuthTransaction *auth_transaction =
      outgoing_request_context->auth_transaction_.get();
  int rv = auth_transaction->HandleChallengeAuthentication(outgoing_request,
      incoming_response, base::Bind(&UserAgent::OnAuthenticationComplete,
          weak_factory_.GetWeakPtr(), original_request->id()));
  if (net::ERR_IO_PENDING == rv) {
    return true;
  } else if (net::OK == rv) {
    OnAuthenticationComplete(original_request->id(), rv);
    return true;
  }
  return false;
}

void UserAgent::OnAuthenticationComplete(
    const std::string &request_id, int rv) {
  DCHECK_NE(rv, net::ERR_IO_PENDING);
  OutgoingRequestMap::iterator i = outgoing_requests_.find(request_id);
  if (outgoing_requests_.end() == i)
    return;
  OutgoingRequestContext *outgoing_request_context = i->second;
  if (net::OK == rv) {
    // Remove topmost Via header
    scoped_refptr<Request> current_outgoing_request =
        outgoing_request_context->outgoing_requests_.back();
    Message::iterator j =
        current_outgoing_request->find_first<Via>();
    if (current_outgoing_request->end() != j) {
      current_outgoing_request->erase(j);
    }
    rv = network_layer_->Send(current_outgoing_request,
        base::Bind(&UserAgent::OnResendRequestComplete,
            weak_factory_.GetWeakPtr(), request_id));
    if (net::ERR_IO_PENDING != rv) {
      OnResendRequestComplete(request_id, rv);
    }
  } else {
    RunUserIncomingResponseCallback(outgoing_request_context->last_response_,
        outgoing_request_context->last_dialog_);
  }
}

void UserAgent::OnResendRequestComplete(
    const std::string &request_id, int rv) {
  DCHECK_NE(rv, net::ERR_IO_PENDING);
  OutgoingRequestMap::iterator i = outgoing_requests_.find(request_id);
  if (outgoing_requests_.end() == i)
    return;
  OutgoingRequestContext *outgoing_request_context = i->second;
  if (net::OK != rv) {
    RunUserTransportErrorCallback(outgoing_request_context->original_request_,
        rv, outgoing_request_context->last_dialog_);
  }
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
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleRequest(dialog_store_.get(), request);
  RunUserIncomingRequestCallback(request, dialog);
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleResponse(dialog_store_.get(), response);
  if (HandleChallengeAuthentication(response, dialog))
    return;
  if (200 <= response->response_code()
      && nullptr != response->refer_to()) {
    OutgoingRequestMap::iterator i =
        outgoing_requests_.find(response->refer_to()->id());
    if (outgoing_requests_.end() != i) {
      OutgoingRequestContext *outgoing_request_context = i->second;
      // Remove the original request first
      outgoing_requests_.erase(
          outgoing_request_context->original_request_->id());
      // Remove children authenticating requests
      std::vector<scoped_refptr<Request> >& outgoing_requests =
          outgoing_request_context->outgoing_requests_;
      for (std::vector<scoped_refptr<Request> >::iterator j =
          outgoing_requests.begin(); j != outgoing_requests.end(); ++j) {
        outgoing_requests_.erase((*j)->id());
      }
      // Finally delete context
      delete outgoing_request_context;
    }
  }
  RunUserIncomingResponseCallback(response, dialog);
}

void UserAgent::OnTimedOut(const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleRequestError(dialog_store_.get(), request);
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTimedOut(request, dialog);
  }
}

void UserAgent::OnTransportError(
    const scoped_refptr<Request> &request, int err) {
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleRequestError(dialog_store_.get(), request);
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTransportError(request, err, dialog);
  }
}

void UserAgent::RunUserIncomingRequestCallback(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingRequest(request, dialog);
  }
}

void UserAgent::RunUserIncomingResponseCallback(
    const scoped_refptr<Response> &response,
    const scoped_refptr<Dialog> &dialog) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnIncomingResponse(response, dialog);
  }
}

void UserAgent::RunUserTransportErrorCallback(
      const scoped_refptr<Request> &request,
      int error,
      const scoped_refptr<Dialog> &dialog) {
  for (std::vector<Delegate*>::iterator i = handlers_.begin();
       i != handlers_.end(); i++) {
    (*i)->OnTransportError(request, error, dialog);
  }
}

}  // namespace ua
}  // namespace sippet
