// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_SSL_CERT_ERROR_TRANSACTION_H_
#define SIPPET_TRANSPORT_SSL_CERT_ERROR_TRANSACTION_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "net/base/completion_callback.h"
#include "net/ssl/ssl_info.h"
#include "sippet/transport/end_point.h"
#include "sippet/transport/ssl_cert_error_handler.h"

namespace net {
class X509Certificate;
class SSLPrivateKey;
}  // namespace net

namespace sippet {

// The SSLCertErrorTransaction collects client security certificates, when
// needed, and also asks for a permission to accept or reject server-side
// certificates that couldn't be verified.
class SSLCertErrorTransaction {
 public:
  SSLCertErrorTransaction(
      SSLCertErrorHandler::Factory* ssl_cert_error_handler_factory);
  virtual ~SSLCertErrorTransaction();

  // Handle a non-fatal SSL error.
  int HandleSSLCertError(const EndPoint &destination,
                         const net::SSLInfo &ssl_info,
                         bool fatal,
                         const net::CompletionCallback& callback);

  bool is_accepted() const { return is_accepted_; }
  const net::SSLInfo &ssl_info() const { return ssl_info_; }
  const EndPoint &destination() const { return destination_; }

  scoped_refptr<net::X509Certificate> client_cert() const {
    return client_cert_;
  }
  scoped_refptr<net::SSLPrivateKey> private_key() const {
    return private_key_;
  }

 private:
  enum State {
    STATE_HANDLE_SSL_CERT_ERROR,
    STATE_HANDLE_SSL_CERT_ERROR_COMPLETE,
    STATE_GET_USER_APPROVAL,
    STATE_GET_USER_APPROVAL_COMPLETE,
    STATE_GET_CLIENT_CERT,
    STATE_GET_CLIENT_CERT_COMPLETE,
    STATE_NONE,
  };

  void OnIOComplete(int result);

  int DoLoop(int last_io_result);
  int DoHandleSSLCertError();
  int DoHandleSSLCertErrorComplete();
  int DoGetUserApproval();
  int DoGetUserApprovalComplete(int result);
  int DoGetClientCert();
  int DoGetClientCertComplete(int result);

  void RunUserCallback(int status);

  State next_state_;

  EndPoint destination_;
  net::SSLInfo ssl_info_;
  SSLCertErrorHandler::Factory* ssl_cert_error_handler_factory_;
  std::unique_ptr<SSLCertErrorHandler> ssl_cert_error_handler_;
  bool fatal_;
  bool is_accepted_;
  scoped_refptr<net::X509Certificate> client_cert_;
  scoped_refptr<net::SSLPrivateKey> private_key_;
  net::CompletionCallback io_callback_;
  net::CompletionCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(SSLCertErrorTransaction);
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_SSL_CERT_ERROR_TRANSACTION_H_
