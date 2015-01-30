// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_USER_AGENT_H_
#define SIPPET_UA_USER_AGENT_H_

#include "sippet/transport/network_layer.h"
#include "sippet/ua/dialog.h"
#include "sippet/ua/auth_transaction.h"
#include "sippet/ua/auth_cache.h"

#include <map>
#include <vector>

namespace sippet {

class Message;
class Request;
class Response;
class DialogStore;
class DialogController;

namespace ua {

// An |UserAgent| can act as both a user agent client and user agent server.
//
// As client, it can create new requests, and then send it to the
// |NetworkLayer|. If the response to that request may create a dialog, the
// |DialogController| will do it and provide it upwards, in the provided
// |Delegate| implementation. Also, if that response contains an authentication
// challenge, it will authenticate using one of the available schemes provided
// by the |AuthHandlerFactory|, collecting usernames and passwords through the
// |PasswordHandler| implementation.
//
// As server, it will receive any incoming request from the network and pass
// them upwards. Provisional and success responses will automatically create
// dialogs, accordingly to the |DialogController| implementation.
//
// Multiple |Delegate| implementations may be provided to the |UserAgent|, each
// handling, or not, their own set of requests, responses and connection
// feedbacks.
class UserAgent :
  public base::RefCountedThreadSafe<UserAgent>,
  public NetworkLayer::Delegate {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // A new channel has connected (or not). This function is always called,
    // matter if the connection was succeeded or not; in the latter case, the
    // provided |err| is not |net::OK|.
    virtual void OnChannelConnected(const EndPoint &destination, int err) = 0;

    // A created channel was closed. It is called whenever a channel is created
    // successfully, therefore |OnChannelConnected| has been called with
    // |net::OK| status before the channel can be closed.
    virtual void OnChannelClosed(const EndPoint &destination) = 0;

    // A new request arrived. The dialog, when present, indicates that the
    // incoming request pertains to it.
    virtual void OnIncomingRequest(
        const scoped_refptr<Request> &incoming_request,
        const scoped_refptr<Dialog> &dialog) = 0;

    // A new response arrived. The dialog, when present, indicates that the
    // incoming response pertains to it.
    virtual void OnIncomingResponse(
        const scoped_refptr<Response> &incoming_response,
        const scoped_refptr<Dialog> &dialog) = 0;

    // The sent request didn't get a response in a reasonable time.
    virtual void OnTimedOut(
        const scoped_refptr<Request> &request,
        const scoped_refptr<Dialog> &dialog) = 0;

    // While sending the request, a network error happened.
    virtual void OnTransportError(
        const scoped_refptr<Request> &request, int error,
        const scoped_refptr<Dialog> &dialog) = 0;
  };

  // Construct a |UserAgent|.
  UserAgent(AuthHandlerFactory *auth_handler_factory,
            PasswordHandler::Factory *password_handler_factory,
            DialogController *dialog_controller,
            const net::BoundNetLog &net_log);

  // The |NetworkLayer| must be set before using the other functions.
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
  friend class base::RefCountedThreadSafe<UserAgent>;
  virtual ~UserAgent();

  typedef std::vector<GURL> UrlListType;
  typedef std::vector<Delegate*> HandlerListType;

  struct OutgoingRequestContext {
    // Holds the outgoing request instance.
    scoped_refptr<Request> original_request_;
    // Holds the outgoing request instances.
    std::vector<scoped_refptr<Request> > outgoing_requests_;
    // First sent time
    base::Time parted_time_;
    // Used to manage authentication
    scoped_ptr<AuthTransaction> auth_transaction_;
    // Used to hold the last matched dialog (when authenticating)
    scoped_refptr<Dialog> last_dialog_;
    // Used to hold the last received response
    scoped_refptr<Response> last_response_;

    OutgoingRequestContext(const scoped_refptr<Request>& original_request);
    ~OutgoingRequestContext();
  };

  typedef std::map<std::string, OutgoingRequestContext*>
      OutgoingRequestMap;

  bool HandleChallengeAuthentication(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog);
  void OnAuthenticationComplete(const std::string &request_id, int rv);
  void OnResendRequestComplete(const std::string &request_id, int rv);

  // sippet::NetworkLayer::Delegate methods:
  virtual void OnChannelConnected(
      const EndPoint &destination, int err) override;
  virtual void OnChannelClosed(const EndPoint &destination) override;
  virtual void OnIncomingRequest(
      const scoped_refptr<Request> &request) override;
  virtual void OnIncomingResponse(
      const scoped_refptr<Response> &response) override;
  virtual void OnTimedOut(const scoped_refptr<Request> &request) override;
  virtual void OnTransportError(
      const scoped_refptr<Request> &request, int err) override;

  void RunUserIncomingRequestCallback(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Dialog> &dialog);
  void RunUserIncomingResponseCallback(
      const scoped_refptr<Response> &response,
      const scoped_refptr<Dialog> &dialog);
  void RunUserTransportErrorCallback(
      const scoped_refptr<Request> &request,
      int error,
      const scoped_refptr<Dialog> &dialog);

  NetworkLayer *network_layer_;
  UrlListType route_set_;
  HandlerListType handlers_;
  AuthCache auth_cache_;
  AuthHandlerFactory *auth_handler_factory_;
  net::BoundNetLog net_log_;
  PasswordHandler::Factory *password_handler_factory_;
  base::WeakPtrFactory<UserAgent> weak_factory_;
  
  scoped_ptr<DialogStore> dialog_store_;
  DialogController *dialog_controller_;
  OutgoingRequestMap outgoing_requests_;

  DISALLOW_COPY_AND_ASSIGN(UserAgent);
};

} // End of ua namespace
} // End of sippet namespace

#endif // SIPPET_UA_USER_AGENT_H_
