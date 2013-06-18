// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CONNECTION_FACTORY_H_
#define SIPPET_TRANSPORT_CONNECTION_FACTORY_H_

#include "base/memory/scoped_ptr.h"
#include "sippet/transport/end_point.h"
#include "net/base/net_log.h"

namespace sippet {

class Connection;

class ConnectionFactory {
 public:
  // Starts listening on a local address.
  virtual int Listen(const EndPoint &endpoint) = 0;

  // Stops listening on a local address.
  virtual int StopListen(const EndPoint &endpoint) = 0;

  // Creates a new connection to the given destination.
  virtual void CreateConnection(
    const EndPoint &endpoint,
    const net::NetLog *netlog,
    scoped_refptr<Connection> *connection) = 0;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CONNECTION_FACTORY_H_
