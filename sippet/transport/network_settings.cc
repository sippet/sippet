// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_settings.h"
#include "sippet/base/user_agent_utils.h"

namespace sippet {

NetworkSettings::NetworkSettings()
  : reuse_lifetime_(60),
    enable_compact_headers_(true),
    crlf_keep_alive_(false),
    crlf_keep_alive_interval_(60),
    software_name_(BuildUserAgentFromProduct("")),
    branch_factory_(BranchFactory::GetDefaultBranchFactory()),
    transaction_factory_(TransactionFactory::GetDefaultTransactionFactory()),
    ssl_cert_error_handler_factory_(nullptr) {
}

NetworkSettings::NetworkSettings(const NetworkSettings& other)
  : reuse_lifetime_(other.reuse_lifetime_),
    enable_compact_headers_(other.enable_compact_headers_),
    crlf_keep_alive_(other.crlf_keep_alive_),
    crlf_keep_alive_interval_(other.crlf_keep_alive_interval_),
    software_name_(other.software_name_),
    branch_factory_(other.branch_factory_),
    transaction_factory_(other.transaction_factory_),
    ssl_cert_error_handler_factory_(other.ssl_cert_error_handler_factory_) {
}

NetworkSettings::~NetworkSettings() {
}

}  // namespace sippet
