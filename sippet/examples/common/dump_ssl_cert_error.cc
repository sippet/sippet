// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/examples/common/dump_ssl_cert_error.h"

#include <iostream>
#include <string>
#include <vector>

#include "base/i18n/time_formatting.h"
#include "net/base/net_errors.h"
#include "net/ssl/ssl_info.h"
#include "net/cert/x509_certificate.h"
#include "sippet/transport/end_point.h"

DumpSSLCertError::Factory::Factory(bool ignore_non_fatal)
  : ignore_non_fatal_(ignore_non_fatal) {
}

DumpSSLCertError::Factory::~Factory() {
}

std::unique_ptr<sippet::SSLCertErrorHandler>
    DumpSSLCertError::Factory::CreateSSLCertificateErrorHandler() {
  std::unique_ptr<sippet::SSLCertErrorHandler> ssl_cert_error_handler(
    new DumpSSLCertError(ignore_non_fatal_));
  return ssl_cert_error_handler.Pass();
}

DumpSSLCertError::DumpSSLCertError(bool ignore_non_fatal)
  : ignore_non_fatal_(ignore_non_fatal) {
}

DumpSSLCertError::~DumpSSLCertError() {
}

int DumpSSLCertError::GetUserApproval(
    const sippet::EndPoint &destination,
    const net::SSLInfo &ssl_info,
    bool fatal,
    bool *is_accepted,
    const net::CompletionCallback& callback) {
  std::cout << "SSL certificate error for channel "
            << destination.ToString()
            << "\n";

  std::cout << "-- gravity: "
            << (fatal ? "fatal" : "non-fatal")
            << "\n";

  std::cout << "-- issued by "
            << (ssl_info.is_issued_by_known_root ? "known" : "unknown")
            << " root\n";

  if (ssl_info.cert) {
    std::cout << "-- issuer: "
              << ssl_info.cert->issuer().GetDisplayName()
              << "\n";

    std::cout << "-- subject: "
              << ssl_info.cert->subject().GetDisplayName()
              << "\n";

    size_t size;
    net::X509Certificate::PublicKeyType public_key_type;
    ssl_info.cert->GetPublicKeyInfo(ssl_info.cert->os_cert_handle(),
        &size, &public_key_type);
    const char *key_type;
    if (public_key_type == net::X509Certificate::kPublicKeyTypeDH) {
      key_type = "DH";
    } else if (public_key_type == net::X509Certificate::kPublicKeyTypeDSA) {
      key_type = "DSA";
    } else if (public_key_type == net::X509Certificate::kPublicKeyTypeECDH) {
      key_type = "ECDH";
    } else if (public_key_type == net::X509Certificate::kPublicKeyTypeECDSA) {
      key_type = "ECDSA";
    } else if (public_key_type == net::X509Certificate::kPublicKeyTypeRSA) {
      key_type = "RSA";
    } else {
      key_type = "Unknown";
    }
    std::cout << "-- public key: "
              << key_type
              << " (" << size << " bits)"
              << "\n";

    std::vector<std::string> dns_names;
    ssl_info.cert->GetDNSNames(&dns_names);
    std::cout << "-- DNS names: ";
    for (std::vector<std::string>::iterator i = dns_names.begin(),
         ie = dns_names.end(); i != ie; i++) {
      if (i != dns_names.begin())
        std::cout << ", ";
      std::cout << *i;
    }
    std::cout << "\n";

    if (!ssl_info.cert->valid_start().is_null()
        && !ssl_info.cert->valid_expiry().is_null()) {
      base::string16 start =
          base::TimeFormatShortDateAndTime(ssl_info.cert->valid_start());
      base::string16 end =
          base::TimeFormatShortDateAndTime(ssl_info.cert->valid_expiry());
      std::cout << "-- Valid from " << start << " to " << end << "\n";
    }
  }

  *is_accepted = (fatal) ? false : ignore_non_fatal_;
  return net::OK;
}

int DumpSSLCertError::GetClientCert(
    const sippet::EndPoint &destination,
    const net::SSLInfo &ssl_info,
    scoped_refptr<net::X509Certificate> *client_cert,
    const net::CompletionCallback& callback) {
  // This simple implementation doesn't return any client certificate for
  // the TLS connection.
  return net::ERR_FAILED;
}

