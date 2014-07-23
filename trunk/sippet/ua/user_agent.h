// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_USER_AGENT_H_
#define SIPPET_UA_USER_AGENT_H_

#include "sippet/transport/network_layer.h"
#include "sippet/ua/dialog.h"
#include "sippet/ua/user_agent_settings.h"
#include "sippet/ua/auth_controller.h"
#include "sippet/ua/auth_cache.h"
#include "sippet/ua/auth_handler.h"

#include <map>
#include <vector>

namespace sippet {

class To;
class From;
class CallId;
class Cseq;
class Via;
class Message;
class Request;
class Response;

namespace ua {

class UserAgent :
  public base::RefCountedThreadSafe<UserAgent>,
  public NetworkLayer::Delegate {
 private:
  DISALLOW_COPY_AND_ASSIGN(UserAgent);
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    virtual void OnChannelConnected(const EndPoint &destination, int err) = 0;

    virtual void OnChannelClosed(const EndPoint &destination) = 0;

    virtual void OnIncomingRequest(
        const scoped_refptr<Request> &incoming_request,
        const scoped_refptr<Dialog> &dialog) = 0;

    virtual void OnIncomingResponse(
        const scoped_refptr<Response> &incoming_response,
        const scoped_refptr<Dialog> &dialog) = 0;

    virtual void OnTimedOut(
        const scoped_refptr<Request> &request,
        const scoped_refptr<Dialog> &dialog) = 0;

    virtual void OnTransportError(
        const scoped_refptr<Request> &request, int error,
        const scoped_refptr<Dialog> &dialog) = 0;
  };

  // Construct a |UserAgent|.
  UserAgent(AuthHandlerFactory *auth_handler_factory,
            const net::BoundNetLog &net_log
            /*const UserAgentSettings &user_agent_settings = UserAgentSettings()*/);
  virtual ~UserAgent() {}

  void SetNetworkLayer(NetworkLayer *network_layer) {
    network_layer_ = network_layer;
  }

  // The route set is the list of servers that need to be traversed to send
  // requests out of a dialog to peers. For outbound proxies, this list is
  // constituted by a single URI.
  void set_route_set(const std::vector<GURL> &route_set) {
    route_set_.assign(route_set.begin(), route_set.end());
  }
  const std::vector<GURL> &route_set() {
    return route_set_;
  }

  // Append an User Agent handler. Handlers receive events in the same order
  // they were registered.
  void AppendHandler(Delegate *delegate);

  // Creates a minimum valid SIP request, containing |To|, |From|, |Cseq|,
  // |CallId| and |MaxForwards|. Remember that the topmost |Via| header is
  // set by the network layer, after sending the request (or after connecting),
  // therefore this method doesn't add any |Via| header.
  scoped_refptr<Request> CreateRequest(
      const Method &method,
      const GURL &request_uri,
      const GURL &from,
      const GURL &to,
      unsigned local_sequence=0);

  // Send a message throughout the nextwork layer. This function encapsulates
  // the dialog creation/destruction handling.
  int Send(
      const scoped_refptr<Message> &message,
      const net::CompletionCallback& callback);

 private:
  typedef std::vector<GURL> UrlListType;
  typedef std::vector<Delegate*> HandlerListType;
  typedef std::map<std::string, scoped_refptr<Dialog> > DialogMapType;

  NetworkLayer *network_layer_;
  UrlListType route_set_;
  HandlerListType handlers_;
  DialogMapType dialogs_;
  AuthCache auth_cache_;
  AuthHandlerFactory *auth_handler_factory_;
  net::BoundNetLog net_log_;

  // TODO
  struct IncomingRequestContext {
    // Holds the incoming request instance.
    scoped_refptr<Request> incoming_request_;
    // Arrival time
    base::Time arrival_time_;
    // Used to send a final automatic response if there's no answer in
    // a reasonable time.
    base::OneShotTimer<NetworkLayer> timer_;
  };

  std::map<std::string, IncomingRequestContext> incoming_requests_;

  struct OutgoingRequestContext {
    // Holds the outgoing request instance.
    scoped_refptr<Request> outgoing_request_;
    // First sent time
    base::Time parted_time_;
    // Used to manage authentication
    scoped_refptr<AuthController> auth_controller_;
  };

  typedef std::map<std::string, OutgoingRequestContext>
      OutgoingRequestMap;
  OutgoingRequestMap outgoing_requests_;

  // Create a local tag
  static std::string CreateTag();

  // Create an unique Call-ID
  static std::string CreateCallId();

  scoped_refptr<Dialog> HandleDialogStateOnResponse(
      const scoped_refptr<Response> &response);
  scoped_refptr<Dialog> HandleDialogStateOnError(
      const scoped_refptr<Request> &request);
  bool HandleChallengeAuthentication(
      const scoped_refptr<Response> &response);

  template <class Message>
  std::pair<scoped_refptr<Dialog>, DialogMapType::iterator>
      GetDialog(const scoped_refptr<Message> &message) {
    std::string id(message->GetDialogId());
    DialogMapType::iterator i = dialogs_.find(id);
    return dialogs_.end() == i
        ? std::make_pair(scoped_refptr<Dialog>(), i)
        : std::make_pair(i->second, i);
  }

  // sippet::NetworkLayer::Delegate methods:
  virtual void OnChannelConnected(
      const EndPoint &destination, int err) OVERRIDE;
  virtual void OnChannelClosed(const EndPoint &destination) OVERRIDE;
  virtual void OnIncomingRequest(
      const scoped_refptr<Request> &request) OVERRIDE;
  virtual void OnIncomingResponse(
      const scoped_refptr<Response> &response) OVERRIDE;
  virtual void OnTimedOut(const scoped_refptr<Request> &request) OVERRIDE;
  virtual void OnTransportError(
      const scoped_refptr<Request> &request, int err) OVERRIDE;
};

} // End of ua namespace
} // End of sippet namespace

#endif // SIPPET_UA_USER_AGENT_H_
