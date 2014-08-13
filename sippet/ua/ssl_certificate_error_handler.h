// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace sippet {

// This is the interface for the application-specific class that will route
// SSL certificate errors to the user and ask his action to continue or dismiss
// the connection attempt.
class SSLCertificateErrorHandler {
 public:
  class Factory {
   public:
    virtual ~Factory() {}

    // Returns the application-specific |SSLCertificateErrorHandler|
    // implementation.
    virtual scoped_ptr<SSLCertificateErrorHandler>
        CreateSSLCertificateErrorHandler() = 0;
  };

  virtual ~SSLCertificateErrorHandler() {}

  // Checks if the SSL information returned by the server is acceptable. If the
  // application has to display a UI dialog to the user, this function shall
  // forward the request to the UI thread and return |ERR_IO_PENDING|, calling
  // the provided callback when done. Otherwise, this function must return
  // |OK|, and the parameter |is_accepted| must have been set.
  virtual int GetUserApproval(const EndPoint &destination,
                              const net::SSLInfo &ssl_info,
                              bool *is_accepted,
                              const net::CompletionCallback& callback) = 0;

  // Gets a SSL client certificate when the server-side asks for it. If the
  // application has to display a UI dialog to the user, this function shall
  // forward the request to the UI thread and return |ERR_IO_PENDING|, calling
  // the provided callback when done. Otherwise, this function must return
  // |OK|, and the parameter |client_cert| must have been set.
  virtual int GetClientCert(const EndPoint &destination,
                            const net::SSLInfo &ssl_info,
                            scoped_refptr<net::X509Certificate> *client_cert,
                            const net::CompletionCallback& callback) = 0;
};

} // namespace sippet
