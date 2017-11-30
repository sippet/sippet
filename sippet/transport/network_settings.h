// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_SETTINGS_H_
#define SIPPET_TRANSPORT_NETWORK_SETTINGS_H_

#include "net/base/net_export.h"
#include "base/memory/ref_counted.h"
#include "sippet/transport/branch_factory.h"
#include "sippet/transport/transaction_factory.h"
#include "sippet/transport/ssl_cert_error_handler.h"

#include <string>

namespace sippet {

class NetworkSettings {
 public:
  NetworkSettings();
  NetworkSettings(const NetworkSettings& other);
  ~NetworkSettings();

  // Time to keep idle channels open
  int reuse_lifetime() const {
    return reuse_lifetime_;
  }
  void set_reuse_lifetime(int value) {
    reuse_lifetime_ = value;
  }

  // Whether to use compact headers in generated messages
  bool enable_compact_headers() const {
    return enable_compact_headers_;
  }
  void set_enable_compact_headers(bool value) {
    enable_compact_headers_ = value;
  }

  // Set the software name (the value added to User-Agent headers)
  std::string software_name() const {
    return software_name_;
  }
  void set_software_name(const std::string &software_name) {
    software_name_ = software_name;
  }

  // The internal branch factory to use
  BranchFactory *branch_factory() const {
    return branch_factory_;
  }
  void set_branch_factory(BranchFactory *branch_factory) {
    DCHECK(branch_factory);
    branch_factory_ = branch_factory;
  }

  // The internal transaction factory to use
  TransactionFactory *transaction_factory() const {
    return transaction_factory_;
  }
  void set_transaction_factory(TransactionFactory *transaction_factory) {
    DCHECK(transaction_factory);
    transaction_factory_ = transaction_factory;
  }

  // The SSL certificate error handler factory to use
  SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory() const {
    return ssl_cert_error_handler_factory_;
  }
  void set_ssl_cert_error_handler_factory(
      SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory) {
    DCHECK(ssl_cert_error_handler_factory);
    ssl_cert_error_handler_factory_ = ssl_cert_error_handler_factory;
  }

  // Send CRLF as connection keep-alive
  bool crlf_keep_alive() const {
    return crlf_keep_alive_;
  }
  void set_crlf_keep_alive(bool value) {
    crlf_keep_alive_ = value;
  }

  // Keep-live is performed using the following interval in seconds
  int crlf_keep_alive_interval() const {
    return crlf_keep_alive_interval_;
  }
  void set_crlf_keep_alive_interval(int seconds) {
    DCHECK_GT(seconds, 0);
    crlf_keep_alive_interval_ = seconds;
  }

 private:
  int reuse_lifetime_;
  bool enable_compact_headers_;
  bool crlf_keep_alive_;
  int crlf_keep_alive_interval_;
  std::string software_name_;
  BranchFactory *branch_factory_;
  TransactionFactory *transaction_factory_;
  SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory_;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_SETTINGS_H_
