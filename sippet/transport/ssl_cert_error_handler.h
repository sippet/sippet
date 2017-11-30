// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_SSL_CERT_ERROR_HANDLER_H_
#define SIPPET_TRANSPORT_SSL_CERT_ERROR_HANDLER_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "net/base/completion_callback.h"

namespace net {
class SSLInfo;
class X509Certificate;
class SSLPrivateKey;
}  // namespace net

namespace sippet {

class EndPoint;

// This is the interface for the application-specific class that will route
// SSL certificate errors to the user and ask his action to continue or dismiss
// the connection attempt.
class SSLCertErrorHandler {
 public:
  class Factory {
   public:
    virtual ~Factory() {}

    // Returns the application-specific |SSLCertErrorHandler|
    // implementation.
    virtual std::unique_ptr<SSLCertErrorHandler>
        CreateSSLCertificateErrorHandler() = 0;
  };

  virtual ~SSLCertErrorHandler() {}

  // Checks if the SSL information returned by the server is acceptable. If the
  // application has to display a UI dialog to the user, this function shall
  // forward the request to the UI thread and return |ERR_IO_PENDING|, calling
  // the provided callback when done. Otherwise, this function must return
  // |OK|, and the parameter |is_accepted| must have been set.
  virtual int GetUserApproval(const EndPoint &destination,
                              const net::SSLInfo &ssl_info,
                              bool fatal,
                              bool *is_accepted,
                              const net::CompletionCallback& callback) = 0;

  // Gets a SSL client certificate when the server-side asks for it. If the
  // application has to display a UI dialog to the user, this function shall
  // forward the request to the UI thread and return |ERR_IO_PENDING|, calling
  // the provided callback when done. Otherwise, this function must return
  // |OK|, and the parameter |client_cert| must have been set. |private_key|
  // mey be NULL.
  virtual int GetClientCert(const EndPoint &destination,
                            const net::SSLInfo &ssl_info,
                            scoped_refptr<net::X509Certificate> *client_cert,
                            scoped_refptr<net::SSLPrivateKey> *private_key,
                            const net::CompletionCallback& callback) = 0;
};

} // namespace sippet

#endif // SIPPET_TRANSPORT_SSL_CERT_ERROR_HANDLER_H_
