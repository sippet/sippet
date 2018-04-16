// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/ua_user_agent.h"

#include <string>

#include "base/md5.h"
#include "base/build_time.h"
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

namespace {

void OnCompleteDoNothing(int rv) {
  // do nothing
}

}  // namespace

UserAgent::OutgoingRequestContext::OutgoingRequestContext(
      const scoped_refptr<Request>& challenged_request)
    : challenged_request_(challenged_request) {
  DCHECK(challenged_request);
}

UserAgent::OutgoingRequestContext::~OutgoingRequestContext() {
}

UserAgent::UserAgent(AuthHandlerFactory *auth_handler_factory,
    PasswordHandler::Factory *password_handler_factory,
    DialogController *dialog_controller,
    const net::NetLogWithSource &net_log)
  : auth_handler_factory_(auth_handler_factory),
    net_log_(net_log),
    password_handler_factory_(password_handler_factory),
    dialog_store_(new DialogStore),
    dialog_controller_(dialog_controller),
    weak_ptr_factory_(this) {
  DCHECK(auth_handler_factory);
  DCHECK(password_handler_factory);
  DCHECK(dialog_controller_);
}

UserAgent::~UserAgent() {}

void UserAgent::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void UserAgent::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const GURL &request_uri,
    const GURL &from_uri,
    const GURL &to_uri,
    unsigned local_sequence,
    bool use_route_set) {
  scoped_refptr<Request> request(
    new Request(method, request_uri));
  std::unique_ptr<To> to(new To(to_uri));
  request->push_back(std::move(to));

  // Add the From header and a local tag (48-bit random string)
  std::unique_ptr<From> from(new From(from_uri));
  from->set_tag(CreateTag());
  request->push_back(std::move(from));

  // The Call-ID is formed by a 120-bit random string
  std::unique_ptr<CallId> call_id(new CallId(CreateCallId()));
  request->push_back(std::move(call_id));

  // Cseq always contain the request method and a new (random) local sequence
  if (local_sequence == 0) {
    local_sequence = Create16BitRandomInteger();
    if (local_sequence == 0)  // Avoiding zero for a question of aesthetics
      local_sequence = 1;
  }

  std::unique_ptr<Cseq> cseq(new Cseq(local_sequence, method));
  request->push_back(std::move(cseq));

  // Max-Forwards header field is always 70.
  std::unique_ptr<MaxForwards> max_forwards(new MaxForwards(70));
  request->push_back(std::move(max_forwards));

  if (Method::REGISTER == method) {
    std::unique_ptr<Supported> supported(new Supported);
    supported->push_back("path");
    supported->push_back("outbound");
    request->push_back(std::move(supported));
  }

  std::string contact_address("sip:");
  contact_address += "domain.invalid";
  std::unique_ptr<Contact> contact(new Contact(GURL(contact_address)));
  if (Method::REGISTER == method) {
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
    contact->front().param_set("reg-id", "1");
  }
  request->push_back(std::move(contact));

  // Adds the Route-Set, case it exists
  if (use_route_set && route_set_.size() > 0) {
    SipURI first_uri(route_set_.front().spec());
    if (first_uri.is_valid()) {
      std::unique_ptr<Route> route(new Route);
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
      request->push_back(std::move(route));
    }
  }

  return request;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  scoped_refptr<Dialog> returned_dialog;  // Discarded
  return SendReturningDialog(message, &returned_dialog, callback);
}

int UserAgent::SendReturningDialog(
    const scoped_refptr<Message> &message,
    scoped_refptr<Dialog>* dialog,
    const net::CompletionCallback& callback) {
  if (isa<Response>(message)) {
    scoped_refptr<Response> response = cast<Response>(message);
    *dialog = dialog_controller_->HandleResponse(dialog_store_.get(), response);
  } else {
    scoped_refptr<Request> request = dyn_cast<Request>(message);
    *dialog = dialog_controller_->HandleRequest(dialog_store_.get(), request);
  }
  return network_layer_->Send(message, callback);
}

void UserAgent::SendIgnoringCompletion(const scoped_refptr<Message> &message) {
  scoped_refptr<Dialog> returned_dialog;  // Discarded
  SendReturningDialog(message, &returned_dialog,
      base::Bind(&OnCompleteDoNothing));
}

void UserAgent::SendReturningDialogOnly(const scoped_refptr<Message>& message,
    scoped_refptr<Dialog>* dialog) {
  SendReturningDialog(message, dialog, base::Bind(&OnCompleteDoNothing));
}

