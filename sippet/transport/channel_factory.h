// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHANNEL_FACTORY_H_
#define SIPPET_TRANSPORT_CHANNEL_FACTORY_H_

#include "base/memory/ref_counted.h"
#include "sippet/transport/end_point.h"
#include "sippet/transport/channel.h"

namespace sippet {

class ChannelFactory {
 public:
  // Creates a new channel to the given destination.
  virtual int CreateChannel(
    const EndPoint &destination,
    Channel::Delegate *delegate,
    scoped_refptr<Channel> *channel) = 0;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHANNEL_FACTORY_H_
