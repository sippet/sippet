// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_CORE_H_
#define SIPPET_CORE_H_

#include <string>

#include "sippet/sippet_export.h"
#include "base/memory/ref_counted.h"

namespace sippet {

class Request;
class Response;

class SIPPET_EXPORT Core {
 public:
  virtual ~Core();

  // Called on a new incoming request.
  //
  // The |id| represents the server transaction identifier, created when
  // the request is received by the transaction layer.
  virtual void OnIncomingRequest(scoped_refptr<Request> request) = 0;

  // Called on a new incoming response.
  //
  // The |id| represents the client transaction identifier, obtained when
  // sending a request using |TransactionLayer::SendRequest|.
  //
  // If the |id| is zero, then the incoming response didn't pass through a
  // client transaction (i.e. 200 OK for INVITE requests).
  virtual void OnIncomingResponse(scoped_refptr<Response> response) = 0;

  // Called when a timeout is detected while trying to send a request, or while
  // trying to send an INVITE error response.
  //
  // |request| the request associated with the transaction. For client
  // requests, it's the outgoing request. For server requests, it's the
  // incoming request.
  virtual void OnTimedOut(const std::string& id) = 0;

  // Called when there's a transport error sending a request or response.
  //
  // The |id| can be a client transaction id or a server transaction id.
  virtual void OnTransportError(const std::string& id, int error) = 0;
};

}  // namespace sippet

#endif  // SIPPET_CORE_H_
