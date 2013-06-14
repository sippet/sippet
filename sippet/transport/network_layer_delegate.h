// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_LAYER_DELEGATE_H_
#define SIPPET_TRANSPORT_NETWORK_LAYER_DELEGATE_H_

#include "base/threading/non_thread_safe.h"
#include "net/base/net_export.h"

namespace sippet {

// Listen to the network layer
class NetworkLayerDelegate :
  public NON_EXPORTED_BASE(public base::NonThreadSafe) {
 public:
  // Called before a connection is registered.
  virtual void OnPreRegisterConnection(const EndPoint &endpoint,
                                       TransportConnection *connection) = 0;

  // Called after a connection is unregistered.
  virtual void OnPostUnregisterConnection(const EndPoint &endpoint,
                                          TransportConnection *connection) = 0;

  // Called when a message arrives.
  virtual void OnMessage(const EndPoint &endpoint, Message *msg) = 0;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_LAYER_DELEGATE_H_
