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
  static std::string GetDefaultSoftwareName();

 private:
  struct Data {
    int reuse_lifetime_;
    bool enable_compact_headers_;
    std::string software_name_;
    BranchFactory *branch_factory_;
    TransactionFactory *transaction_factory_;
    SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory_;
    // Default values
    Data() :
      reuse_lifetime_(60),
      enable_compact_headers_(true),
      software_name_(GetDefaultSoftwareName()),
      branch_factory_(BranchFactory::GetDefaultBranchFactory()),
      transaction_factory_(TransactionFactory::GetDefaultTransactionFactory()),
      ssl_cert_error_handler_factory_(nullptr) {}
  };

  Data data_;
 public:
  NetworkSettings() {}
  NetworkSettings(const NetworkSettings &other)
    : data_(other.data_) {}

  NetworkSettings &operator=(const NetworkSettings &other) {
    data_ = other.data_;
    return *this;
  }

  // Time to keep idle channels open
  int reuse_lifetime() const {
    return data_.reuse_lifetime_;
  }
  void set_reuse_lifetime(int value) {
    data_.reuse_lifetime_ = value;
  }

  // Whether to use compact headers in generated messages
  bool enable_compact_headers() const {
    return data_.enable_compact_headers_;
  }
  void set_enable_compact_headers(bool value) {
    data_.enable_compact_headers_ = value;
  }

  // Set the software name (the value added to User-Agent headers)
  std::string software_name() const {
    return data_.software_name_;
  }
  void set_software_name(const std::string &software_name) {
    data_.software_name_ = software_name;
  }

  // The internal branch factory to use
  BranchFactory *branch_factory() const {
    return data_.branch_factory_;
  }
  void set_branch_factory(BranchFactory *branch_factory) {
    DCHECK(branch_factory);
    data_.branch_factory_ = branch_factory;
  }

  // The internal transaction factory to use
  TransactionFactory *transaction_factory() const {
    return data_.transaction_factory_;
  }
  void set_transaction_factory(TransactionFactory *transaction_factory) {
    DCHECK(transaction_factory);
    data_.transaction_factory_ = transaction_factory;
  }

  // The SSL certificate error handler factory to use
  SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory() const {
    return data_.ssl_cert_error_handler_factory_;
  }
  void set_ssl_cert_error_handler_factory(
      SSLCertErrorHandler::Factory *ssl_cert_error_handler_factory) {
    DCHECK(ssl_cert_error_handler_factory);
    data_.ssl_cert_error_handler_factory_ = ssl_cert_error_handler_factory;
  }
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_SETTINGS_H_
