// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSACTION_LAYER_H_
#define SIPPET_TRANSACTION_LAYER_H_

#include "sippet/sippet_export.h"

namespace sippet {

class SIPPET_EXPORT TransactionLayer {
 public:
  virtual ~TransactionLayer();

  // |transport| is the transport layer to be used. It must be valid.
  // |core| is stack core, and should not be destroyed before the transaction
  // layer.
  static std::unique_ptr<TransactionLayer> Create(
      std::unique_ptr<TransportLayer> transport,
      Core* core);

  // Sends a request using client transactions.
  //
  // Requests of method ACK shall be sent directly to the |TransportLayer|.
  //
  // A |ClientTransaction| is created to handle request retransmissions, when
  // the transport presumes it, and match response retransmissions, so the
  // |Core| doesn't get retransmissions other than 200 OK for INVITE requests.
  //
  // It returns the id of the client transaction.
  virtual int SendRequest(scoped_ptr<Request> request) = 0;

  // Sends a response to a server transaction.
  //
  // Server transactions are created when the incoming request is received.
  // |id| is the server transaction id.
  virtual void SendResponse(scoped_ptr<Response> response, int id) = 0;

  // Terminates a client or server transaction forcefully.
  //
  // This function is not generally executed by entities; there is a single
  // case where it is fundamental, which is when a client transaction is in
  // proceeding state for a long time, and the transaction has to be finished
  // forcibly, or it will never finish by itself.
  //
  // If a transaction with such |id| does not exist, it returns false.
  virtual bool Terminate(int id) = 0;

  // Receives a message from transport.
  //
  // If the message is a request, then it will look if a server transaction
  // already exists and route to it. Otherwise, if the request method is ACK,
  // it will redirect the request directly to |Core|; otherwise, then a new
  // |ServerTransaction| will be created.
  //
  // If the message is a response, it looks if a client transaction already
  // exists in order to handle it, and if so, redirects to it. Otherwise the
  // response is redirected directly to the |Core|. The latter is done
  // so because of the usual SIP behavior or handling the 200 OK response
  // retransmissions for requests with INVITE method directly.
  virtual void OnMessage(scoped_ptr<Message> message) = 0;

  // Receives an error from transport.
  //
  // The client and server identifiers are passed to the transport by the
  // transactions. If the transport faces an error, it has to inform the
  // transaction using this function.
  //
  // If a transaction with such a key does not exist, it will be silently
  // ignored.
  virtual void OnTransportError(int id, int error) = 0;
};

}  // namespace sippet

#endif  // SIPPET_TRANSACTION_LAYER_H_
