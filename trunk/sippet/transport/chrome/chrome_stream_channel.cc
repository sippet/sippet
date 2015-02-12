// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_channel.h"

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/http/http_network_session.h"
#include "net/socket/client_socket_handle.h"
#include "net/socket/client_socket_factory.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/socket/ssl_client_socket.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/ssl/ssl_cert_request_info.h"
#include "sippet/message/message.h"

namespace sippet {

// This number will couple with quite long SIP messages
const size_t kReadBufSize = 64U * 1024U;

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
          pac_request_(nullptr),
          destination_(destination),
          // Assume that we intend to do TLS on this socket; that means that
          // if a proxy is found, then CONNECT will be used on it first.
          proxy_url_("https://" + destination.hostport().ToString()),
          tried_direct_connect_fallback_(false),
          bound_net_log_(
              net::BoundNetLog::Make(
                  request_context_getter->GetURLRequestContext()->net_log(),
                  net::NetLog::SOURCE_SOCKET)),
          weak_ptr_factory_(this),
          dest_host_port_pair_(destination.host(), destination.port()),
          delegate_(delegate),
          is_connecting_(false),
          request_context_getter_(request_context_getter),
          client_socket_factory_(client_socket_factory) {
  DCHECK(request_context_getter.get());
  net::URLRequestContext* request_context =
      request_context_getter->GetURLRequestContext();
  DCHECK(request_context);
  DCHECK(!dest_host_port_pair_.host().empty());
  DCHECK_GT(dest_host_port_pair_.port(), 0);
  DCHECK(proxy_url_.is_valid());
  DCHECK(delegate_);
  
  net::HttpNetworkSession::Params session_params;
  session_params.client_socket_factory = client_socket_factory;
  session_params.host_resolver = request_context->host_resolver();
  session_params.cert_verifier = request_context->cert_verifier();
  session_params.transport_security_state =
      request_context->transport_security_state();
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
    net::IPEndPoint ip_endpoint;
    int rv = transport_->socket()->GetLocalAddress(&ip_endpoint);
    if (net::OK != rv)
      return rv;
    *origin = EndPoint(net::HostPortPair::FromIPEndPoint(ip_endpoint),
        destination_.protocol());
    return net::OK;
  }
  NOTREACHED() << "not connected";
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
  DCHECK(!is_connecting_);

  tried_direct_connect_fallback_ = false;
 
  // First we try and resolve the proxy.
  int status = network_session_->proxy_service()->ResolveProxy(
      proxy_url_, 0,
      &proxy_info_,
      proxy_resolve_callback_,
      &pac_request_,
      nullptr,
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
                   weak_ptr_factory_.GetWeakPtr(), status));
  }

  is_connecting_ = true;
}

int ChromeStreamChannel::ReconnectIgnoringLastError() {
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeStreamChannel::DoTcpConnect,
                  weak_ptr_factory_.GetWeakPtr()));
  return net::ERR_IO_PENDING;
}

int ChromeStreamChannel::ReconnectWithCertificate(
    net::X509Certificate* client_cert) {
  ssl_config_.send_client_cert = true;
  ssl_config_.client_cert = client_cert;
  network_session_->ssl_client_auth_cache()->Add(
      destination_.hostport(), client_cert);
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeStreamChannel::DoTcpConnect,
                  weak_ptr_factory_.GetWeakPtr()));
  return net::ERR_IO_PENDING;
}

