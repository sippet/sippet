// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/glue/ws_channel_factory.h"
#include "net/base/net_errors.h"

namespace sippet {

int WsChannelFactory::CreateChannel(
    const EndPoint &destination,
    Channel::Delegate *delegate,
    scoped_refptr<Channel> *channel) {
  // TODO
  return net::ERR_NOT_IMPLEMENTED;
}

} // End of sippet namespace
