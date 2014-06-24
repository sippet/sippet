// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_USER_AGENT_H_
#define SIPPET_UA_USER_AGENT_H_

#include "sippet/transport/network_layer.h"
#include "sippet/ua/dialog.h"

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

    virtual void OnChannelConnected(const EndPoint &destination) = 0;

    virtual void OnChannelClosed(const EndPoint &destination, int err) = 0;

    virtual void OnIncomingRequest(
        const scoped_refptr<Request> &incoming_request,
        const scoped_refptr<Dialog> &dialog) = 0;

    virtual void OnIncomingResponse(
        const scoped_refptr<Response> &incoming_response,
        const scoped_refptr<Dialog> &dialog) = 0;

    virtual void OnTimedOut(const std::string &id) = 0;

    virtual void OnTransportError(const std::string &id, int error) = 0;
  };

  UserAgent();
  virtual ~UserAgent() {}

  // Append an User Agent handler. Handlers receive events in the same order
  // they were registered.
  void AppendHandler(Delegate *delegate);

  // Creates a minimum valid SIP request, containing the following header
  // fields: To, From, CSeq, Call-ID and Max-Forwards. Remember that the
  // topmost Via header is always set by the network layer, after sending the
  // request (or after connecting).
  scoped_refptr<Request> CreateRequest(
      const Method &method,
      const GURL &from,
      const GURL &to);
  scoped_refptr<Request> CreateRequest(
      const Method &method,
      const scoped_refptr<Dialog> &dialog);

  // Creates a minimum valid SIP response, containing header fields To, From,
  // CSeq, Call-ID and Via.
  scoped_refptr<Response> CreateResponse(
      int response_code,
      const scoped_refptr<Request> &request);

  // Send a message throughout the nextwork layer. This function encapsulates
  // the dialog creation/destruction handling.
  int Send(
      const scoped_refptr<Message> &message,
      const net::CompletionCallback& callback);

 private:
  int local_sequence_;
  std::vector<Delegate*> handlers_;
  std::map<std::string, scoped_refptr<Dialog> > dialogs_;

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

  // sippet::NetworkLayer::Delegate methods:
  virtual void OnChannelConnected(const EndPoint &destination) OVERRIDE;
  virtual void OnChannelClosed(const EndPoint &destination, int err) OVERRIDE;
  virtual void OnIncomingRequest(
      const scoped_refptr<Request> &request) OVERRIDE;
  virtual void OnIncomingResponse(
      const scoped_refptr<Response> &response) OVERRIDE;
  virtual void OnTimedOut(const std::string &id) OVERRIDE;
  virtual void OnTransportError(const std::string &id, int error) OVERRIDE;
};

} // End of ua namespace
} // End of sippet namespace

#endif // SIPPET_UA_USER_AGENT_H_
