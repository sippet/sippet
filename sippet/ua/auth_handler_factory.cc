// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_factory.h"

#include <string>
#include <vector>

#include "base/memory/ptr_util.h"
#include "base/strings/string_util.h"
#include "net/base/net_errors.h"
#include "sippet/message/headers.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_handler_digest.h"

namespace sippet {

// Based on net/http/http_auth_handler_factory.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

int AuthHandlerFactory::CreateChallengeAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    const net::NetLogWithSource& net_log,
    std::unique_ptr<AuthHandler>* handler) {
  return CreateAuthHandler(challenge, target, origin, CREATE_CHALLENGE, 1,
                           net_log, handler);
}

int AuthHandlerFactory::CreatePreemptiveAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    int digest_nonce_count,
    const net::NetLogWithSource& net_log,
    std::unique_ptr<AuthHandler>* handler) {
  return CreateAuthHandler(challenge, target, origin, CREATE_PREEMPTIVE,
                           digest_nonce_count, net_log, handler);
}

// static
AuthHandlerRegistryFactory* AuthHandlerFactory::CreateDefault(
    net::HostResolver* host_resolver) {
  DCHECK(host_resolver);
  AuthHandlerRegistryFactory* registry_factory =
      new AuthHandlerRegistryFactory();
  registry_factory->RegisterSchemeFactory(
      "digest", base::WrapUnique(new AuthHandlerDigest::Factory()));
  return registry_factory;
}

namespace {

bool IsSupportedScheme(const std::vector<std::string>& supported_schemes,
                       const std::string& scheme) {
  std::vector<std::string>::const_iterator it = std::find(
      supported_schemes.begin(), supported_schemes.end(), scheme);
  return it != supported_schemes.end();
}

}  // namespace

AuthHandlerRegistryFactory::AuthHandlerRegistryFactory() {}

AuthHandlerRegistryFactory::~AuthHandlerRegistryFactory() {}

void AuthHandlerRegistryFactory::RegisterSchemeFactory(
    const std::string& scheme,
    std::unique_ptr<AuthHandlerFactory> factory) {
  std::string lower_scheme = base::ToLowerASCII(scheme);
  if (factory)
    factory_map_.insert(std::make_pair(lower_scheme, std::move(factory)));
  else
    factory_map_.erase(lower_scheme);
}

AuthHandlerFactory* AuthHandlerRegistryFactory::GetSchemeFactory(
    const std::string& scheme) const {
  std::string lower_scheme = base::ToLowerASCII(scheme);
  FactoryMap::const_iterator it = factory_map_.find(lower_scheme);
  if (it == factory_map_.end()) {
    return nullptr;  // |scheme| is not registered.
  }
  return it->second.get();
}

// static
AuthHandlerRegistryFactory* AuthHandlerRegistryFactory::Create(
    const std::vector<std::string>& supported_schemes,
    net::HostResolver* host_resolver,
    const std::string& gssapi_library_name,
    bool negotiate_disable_cname_lookup,
    bool negotiate_enable_port) {
  AuthHandlerRegistryFactory* registry_factory =
      new AuthHandlerRegistryFactory();
  if (IsSupportedScheme(supported_schemes, "digest"))
    registry_factory->RegisterSchemeFactory(
        "digest", base::WrapUnique(new AuthHandlerDigest::Factory()));
  return registry_factory;
}

int AuthHandlerRegistryFactory::CreateAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    CreateReason reason,
    int digest_nonce_count,
    const net::NetLogWithSource& net_log,
    std::unique_ptr<AuthHandler>* handler) {
  std::string scheme = challenge.scheme();
  if (scheme.empty()) {
    handler->reset();
    return net::ERR_INVALID_RESPONSE;
  }
  std::string lower_scheme = base::ToLowerASCII(scheme);
  FactoryMap::iterator it = factory_map_.find(lower_scheme);
  if (it == factory_map_.end()) {
    handler->reset();
    return net::ERR_UNSUPPORTED_AUTH_SCHEME;
  }
  DCHECK(it->second);
  return it->second->CreateAuthHandler(challenge, target, origin, reason,
                                       digest_nonce_count, net_log, handler);
}

}  // namespace sippet