bool UserAgent::HandleChallengeAuthentication(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  int response_code = incoming_response->response_code();
  if (SIP_UNAUTHORIZED != response_code
      && SIP_PROXY_AUTHENTICATION_REQUIRED != response_code)
    return false;
  scoped_refptr<Request> challenged_request(incoming_response->refer_to());
  if (nullptr == challenged_request)
    return false;
  OutgoingRequestContext *outgoing_request_context;
  OutgoingRequestMap::iterator i = outgoing_requests_.find(
      challenged_request->id());
  if (outgoing_requests_.end() == i) {
    outgoing_request_context = new OutgoingRequestContext(challenged_request);
    outgoing_request_context->auth_transaction_.reset(
        new AuthTransaction(&auth_cache_, auth_handler_factory_,
            password_handler_factory_, net_log_));
    outgoing_requests_.insert(std::make_pair(challenged_request->id(),
        base::WrapUnique(outgoing_request_context)));
  } else {
    outgoing_request_context = i->second.get();
    challenged_request = outgoing_request_context->challenged_request_;
  }
  outgoing_request_context->last_dialog_ = dialog;
  outgoing_request_context->last_response_ = incoming_response;
  Message::iterator j = challenged_request->find_first<Cseq>();
  if (challenged_request->end() != j) {
    Cseq *cseq = cast<Cseq>(j);
    cseq->set_sequence(cseq->sequence() + 1);
    if (dialog) {
      dialog->set_local_sequence(cseq->sequence());
    }
  }
  AuthTransaction *auth_transaction =
      outgoing_request_context->auth_transaction_.get();
  int rv = auth_transaction->HandleChallengeAuthentication(challenged_request,
      incoming_response, base::Bind(&UserAgent::OnAuthenticationComplete,
          weak_ptr_factory_.GetWeakPtr(), challenged_request->id()));
  if (net::ERR_IO_PENDING == rv) {
    return true;
  } else if (net::OK == rv) {
    OnAuthenticationComplete(challenged_request->id(), rv);
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
  OutgoingRequestContext *outgoing_request_context = i->second.get();
  if (net::OK == rv) {
    // Remove topmost Via header
    scoped_refptr<Request> current_outgoing_request =
        outgoing_request_context->challenged_request_;
    Message::iterator j =
        current_outgoing_request->find_first<Via>();
    if (current_outgoing_request->end() != j) {
      current_outgoing_request->erase(j);
    }
    rv = network_layer_->Send(current_outgoing_request,
        base::Bind(&UserAgent::OnResendRequestComplete,
            weak_ptr_factory_.GetWeakPtr(), request_id));
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
  if (net::OK != rv) {
    OutgoingRequestContext *outgoing_request_context = i->second.get();
    RunUserTransportErrorCallback(
        outgoing_request_context->challenged_request_, rv);
  }
}

void UserAgent::OnChannelConnected(const EndPoint &destination, int err) {
  for (auto& observer : observer_list_) {
    observer.OnChannelConnected(destination, err);
  }
}

void UserAgent::OnChannelClosed(const EndPoint &destination) {
  for (auto& observer : observer_list_) {
    observer.OnChannelClosed(destination);
  }
}

void UserAgent::OnIncomingRequest(
    const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleRequest(dialog_store_.get(), request);
  RunUserIncomingRequestCallback(request, dialog);
  if (!request->referred_by()) {
    scoped_refptr<Response> response;
    if (Method::CANCEL == request->method()) {
      // CANCEL requests should cancel some ongoing INVITE transaction. If not,
      // then the CANCEL request didn't match any transaction.
      response = request->CreateResponse(SIP_CALL_TRANSACTION_DOES_NOT_EXIST);
    } else if (Method::ACK != request->method()
               && Method::BYE != request->method()) {
      // By default, "405 Method Not Allowed" is returned, case all of the
      // observers refuse to handle the request.
      LOG(INFO) << "Generating (405 Method Not Allowed) for "
                << request->method().str();
      response = request->CreateResponse(SIP_METHOD_NOT_ALLOWED);
    }
    if (response)
      SendIgnoringCompletion(response);
  }
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  scoped_refptr<Dialog> dialog =
      dialog_controller_->HandleResponse(dialog_store_.get(), response);
  if (HandleChallengeAuthentication(response, dialog))
    return;
  if (nullptr != response->refer_to()) {
    OutgoingRequestMap::iterator i =
        outgoing_requests_.find(response->refer_to()->id());
    if (outgoing_requests_.end() != i) {
      OutgoingRequestContext *outgoing_request_context = i->second.get();
      // Switch the refer_to: it should point to the original request, always
      response->set_refer_to(outgoing_request_context->challenged_request_);
      if (200 <= response->response_code()) {  // Erase context
        // Remove the original request first
        outgoing_requests_.erase(
            outgoing_request_context->challenged_request_->id());
      }
    }
  }
  RunUserIncomingResponseCallback(response, dialog);
}

void UserAgent::OnTimedOut(const scoped_refptr<Request> &request) {
  for (auto& observer : observer_list_) {
    observer.OnTimedOut(request);
  }
}

void UserAgent::OnTransportError(
    const scoped_refptr<Request> &request, int err) {
  RunUserTransportErrorCallback(request, err);
}

void UserAgent::RunUserIncomingRequestCallback(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) {
  for (auto& observer : observer_list_) {
    observer.OnIncomingRequest(request, dialog);
  }
}

void UserAgent::RunUserIncomingResponseCallback(
    const scoped_refptr<Response> &response,
    const scoped_refptr<Dialog> &dialog) {
  for (auto& observer : observer_list_) {
    observer.OnIncomingResponse(response, dialog);
  }
}

void UserAgent::RunUserTransportErrorCallback(
      const scoped_refptr<Request> &request,
      int error) {
  for (auto& observer : observer_list_) {
    observer.OnTransportError(request, error);
  }
}

}  // namespace ua
}  // namespace sippet
