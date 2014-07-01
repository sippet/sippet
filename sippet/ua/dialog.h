// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_DIALOG_H_
#define SIPPET_UA_DIALOG_H_

#include "sippet/message/method.h"
#include "sippet/message/status.h"
#include "base/memory/ref_counted.h"
#include "url/gurl.h"
#include <vector>

namespace sippet {

class Request;
class Response;

namespace ua {
class UserAgent;
}

// A dialog represents a peer-to-peer SIP relationship between two user agents
// that persists for some time. Dialogs are typically used by user agents to
// facilitate management of state. Dialogs are typically not relevant to proxy
// servers. The dialog facilitates sequencing of messages between the user
// agents and proper routing of requests between both of them. The dialog
// represents a context in which to interpret SIP Transactions and Messages.
// However, a Dialog is not necessary for message processing.
//
// A dialog is identified at each User Agent with a dialog Id, which consists
// of a Call-Id value, a local tag and a remote tag. The dialog Id at each User
// Agent involved in the dialog is not the same. Specifically, the local tag at
// one User Agent is identical to the remote tag at the peer User Agent. The
// tags are opaque tokens that facilitate the generation of unique dialog Ids.
class Dialog :
  public base::RefCountedThreadSafe<Dialog> {
 public:
  // A dialog has its own state machine, the current state is determined
  // by the sequence of messages that occur on the initial dialog.
  //
  // Invite Dialog States:
  // |STATE_EARLY| --> |STATE_CONFIRMED| --> |STATE_TERMINATED|
  //
  // Other Dialog-creating Requests Dialog States (i.e. |Method::SUBSCRIBE|):
  // |STATE_CONFIRMED| --> |STATE_TERMINATED|
  enum State {
    STATE_EARLY,
    STATE_CONFIRMED,
    STATE_TERMINATED,
  };

  // The dialog state.
  State state() {
    return state_;
  }

  // Unique value used to identify the dialog.
  std::string id() {
    std::ostringstream oss;
    oss << call_id() << ":" << local_tag() << ":" << remote_tag();
    return oss.str();
  }

  // The Call-Id of the dialog.
  std::string call_id() const {
    return call_id_;
  }

  // The Local Tag of the dialog.
  std::string local_tag() const {
    return local_tag_;
  }

  // The Remote Tag of the dialog.
  std::string remote_tag() const {
    return remote_tag_;
  }

  // Used to order requests from the User Agent to its peer.
  unsigned local_sequence() const {
    return local_sequence_;
  }

  // Used to order requests from its peer to the User Agent.
  unsigned remote_sequence() const {
    return remote_sequence_;
  }

  // The address of the local party.
  GURL local_uri() const {
    return local_uri_;
  }

  // The address of the remote party.
  GURL remote_uri() const {
    return remote_uri_;
  }

  // The address from the Contact header field of the request or response or
  // refresh request or response.
  GURL remote_target() const {
    return remote_target_;
  }

  // Determines if the dialog is secure i.e. use the sips: scheme.
  bool is_secure() const {
    return is_secure_;
  }

  // An ordered list of URIs. The route set is the list of servers that need to
  // be traversed to send a request to the peer.
  const std::vector<GURL> &route_set() const {
    return route_set_;
  }

  // Create a |Request| whithin a dialog.
  scoped_refptr<Request> CreateRequest(const Method &method);

  // Create a |Response| for a request whithin a dialog.
  scoped_refptr<Response> CreateResponse(
      int response_code,
      const std::string &reason_phrase,
      const scoped_refptr<Request> &request);
  scoped_refptr<Response> CreateResponse(
      Status::Type status,
      const scoped_refptr<Request> &request);

  // Create a |Method::ACK| request for a 2xx response, passing the INVITE
  // request being acknowledged.
  scoped_refptr<Request> CreateAck(const scoped_refptr<Request> &invite);

 private:
  friend class ua::UserAgent;

  Dialog(State state,
         const std::string &call_id,
         const std::string &local_tag,
         const std::string &remote_tag,
         bool has_local_sequence,
         unsigned local_sequence,
         bool has_remote_sequence,
         unsigned remote_sequence,
         const GURL &local_uri,
         const GURL &remote_uri,
         const GURL &remote_target,
         bool is_secure,
         const std::vector<GURL> &route_set)
    : state_(state),
      call_id_(call_id),
      local_tag_(local_tag),
      remote_tag_(remote_tag),
      has_local_sequence_(has_local_sequence),
      local_sequence_(local_sequence),
      has_remote_sequence_(has_remote_sequence),
      remote_sequence_(remote_sequence),
      local_uri_(local_uri),
      remote_uri_(remote_uri),
      remote_target_(remote_target),
      is_secure_(is_secure),
      route_set_(route_set) {}

  State state_;
  std::string call_id_;
  std::string local_tag_;
  std::string remote_tag_;
  bool has_local_sequence_;
  unsigned local_sequence_;
  bool has_remote_sequence_;
  unsigned remote_sequence_;
  GURL local_uri_;
  GURL remote_uri_;
  GURL remote_target_;
  bool is_secure_;
  std::vector<GURL> route_set_;

  // Create a client |Dialog|.
  static scoped_refptr<Dialog> CreateClientDialog(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Response> &response);

  // Create a server |Dialog|.
  static scoped_refptr<Dialog> CreateServerDialog(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Response> &response);

  // Set the dialog state (UserAgent only).
  void set_state(State state) {
    state_ = state;
  }

  // Set the dialog remote sequence (UserAgent only).
  void set_remote_sequence(unsigned sequence) {
    remote_sequence_ = sequence;
  }

  // Generate a new local sequence and return it.
  unsigned GetNewLocalSequence();

  scoped_refptr<Request> CreateRequestInternal(
      const Method &method, unsigned local_sequence);

  DISALLOW_COPY_AND_ASSIGN(Dialog);
};

} // End of sippet namespace

#endif // SIPPET_UA_DIALOG_H_
