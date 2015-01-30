// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_STREAM_CHANNEL_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_STREAM_CHANNEL_H_

#include "sippet/transport/channel.h"
#include "sippet/transport/chrome/chrome_stream_writer.h"
#include "sippet/transport/chrome/chrome_stream_reader.h"
#include "base/memory/weak_ptr.h"
#include "net/proxy/proxy_info.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/ssl_config_service.h"
#include "url/gurl.h"

namespace net {
class ClientSocketFactory;
class ClientSocketHandle;
class URLRequestContextGetter;
class HttpNetworkSession;
}

namespace sippet {

class Message;

class ChromeStreamChannel : public Channel {
 public:
  ChromeStreamChannel(const EndPoint& destination,
      Channel::Delegate *delegate,
      net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter,
      const net::SSLConfig& ssl_config);

  virtual int origin(EndPoint *origin) const OVERRIDE;
  virtual const EndPoint& destination() const OVERRIDE;

  virtual bool is_secure() const OVERRIDE;
  virtual bool is_connected() const OVERRIDE;
  virtual bool is_stream() const OVERRIDE;

  virtual void Connect() OVERRIDE;
  virtual int ReconnectIgnoringLastError() OVERRIDE;
  virtual int ReconnectWithCertificate(
      net::X509Certificate* client_cert) OVERRIDE;

  virtual int Send(const scoped_refptr<Message> &message,
                   const net::CompletionCallback& callback) OVERRIDE;

  virtual void Close() OVERRIDE;

  virtual void CloseWithError(int err) OVERRIDE;

  virtual void DetachDelegate() OVERRIDE;

 private:
  friend class base::RefCountedThreadSafe<Channel>;
  virtual ~ChromeStreamChannel();

  // Proxy resolution and connection functions.
  void ProcessProxyResolveDone(int status);
  void DoTcpConnect();
  void ProcessConnectDone(int status);

  void CloseTransportSocket();
  void RunUserConnectCallback(int status);
  void RunUserChannelClosed(int status);
  int ReconsiderProxyAfterError(int error);
  void ReportSuccessfulProxyConnection();
  int HandleCertificateRequest(int result, net::SSLConfig* ssl_config);
  int HandleCertificateError(int result);
  bool AllowCertErrorForReconnection(net::SSLConfig* ssl_config);

  void PostDoRead();
  void DoRead();
  void OnReadComplete(int result);

  // TLS related functions
  void StartTls();
  void ProcessSSLConnectDone(int status);

  EndPoint destination_;
  Channel::Delegate *delegate_;

  // Callbacks passed to net APIs.
  net::CompletionCallback proxy_resolve_callback_;
  net::CompletionCallback connect_callback_;

  scoped_refptr<net::HttpNetworkSession> network_session_;
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  net::ClientSocketFactory* client_socket_factory_;

  // The transport socket.
  scoped_ptr<net::ClientSocketHandle> transport_;
  scoped_ptr<ChromeStreamReader> stream_reader_;
  scoped_ptr<ChromeStreamWriter> stream_writer_;

  bool is_connecting_;
  net::SSLConfig ssl_config_;
  net::ProxyService::PacRequest* pac_request_;
  net::ProxyInfo proxy_info_;
  const net::HostPortPair dest_host_port_pair_;
  const GURL proxy_url_;
  bool tried_direct_connect_fallback_;
  net::BoundNetLog bound_net_log_;

  base::WeakPtrFactory<ChromeStreamChannel> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStreamChannel);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_CHROME_STREAM_CHANNEL_H_
