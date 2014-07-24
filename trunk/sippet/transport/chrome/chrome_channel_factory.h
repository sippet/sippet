// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_SOCKET_CHANNEL_FACTORY_H_
#define SIPPET_TRANSPORT_CHROME_SOCKET_CHANNEL_FACTORY_H_

#include "sippet/transport/channel_factory.h"
#include "base/memory/ref_counted.h"
#include "net/ssl/ssl_config_service.h"
#include "net/url_request/url_request_context_getter.h"

namespace net {
class ClientSocketFactory;
}

namespace sippet {

class ChromeChannelFactory : public ChannelFactory {
 public:
  ChromeChannelFactory(net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter,
      const net::SSLConfig& ssl_config);
  ~ChromeChannelFactory();

  virtual int CreateChannel(
    const EndPoint &destination,
    Channel::Delegate *delegate,
    scoped_refptr<Channel> *channel) OVERRIDE;

 private:
  net::ClientSocketFactory* const client_socket_factory_;
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  net::SSLConfig ssl_config_;

  DISALLOW_COPY_AND_ASSIGN(ChromeChannelFactory);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_SOCKET_CHANNEL_FACTORY_H_