int ChromeStreamChannel::Send(const scoped_refptr<Message> &message,
        const net::CompletionCallback& callback) {
  if (transport_.get() && transport_->socket()) {
    scoped_refptr<net::StringIOBuffer> string_buffer =
        new net::StringIOBuffer(message->ToString());
    return stream_writer_->Write(
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
  if (stream_writer_.get()) {
    stream_writer_->CloseWithError(err);
  }
}

void ChromeStreamChannel::DetachDelegate() {
  delegate_ = nullptr;
}

void ChromeStreamChannel::RunUserConnectCallback(int status) {
  DCHECK_LE(status, net::OK);
  is_connecting_ = false;
  if (delegate_)
    delegate_->OnChannelConnected(this, status);
}

void ChromeStreamChannel::RunUserChannelClosed(int status) {
  DCHECK_LE(status, net::OK);
  is_connecting_ = false;
  if (delegate_)
    delegate_->OnChannelClosed(this, status);
}

// Always runs asynchronously.
void ChromeStreamChannel::ProcessProxyResolveDone(int status) {
  pac_request_ = nullptr;

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
      // |this| may be deleted after this call.
      return;
    }
  }

  // Now that we have resolved the proxy, we need to connect.
  DoTcpConnect();
}

void ChromeStreamChannel::DoTcpConnect() {
  if (!transport_.get()) {
    transport_.reset(new net::ClientSocketHandle);
  }

  if (destination_.protocol() == Protocol::WS) {
    /* TODO
    status = net::InitSocketHandleForWebSocketRequest(
        GURL("http://" + destination_.hostport().ToString() + "/"),
        request_extra_headers, 0, net::DEFAULT_PRIORITY, network_session_,
        proxy_info_, false, false, ssl_config_, ssl_config_,
        net::kPrivacyModeDisabled, bound_net_log_, transport_.get(),
        resolution_callback_, connect_callback_);
     */
  } else {
    int status = net::InitSocketHandleForRawConnect(
        dest_host_port_pair_, network_session_.get(), proxy_info_, ssl_config_,
        ssl_config_, net::PRIVACY_MODE_DISABLED, bound_net_log_, transport_.get(),
        connect_callback_);
    if (status != net::ERR_IO_PENDING) {
      // Since this method is always called asynchronously. It is OK to call
      // ProcessConnectDone synchronously.
      ProcessConnectDone(status);
    }
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
    stream_reader_.reset(new ChromeStreamReader(transport_->socket()));
    stream_writer_.reset(new ChromeStreamWriter(transport_->socket()));
  }
  if (status != net::OK) {
    // If the connection failed, notify immediately
    RunUserConnectCallback(status);
    // |this| may be deleted after this call.
  } else if (destination_.protocol() != Protocol::TLS) {
    // If it's a normal TCP connection, start reading and report to delegate
    // that the connection was successful
    PostDoRead();
    RunUserConnectCallback(net::OK);
    // |this| may be deleted after this call.
  } else {
    // Otherwise, start TLS now
    StartTls();
  }
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
        proxy_info_.proxy_server().host_port_pair());
  }

  int rv = network_session_->proxy_service()->ReconsiderProxyAfterError(
      proxy_url_, 0, net::OK, &proxy_info_, proxy_resolve_callback_, &pac_request_,
      nullptr, bound_net_log_);
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
                   weak_ptr_factory_.GetWeakPtr(), rv));
    // Since we potentially have another try to go (trying the direct connect)
    // set the return code code to ERR_IO_PENDING.
    rv = net::ERR_IO_PENDING;
  }
  return rv;
}

void ChromeStreamChannel::ReportSuccessfulProxyConnection() {
  network_session_->proxy_service()->ReportSuccess(proxy_info_, nullptr);
}

