// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_REQUEST_H_
#define SIPPET_MESSAGE_REQUEST_H_

#include "sippet/message/message.h"
#include "sippet/message/method.h"
#include "sippet/message/version.h"
#include "url/gurl.h"
#include "base/time/time.h"

namespace sippet {

class Request :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Request);
  virtual ~Request();
public:
  Request(const Method &method,
          const GURL &request_uri,
          const Version &version = Version(2,0));

  //! Every SIP request has an unique associated ID.
  const std::string &id() const { return id_; }

  Method method() const;
  void set_method(const Method &method);

  GURL request_uri() const;
  void set_request_uri(const GURL &request_uri);

  Version version() const;
  void set_version(const Version &version);

  virtual void print(raw_ostream &os) const OVERRIDE;

  scoped_refptr<Response> CreateResponse(int response_code,
                                         const std::string &to_tag = "");

  // A |Method::INVITE| request can be created from an |Method::INVITE|
  // request by calling this method. Headers |MaxForwards|, |From|, |To|,
  // |CallId|, |Cseq| and |Route| are populated from the current request.
  // A |remote_tag| needs to collected from a |To::tag| contained on a final
  // response to the initial |Method::INVITE| request. Note that the header
  // |Via| is not copied, therefore, if you're sending a |Method::ACK| for a
  // 2xx response, you just need to pass the request to the |NetworkLayer| and
  // it will include a new automatic |Via| header.
  int CreateAck(const std::string &remote_tag,
                scoped_refptr<Request> &ack);

  // A |Method::CANCEL| request can be created from an |Method::INVITE|
  // request by calling this method. Headers |Via|, |MaxForwards|, |From|,
  // |To|, |CallId|, |Cseq| and |Route| are populated from the current request.
  int CreateCancel(scoped_refptr<Request> &cancel);

private:
  Method method_;
  std::string id_;
  GURL request_uri_;
  Version version_;
  base::Time time_stamp_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_REQUEST_H_
