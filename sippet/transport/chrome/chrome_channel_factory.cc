// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/transport/chrome/chrome_stream_channel.h"
#include "net/base/net_errors.h"
#include "net/socket/client_socket_factory.h"

namespace sippet {

ChromeChannelFactory::ChromeChannelFactory(
    net::ClientSocketFactory* client_socket_factory,
    const scoped_refptr<net::URLRequestContextGetter>& request_context_getter,
    const net::SSLConfig& ssl_config)
    : client_socket_factory_(client_socket_factory),
      request_context_getter_(request_context_getter),
      ssl_config_(ssl_config) {
  CHECK(client_socket_factory_);
}

int ChromeChannelFactory::CreateChannel(
    const EndPoint &destination,
    Channel::Delegate *delegate,
    scoped_refptr<Channel> *channel) {
  if (destination.protocol() == sippet::Protocol::TCP
      || destination.protocol() == sippet::Protocol::TLS) {
    *channel = new ChromeStreamChannel(destination, delegate,
        client_socket_factory_, request_context_getter_, ssl_config_);
    return net::OK;
  }
  return net::ERR_NOT_IMPLEMENTED;
}

} // End of sippet namespace
