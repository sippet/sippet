// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_channel.h"

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/http/http_network_session.h"
#include "net/socket/client_socket_handle.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "sippet/message/message.h"

namespace sippet {

ChromeStreamChannel::ChromeStreamChannel(const EndPoint& destination,
      Channel::Delegate *delegate,
      net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter,
      const net::SSLConfig& ssl_config)
        : proxy_resolve_callback_(
              base::Bind(&ChromeStreamChannel::ProcessProxyResolveDone,
                         base::Unretained(this))),
          connect_callback_(
              base::Bind(&ChromeStreamChannel::ProcessConnectDone,
                         base::Unretained(this))),
          ssl_config_(ssl_config),
          pac_request_(NULL),
          destination_(destination),
          // Assume that we intend to do TLS on this socket; all
          // current use cases do.
          proxy_url_("https://" + destination.hostport().ToString()),
          tried_direct_connect_fallback_(false),
          bound_net_log_(
              net::BoundNetLog::Make(
                  request_context_getter->GetURLRequestContext()->net_log(),
                  net::NetLog::SOURCE_SOCKET)),
          weak_factory_(this) {
  DCHECK(request_context_getter.get());
  net::URLRequestContext* request_context =
      request_context_getter->GetURLRequestContext();
  DCHECK(request_context);
  DCHECK(!dest_host_port_pair_.host().empty());
  DCHECK_GT(dest_host_port_pair_.port(), 0);
  DCHECK(proxy_url_.is_valid());

  net::HttpNetworkSession::Params session_params;
  session_params.client_socket_factory = client_socket_factory;
  session_params.host_resolver = request_context->host_resolver();
  session_params.cert_verifier = request_context->cert_verifier();
  session_params.transport_security_state =
      request_context->transport_security_state();
  session_params.server_bound_cert_service =
      request_context->server_bound_cert_service();
  session_params.proxy_service = request_context->proxy_service();
  session_params.ssl_config_service = request_context->ssl_config_service();
  session_params.http_auth_handler_factory =
      request_context->http_auth_handler_factory();
  session_params.network_delegate = request_context->network_delegate();
  session_params.http_server_properties =
      request_context->http_server_properties();
  session_params.net_log = request_context->net_log();

  const net::HttpNetworkSession::Params* reference_params =
      request_context->GetNetworkSessionParams();
  if (reference_params) {
    session_params.host_mapping_rules = reference_params->host_mapping_rules;
    session_params.ignore_certificate_errors =
        reference_params->ignore_certificate_errors;
    session_params.http_pipelining_enabled =
        reference_params->http_pipelining_enabled;
    session_params.testing_fixed_http_port =
        reference_params->testing_fixed_http_port;
    session_params.testing_fixed_https_port =
        reference_params->testing_fixed_https_port;
    session_params.trusted_spdy_proxy = reference_params->trusted_spdy_proxy;
  }

  network_session_ = new net::HttpNetworkSession(session_params);
}

ChromeStreamChannel::~ChromeStreamChannel() {
}

int ChromeStreamChannel::origin(EndPoint *origin) const {
  if (transport_.get() && transport_->socket()) {
    net::IPEndPoint endpoint;
    int rv = transport_->socket()->GetLocalAddress(&endpoint);
    if (net::OK != rv)
      return rv;
    *origin = EndPoint(net::HostPortPair::FromIPEndPoint(endpoint),
        destination_.protocol());
    return net::OK;
  }
  NOTREACHED();
  return net::ERR_SOCKET_NOT_CONNECTED;
}

const EndPoint& ChromeStreamChannel::destination() const {
  return destination_;
}

bool ChromeStreamChannel::is_secure() const {
  return Protocol::TLS == destination_.protocol();
}

bool ChromeStreamChannel::is_connected() const {
  if (!transport_.get() || !transport_->socket())
    return false;
  return transport_->socket()->IsConnected();
}

bool ChromeStreamChannel::is_stream() const {
  return true;
}

void ChromeStreamChannel::Connect() {
  // TODO: check double call to Connect
  tried_direct_connect_fallback_ = false;
 
  // First we try and resolve the proxy.
  int status = network_session_->proxy_service()->ResolveProxy(
      proxy_url_,
      &proxy_info_,
      proxy_resolve_callback_,
      &pac_request_,
      bound_net_log_);
  if (status != net::ERR_IO_PENDING) {
    // We defer execution of ProcessProxyResolveDone instead of calling it
    // directly here for simplicity. From the caller's point of view,
    // the connect always happens asynchronously.
    base::MessageLoop* message_loop = base::MessageLoop::current();
    CHECK(message_loop);
    message_loop->PostTask(
        FROM_HERE,
        base::Bind(&ChromeStreamChannel::ProcessProxyResolveDone,
                   weak_factory_.GetWeakPtr(), status));
  }
}

int ChromeStreamChannel::Send(const scoped_refptr<Message> &message,
        const net::CompletionCallback& callback) {
  if (transport_.get() && transport_->socket()) {
    scoped_refptr<net::StringIOBuffer> string_buffer =
        new net::StringIOBuffer(message->ToString());
    return transport_->socket()->Write(
        string_buffer.get(),
        string_buffer->size(),
        callback);
  }
  NOTREACHED();
  return net::ERR_SOCKET_NOT_CONNECTED;
}

void ChromeStreamChannel::Close() {
  CloseTransportSocket();
}

void ChromeStreamChannel::CloseWithError(int err) {
  // TODO
}

void ChromeStreamChannel::DetachDelegate() {
  delegate_ = 0;
}

void ChromeStreamChannel::RunUserConnectCallback(int status) {
  DCHECK_LE(status, net::OK);
  delegate_->OnChannelConnected(this, status);
}

// Always runs asynchronously.
void ChromeStreamChannel::ProcessProxyResolveDone(int status) {
  pac_request_ = NULL;

  DCHECK_NE(status, net::ERR_IO_PENDING);
  if (status == net::OK) {
    // Remove unsupported proxies from the list.
    proxy_info_.RemoveProxiesWithoutScheme(
        net::ProxyServer::SCHEME_DIRECT |
        net::ProxyServer::SCHEME_HTTP | net::ProxyServer::SCHEME_HTTPS |
        net::ProxyServer::SCHEME_SOCKS4 | net::ProxyServer::SCHEME_SOCKS5);

    if (proxy_info_.is_empty()) {
      // No proxies/direct to choose from. This happens when we don't support
      // any of the proxies in the returned list.
      status = net::ERR_NO_SUPPORTED_PROXIES;
    }
  }

  // Since we are faking the URL, it is possible that no proxies match our URL.
  // Try falling back to a direct connection if we have not tried that before.
  if (status != net::OK) {
    if (!tried_direct_connect_fallback_) {
      tried_direct_connect_fallback_ = true;
      proxy_info_.UseDirect();
    } else {
      CloseTransportSocket();
      RunUserConnectCallback(status);
      return;
    }
  }

  transport_.reset(new net::ClientSocketHandle);
  // Now that we have resolved the proxy, we need to connect.
  status = net::InitSocketHandleForRawConnect(
      dest_host_port_pair_, network_session_.get(), proxy_info_, ssl_config_,
      ssl_config_, net::kPrivacyModeDisabled, bound_net_log_, transport_.get(),
      connect_callback_);
  if (status != net::ERR_IO_PENDING) {
    // Since this method is always called asynchronously. it is OK to call
    // ProcessConnectDone synchronously.
    ProcessConnectDone(status);
  }
}

void ChromeStreamChannel::ProcessConnectDone(int status) {
  if (status != net::OK) {
    // If the connection fails, try another proxy.
    status = ReconsiderProxyAfterError(status);
    // ReconsiderProxyAfterError either returns an error (in which case it is
    // not reconsidering a proxy) or returns ERR_IO_PENDING if it is considering
    // another proxy.
    DCHECK_NE(status, net::OK);
    if (status == net::ERR_IO_PENDING)
      // Proxy reconsideration pending. Return.
      return;
    CloseTransportSocket();
  } else {
    ReportSuccessfulProxyConnection();
  }
  RunUserConnectCallback(status);
}

int ChromeStreamChannel::ReconsiderProxyAfterError(int error) {
  DCHECK(!pac_request_);
  DCHECK_NE(error, net::OK);
  DCHECK_NE(error, net::ERR_IO_PENDING);
  // A failure to resolve the hostname or any error related to establishing a
  // TCP connection could be grounds for trying a new proxy configuration.
  //
  // Why do this when a hostname cannot be resolved?  Some URLs only make sense
  // to proxy servers.  The hostname in those URLs might fail to resolve if we
  // are still using a non-proxy config.  We need to check if a proxy config
  // now exists that corresponds to a proxy server that could load the URL.
  //
  switch (error) {
    case net::ERR_PROXY_CONNECTION_FAILED:
    case net::ERR_NAME_NOT_RESOLVED:
    case net::ERR_INTERNET_DISCONNECTED:
    case net::ERR_ADDRESS_UNREACHABLE:
    case net::ERR_CONNECTION_CLOSED:
    case net::ERR_CONNECTION_RESET:
    case net::ERR_CONNECTION_REFUSED:
    case net::ERR_CONNECTION_ABORTED:
    case net::ERR_TIMED_OUT:
    case net::ERR_TUNNEL_CONNECTION_FAILED:
    case net::ERR_SOCKS_CONNECTION_FAILED:
      break;
    case net::ERR_SOCKS_CONNECTION_HOST_UNREACHABLE:
      // Remap the SOCKS-specific "host unreachable" error to a more
      // generic error code (this way consumers like the link doctor
      // know to substitute their error page).
      //
      // Note that if the host resolving was done by the SOCSK5 proxy, we can't
      // differentiate between a proxy-side "host not found" versus a proxy-side
      // "address unreachable" error, and will report both of these failures as
      // ERR_ADDRESS_UNREACHABLE.
      return net::ERR_ADDRESS_UNREACHABLE;
    default:
      return error;
  }

  if (proxy_info_.is_https() && ssl_config_.send_client_cert) {
    network_session_->ssl_client_auth_cache()->Remove(
        proxy_info_.proxy_server().host_port_pair().ToString());
  }

  int rv = network_session_->proxy_service()->ReconsiderProxyAfterError(
      proxy_url_, &proxy_info_, proxy_resolve_callback_, &pac_request_,
      bound_net_log_);
  if (rv == net::OK || rv == net::ERR_IO_PENDING) {
    CloseTransportSocket();
  } else {
    // If ReconsiderProxyAfterError() failed synchronously, it means
    // there was nothing left to fall-back to, so fail the transaction
    // with the last connection error we got.
    rv = error;
  }

  // We either have new proxy info or there was an error in falling back.
  // In both cases we want to post ProcessProxyResolveDone (in the error case
  // we might still want to fall back a direct connection).
  if (rv != net::ERR_IO_PENDING) {
    base::MessageLoop* message_loop = base::MessageLoop::current();
    CHECK(message_loop);
    message_loop->PostTask(
        FROM_HERE,
        base::Bind(&ChromeStreamChannel::ProcessProxyResolveDone,
                   weak_factory_.GetWeakPtr(), rv));
    // Since we potentially have another try to go (trying the direct connect)
    // set the return code code to ERR_IO_PENDING.
    rv = net::ERR_IO_PENDING;
  }
  return rv;
}

void ChromeStreamChannel::ReportSuccessfulProxyConnection() {
  network_session_->proxy_service()->ReportSuccess(proxy_info_);
}

void ChromeStreamChannel::CloseTransportSocket() {
  if (transport_.get() && transport_->socket())
    transport_->socket()->Disconnect();
  transport_.reset();
}

} // End of sippet namespace
