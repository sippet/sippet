// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_WS_CHANNEL_FACTORY_H_
#define SIPPET_TRANSPORT_CHROME_WS_CHANNEL_FACTORY_H_

#include "sippet/transport/channel_factory.h"

namespace sippet {

class WsChannelFactory : public ChannelFactory {
 public:
  virtual int CreateChannel(
    const EndPoint &destination,
    Channel::Delegate *delegate,
    scoped_refptr<Channel> *channel) OVERRIDE;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_WS_CHANNEL_FACTORY_H_
