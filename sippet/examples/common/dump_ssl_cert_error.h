// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_EXAMPLES_COMMON_DUMP_SSL_CERT_ERROR_H_
#define SIPPET_EXAMPLES_COMMON_DUMP_SSL_CERT_ERROR_H_

#include "sippet/transport/ssl_cert_error_handler.h"

class DumpSSLCertError : public sippet::SSLCertErrorHandler {
 public:
  class Factory : public sippet::SSLCertErrorHandler::Factory {
   public:
    Factory(bool ignore_non_fatal);
    ~Factory() override;

    scoped_ptr<sippet::SSLCertErrorHandler>
         CreateSSLCertificateErrorHandler() override;

   private:
    bool ignore_non_fatal_;
  };

  DumpSSLCertError(bool ignore_non_fatal);
  ~DumpSSLCertError() override;

  int GetUserApproval(
      const sippet::EndPoint &destination,
      const net::SSLInfo &ssl_info,
      bool fatal,
      bool *is_accepted,
      const net::CompletionCallback& callback) override;

  int GetClientCert(
      const sippet::EndPoint &destination,
      const net::SSLInfo &ssl_info,
      scoped_refptr<net::X509Certificate> *client_cert,
      const net::CompletionCallback& callback) override;

 private:
  bool ignore_non_fatal_;
};

#endif // SIPPET_EXAMPLES_COMMON_DUMP_SSL_CERT_ERROR_H_