void ChromeStreamChannel::CloseTransportSocket() {
  if (transport_.get() && transport_->socket())
    transport_->socket()->Disconnect();
  transport_.reset();
  stream_reader_.reset();
  stream_writer_.reset();
  is_connecting_ = false;
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void ChromeStreamChannel::PostDoRead() {
  base::MessageLoop* message_loop = base::MessageLoop::current();
  CHECK(message_loop);
  message_loop->PostTask(
      FROM_HERE,
      base::Bind(&ChromeStreamChannel::DoRead,
                 weak_ptr_factory_.GetWeakPtr()));
}

void ChromeStreamChannel::DoRead() {
  DCHECK(stream_reader_.get());
  int result = stream_reader_->Read(
      base::Bind(&ChromeStreamChannel::OnReadComplete,
                 weak_ptr_factory_.GetWeakPtr()));
  if (net::ERR_IO_PENDING == result)
    return;
  OnReadComplete(result);
}

void ChromeStreamChannel::OnReadComplete(int result) {
  DCHECK_NE(net::ERR_IO_PENDING, result);
  if (net::OK == result) {
    PostDoRead();
    delegate_->OnIncomingMessage(this, stream_reader_->GetIncomingMessage());
  } else if (result < 0) {
    RunUserChannelClosed(result);
    // |this| may be deleted after this call.
  }
}

void ChromeStreamChannel::StartTls() {
  DCHECK(is_connected());
  DCHECK(destination_.protocol().Equals(Protocol::TLS));
  DCHECK(stream_reader_->is_idle());

  net::SSLInfo ssl_info;
  if (!transport_->socket()->GetSSLInfo(&ssl_info)) {
    scoped_ptr<net::ClientSocketHandle> socket_handle(
        new net::ClientSocketHandle());
    socket_handle->SetSocket(transport_->PassSocket().Pass());

    net::SSLClientSocketContext context;
    context.cert_verifier =
        request_context_getter_->GetURLRequestContext()->cert_verifier();
    context.transport_security_state =
        request_context_getter_->GetURLRequestContext()->transport_security_state();

    DCHECK(context.transport_security_state);

    transport_->SetSocket(
        client_socket_factory_->CreateSSLClientSocket(socket_handle.Pass(),
            dest_host_port_pair_, ssl_config_, context).Pass());
  
    int status = transport_->socket()->Connect(
        base::Bind(&ChromeStreamChannel::ProcessSSLConnectDone,
                   weak_ptr_factory_.GetWeakPtr()));
    if (status != net::ERR_IO_PENDING) {
      base::MessageLoop* message_loop = base::MessageLoop::current();
      CHECK(message_loop);
      message_loop->PostTask(
          FROM_HERE,
          base::Bind(&ChromeStreamChannel::ProcessSSLConnectDone,
                     weak_ptr_factory_.GetWeakPtr(), status));
    }
  } else {
    // The socket is already connected, so let's continue as if it has
    // just connected without problems
    base::MessageLoop* message_loop = base::MessageLoop::current();
    CHECK(message_loop);
    message_loop->PostTask(
        FROM_HERE,
        base::Bind(&ChromeStreamChannel::ProcessSSLConnectDone,
                    weak_ptr_factory_.GetWeakPtr(), net::OK));
  }
}

int ChromeStreamChannel::HandleCertificateRequest(int result,
    net::SSLConfig* ssl_config) {
  if (ssl_config->send_client_cert) {
    // We already have performed SSL client authentication once and failed.
    return result;
  }

  DCHECK(transport_->socket());
  scoped_refptr<net::SSLCertRequestInfo> cert_request_info =
      new net::SSLCertRequestInfo;
  net::SSLClientSocket* ssl_socket =
      static_cast<net::SSLClientSocket*>(transport_->socket());
  ssl_socket->GetSSLCertRequestInfo(cert_request_info.get());

  DCHECK(network_session_);

  // If the user selected one of the certificates in client_certs or declined
  // to provide one for this server before, use the past decision
  // automatically.
  scoped_refptr<net::X509Certificate> client_cert;
  if (!network_session_->ssl_client_auth_cache()->Lookup(
          cert_request_info->host_and_port, &client_cert)) {
    return result;
  }

  // Note: |client_cert| may be NULL, indicating that the caller
  // wishes to proceed anonymously (eg: continue the handshake
  // without sending a client cert)
  //
  // Check that the certificate selected is still a certificate the server
  // is likely to accept, based on the criteria supplied in the
  // CertificateRequest message.
  const std::vector<std::string>& cert_authorities =
      cert_request_info->cert_authorities;
  if (client_cert.get() && !cert_authorities.empty() &&
      !client_cert->IsIssuedByEncoded(cert_authorities)) {
    return result;
  }

  ssl_config->send_client_cert = true;
  ssl_config->client_cert = client_cert;
  return net::OK;
}

int ChromeStreamChannel::HandleCertificateError(int result) {
  DCHECK(net::IsCertificateError(result));
  net::SSLClientSocket* ssl_socket =
      static_cast<net::SSLClientSocket*>(transport_->socket());
  DCHECK(ssl_socket);

  net::URLRequestContext *context =
      request_context_getter_->GetURLRequestContext();
  if (net::SSLClientSocket::IgnoreCertError(result,
      net::LOAD_IGNORE_ALL_CERT_ERRORS)) {
    const net::HttpNetworkSession::Params* session_params =
        context->GetNetworkSessionParams();
    if (session_params && session_params->ignore_certificate_errors)
      return net::OK;
  }

  net::SSLInfo ssl_info;
  ssl_socket->GetSSLInfo(&ssl_info);

  net::TransportSecurityState::DomainState domain_state;
  const bool fatal = context->transport_security_state() &&
      context->transport_security_state()->GetStaticDomainState(
          destination_.host(), &domain_state) &&
      domain_state.ShouldSSLErrorsBeFatal();

  delegate_->OnSSLCertificateError(this, ssl_info, fatal);
  return net::ERR_IO_PENDING;
}

bool ChromeStreamChannel::AllowCertErrorForReconnection(net::SSLConfig* ssl_config) {
  DCHECK(ssl_config);
  // The SSL handshake didn't finish, or the server closed the SSL connection.
  // So, we should restart establishing connection with the certificate in
  // allowed bad certificates in |ssl_config|.
  // See also net/http/http_network_transaction.cc HandleCertificateError() and
  // RestartIgnoringLastError().
  net::SSLClientSocket* ssl_socket =
      static_cast<net::SSLClientSocket*>(transport_->socket());
  net::SSLInfo ssl_info;
  ssl_socket->GetSSLInfo(&ssl_info);
  if (ssl_info.cert.get() == nullptr ||
      ssl_config->IsAllowedBadCert(ssl_info.cert.get(), nullptr)) {
    // If we already have the certificate in the set of allowed bad
    // certificates, we did try it and failed again, so we should not
    // retry again: the connection should fail at last.
    return false;
  }
  // Add the bad certificate to the set of allowed certificates in the
  // SSL config object.
  net::SSLConfig::CertAndStatus bad_cert;
  if (!net::X509Certificate::GetDEREncoded(ssl_info.cert->os_cert_handle(),
                                           &bad_cert.der_cert)) {
    return false;
  }
  bad_cert.cert_status = ssl_info.cert_status;
  ssl_config->allowed_bad_certs.push_back(bad_cert);
  return true;
}

void ChromeStreamChannel::ProcessSSLConnectDone(int status) {
  DCHECK_NE(status, net::ERR_IO_PENDING);
  DCHECK(stream_reader_->is_idle());

  if (net::ERR_SSL_CLIENT_AUTH_CERT_NEEDED == status) {
    // This will recover past used client certificates case it has been
    // selected by the user in the previous session.
    status = HandleCertificateRequest(status, &ssl_config_);
    if (net::OK == status) {
      DoTcpConnect();
      return;
    }
  }

  if (net::IsCertificateError(status)) {
    status = HandleCertificateError(status);
    if (net::ERR_IO_PENDING == status) {
      return;
    } else if (net::OK == status) {
      if (!transport_->socket()->IsConnectedAndIdle()) {
        if (AllowCertErrorForReconnection(&ssl_config_)) {
          // Restart connection ignoring the bad certificate.
          transport_->socket()->Disconnect();
          transport_->Reset();
          DoTcpConnect();
          return;
        }
        status = net::ERR_UNEXPECTED;
      } else if (!transport_->socket()
                 || !transport_->socket()->IsConnected()) {
        status = net::ERR_CONNECTION_FAILED;
      }
    }
  }

  if (status == net::OK)
    PostDoRead();
  RunUserConnectCallback(status);
  // |this| may be deleted after this call.
}

} // End of sippet namespace
