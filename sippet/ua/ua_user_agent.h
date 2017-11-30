// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_USER_AGENT_H_
#define SIPPET_UA_USER_AGENT_H_

#include "base/observer_list.h"
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
  public NetworkLayer::Delegate {
 public:
  class Observer {
   public:
    virtual ~Observer() {}

    // A new channel has connected (or not). This function is always called, no
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
    virtual void OnTimedOut(const scoped_refptr<Request> &request) = 0;

    // While sending the request or receiving a response, a network error
    // happened.
    virtual void OnTransportError(const scoped_refptr<Request> &request,
        int error) = 0;
  };

  // Construct a |UserAgent|.
  UserAgent(AuthHandlerFactory *auth_handler_factory,
            PasswordHandler::Factory *password_handler_factory,
            DialogController *dialog_controller,
            const net::NetLogWithSource &net_log);
  ~UserAgent() override;

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

  // Adds/Removes an User Agent observer.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Creates a minimum valid SIP request, containing |To|, |From|, |Cseq|,
  // |CallId| and |MaxForwards|. Remember that the topmost |Via| header is
  // set by the network layer, after sending the request (or after connecting),
  // therefore this method doesn't add any |Via| header.
  scoped_refptr<Request> CreateRequest(
      const Method &method,
      const GURL &request_uri,
      const GURL &from,
      const GURL &to,
      unsigned local_sequence = 0,
      bool use_route_set = true);

  // Send a message throughout the nextwork layer. This function encapsulates
  // the dialog creation/destruction handling.
  int Send(
      const scoped_refptr<Message> &message,
      const net::CompletionCallback& callback);

  // Send a message throughout the network layer. If the message refers to an
  // existing dialog or creates a dialog, it will be returned on the |dialog|
  // parameter.
  int SendReturningDialog(const scoped_refptr<Message> &message,
      scoped_refptr<Dialog>* dialog, const net::CompletionCallback& callback);

  // Send a message throughout the network layer, but ignoring completion.
  // Should be used with care.
  void SendIgnoringCompletion(const scoped_refptr<Message>& message);

  // Send a message throughout the network, returning the dialog and ignoring
  // completion. Should be used with care.
  void SendReturningDialogOnly(const scoped_refptr<Message>& message,
      scoped_refptr<Dialog>* dialog);

 private:
  typedef std::vector<GURL> UrlListType;

  struct OutgoingRequestContext {
    // Holds the outgoing request instance.
    scoped_refptr<Request> challenged_request_;
    // First sent time
    base::Time parted_time_;
    // Used to manage authentication
    std::unique_ptr<AuthTransaction> auth_transaction_;
    // Used to hold the last matched dialog (when authenticating)
    scoped_refptr<Dialog> last_dialog_;
    // Used to hold the last received response
    scoped_refptr<Response> last_response_;

    OutgoingRequestContext(const scoped_refptr<Request>& challenged_request);
    ~OutgoingRequestContext();
  };

  typedef std::map<std::string, std::unique_ptr<OutgoingRequestContext>>
      OutgoingRequestMap;

  bool HandleChallengeAuthentication(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog);
  void OnAuthenticationComplete(const std::string &request_id, int rv);
  void OnResendRequestComplete(const std::string &request_id, int rv);

  // sippet::NetworkLayer::Delegate methods:
  void OnChannelConnected(const EndPoint &destination, int err) override;
  void OnChannelClosed(const EndPoint &destination) override;
  void OnIncomingRequest(const scoped_refptr<Request> &request) override;
  void OnIncomingResponse(const scoped_refptr<Response> &response) override;
  void OnTimedOut(const scoped_refptr<Request> &request) override;
  void OnTransportError(
      const scoped_refptr<Request> &request, int err) override;

  void RunUserIncomingRequestCallback(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Dialog> &dialog);
  void RunUserIncomingResponseCallback(
      const scoped_refptr<Response> &response,
      const scoped_refptr<Dialog> &dialog);
  void RunUserTransportErrorCallback(
      const scoped_refptr<Request> &request,
      int error);

  NetworkLayer *network_layer_;
  UrlListType route_set_;
  base::ObserverList<Observer> observer_list_;
  AuthCache auth_cache_;
  AuthHandlerFactory *auth_handler_factory_;
  const net::NetLogWithSource net_log_;
  PasswordHandler::Factory *password_handler_factory_;
  
  std::unique_ptr<DialogStore> dialog_store_;
  DialogController *dialog_controller_;
  OutgoingRequestMap outgoing_requests_;

  base::WeakPtrFactory<UserAgent> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(UserAgent);
};

} // End of ua namespace
} // End of sippet namespace

#endif // SIPPET_UA_USER_AGENT_H_
