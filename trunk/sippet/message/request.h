// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_REQUEST_H_
#define SIPPET_MESSAGE_REQUEST_H_

#include "sippet/message/message.h"
#include "sippet/message/method.h"
#include "sippet/message/version.h"
#include "sippet/message/status_code.h"
#include "url/gurl.h"
#include "base/time/time.h"

namespace sippet {

class Request :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Request);
  virtual ~Request();
public:
  // Create a |Request|. New requests assume the |Outgoing| direction.
  Request(const Method &method,
          const GURL &request_uri,
          const Version &version = Version(2,0));

  // Every SIP request has an unique associated ID.
  const std::string &id() const { return id_; }

  Method method() const;
  void set_method(const Method &method);

  GURL request_uri() const;
  void set_request_uri(const GURL &request_uri);

  Version version() const;
  void set_version(const Version &version);

  virtual void print(raw_ostream &os) const OVERRIDE;

  // Responses can be generated from incoming requests by using this method.
  // Headers |From|, |CallId|, |CSeq|, |Via| and |To| are copied from the
  // request. If the |To| header doesn't contain a tag, then a new random one
  // is generated (32-bit random string). When a 100 (Trying) |Response| is
  // generated, any |Timestamp| header present in the |Request| is copied into
  // the |Response|. Delays are added into the |Timestamp| header of the
  // response by using the internal timestamp value of the |Request| creation.
  // By default, any |RecordRoute| header available in the |Request| is copied
  // to the generated |Response|. Created responses have always |Outgoing|
  // direction.
  scoped_refptr<Response> CreateResponse(
      int response_code,
      const std::string &reason_phrase);
  scoped_refptr<Response> CreateResponse(StatusCode code);

  // A |Method::CANCEL| request can be created from an |Method::INVITE|
  // request by calling this method. Headers |Via|, |MaxForwards|, |From|,
  // |To|, |CallId|, |Cseq| and |Route| are populated from the current request.
  int CreateCancel(scoped_refptr<Request> &cancel);

  // Get a the dialog identifier.
  std::string GetDialogId();

private:
  friend class Dialog;
  friend class Message;
  friend class ClientTransactionImpl;

  Method method_;
  std::string id_;
  GURL request_uri_;
  Version version_;
  base::Time time_stamp_;

  // Used by the parser.
  Request(const Method &method,
          const GURL &request_uri,
          Direction direction,
          const Version &version);

  // Creates an |Method::ACK| request from a |Method::INVITE| request. Intended
  // to be used by the transaction layer only. Headers |MaxForwards|, |From|,
  // |To|, |CallId|, |Cseq|, |Route| and |Via| are copied from the current
  // request. A |remote_tag| needs to collected from a |To::tag| contained on
  // a final response to the initial |Method::INVITE| request.
  int CreateAck(const std::string &remote_tag,
                scoped_refptr<Request> &ack);

  // Create a local tag for responses.
  static std::string CreateTag();

  scoped_refptr<Response> CreateResponseInternal(
      int response_code,
      const std::string &reason_phrase);

  // Intended to be used only by the dialog.
  scoped_refptr<Response> CreateResponse(
      int response_code,
      const std::string &reason_phrase,
      const std::string &remote_tag);
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_REQUEST_H_
