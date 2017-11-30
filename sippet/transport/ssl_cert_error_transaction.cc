// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/ssl_cert_error_transaction.h"

#include "net/base/net_errors.h"
#include "net/ssl/ssl_info.h"
#include "net/cert/x509_certificate.h"

namespace sippet {

SSLCertErrorTransaction::SSLCertErrorTransaction(
    SSLCertErrorHandler::Factory* ssl_cert_error_handler_factory)
    : ssl_cert_error_handler_factory_(ssl_cert_error_handler_factory),
      is_accepted_(false),
      io_callback_(
          base::Bind(&SSLCertErrorTransaction::OnIOComplete,
              base::Unretained(this))) {
  DCHECK(ssl_cert_error_handler_factory_);
}

SSLCertErrorTransaction::~SSLCertErrorTransaction() {
}

int SSLCertErrorTransaction::HandleSSLCertError(
    const EndPoint &destination,
    const net::SSLInfo &ssl_info,
    bool fatal,
    const net::CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  destination_ = destination;
  ssl_info_ = ssl_info;
  callback_ = callback;
  fatal_ = fatal;
  next_state_ = STATE_HANDLE_SSL_CERT_ERROR;
  return DoLoop(net::OK);
}

void SSLCertErrorTransaction::OnIOComplete(int result) {
  DCHECK_NE(STATE_NONE, next_state_);
  int rv = DoLoop(result);
  if (rv != net::ERR_IO_PENDING)
    RunUserCallback(rv);
}

int SSLCertErrorTransaction::DoLoop(int last_io_result) {
  DCHECK_NE(next_state_, STATE_NONE);
  int rv = last_io_result;
  do {
    State state = next_state_;
    next_state_ = STATE_NONE;
    switch (state) {
      case STATE_HANDLE_SSL_CERT_ERROR:
        DCHECK_EQ(net::OK, rv);
        rv = DoHandleSSLCertError();
        break;
      case STATE_HANDLE_SSL_CERT_ERROR_COMPLETE:
        DCHECK_EQ(net::OK, rv);
        rv = DoHandleSSLCertErrorComplete();
        break;
      case STATE_GET_USER_APPROVAL:
        DCHECK_EQ(net::OK, rv);
        rv = DoGetUserApproval();
        break;
      case STATE_GET_USER_APPROVAL_COMPLETE:
        rv = DoGetUserApprovalComplete(rv);
        break;
      case STATE_GET_CLIENT_CERT:
        DCHECK_EQ(net::OK, rv);
        rv = DoGetClientCert();
        break;
      case STATE_GET_CLIENT_CERT_COMPLETE:
        rv = DoGetClientCertComplete(rv);
        break;
      default:
        NOTREACHED() << "bad state";
        rv = net::ERR_UNEXPECTED;
        break;
    }
  } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);
  return rv;
}

int SSLCertErrorTransaction::DoHandleSSLCertError() {
  std::unique_ptr<SSLCertErrorHandler> ssl_cert_error_handler(
      ssl_cert_error_handler_factory_->CreateSSLCertificateErrorHandler());
  ssl_cert_error_handler_.reset(ssl_cert_error_handler.release());
  return net::OK;
}

int SSLCertErrorTransaction::DoHandleSSLCertErrorComplete() {
  if (ssl_info_.cert) {
    next_state_ = STATE_GET_USER_APPROVAL;
  } else {
    next_state_ = STATE_GET_CLIENT_CERT;
  }
  return net::OK;
}

int SSLCertErrorTransaction::DoGetUserApproval() {
  DCHECK(ssl_cert_error_handler_.get());
  next_state_ = STATE_GET_USER_APPROVAL_COMPLETE;
  return ssl_cert_error_handler_->GetUserApproval(destination_,
      ssl_info_, fatal_, &is_accepted_, io_callback_);
}

int SSLCertErrorTransaction::DoGetUserApprovalComplete(int result) {
  next_state_ = STATE_NONE;
  return net::OK;
}

int SSLCertErrorTransaction::DoGetClientCert() {
  DCHECK(ssl_cert_error_handler_.get());
  next_state_ = STATE_GET_CLIENT_CERT_COMPLETE;
  return ssl_cert_error_handler_->GetClientCert(destination_,
      ssl_info_, &client_cert_, &private_key_, io_callback_);
}

int SSLCertErrorTransaction::DoGetClientCertComplete(int result) {
  next_state_ = STATE_NONE;
  return net::OK;
}

void SSLCertErrorTransaction::RunUserCallback(int status) {
  DCHECK(!callback_.is_null());
  net::CompletionCallback c = callback_;
  callback_.Reset();
  c.Run(status);
}

}  // namespace sippet

