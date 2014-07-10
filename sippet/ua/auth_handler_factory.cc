// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_factory.h"

#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "net/base/net_errors.h"
#include "sippet/message/headers.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_handler_digest.h"

namespace sippet {

int AuthHandlerFactory::CreateChallengeAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    const net::BoundNetLog& net_log,
    scoped_ptr<AuthHandler>* handler) {
  return CreateAuthHandler(challenge, target, origin, CREATE_CHALLENGE, 1,
                           net_log, handler);
}

int AuthHandlerFactory::CreatePreemptiveAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    int digest_nonce_count,
    const net::BoundNetLog& net_log,
    scoped_ptr<AuthHandler>* handler) {
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
      "digest", new AuthHandlerDigest::Factory());
  return registry_factory;
}

namespace {

bool IsSupportedScheme(const std::vector<std::string>& supported_schemes,
                       const std::string& scheme) {
  std::vector<std::string>::const_iterator it = std::find(
      supported_schemes.begin(), supported_schemes.end(), scheme);
  return it != supported_schemes.end();
}

} // namespace

AuthHandlerRegistryFactory::AuthHandlerRegistryFactory() {
}

AuthHandlerRegistryFactory::~AuthHandlerRegistryFactory() {
  STLDeleteContainerPairSecondPointers(factory_map_.begin(),
                                       factory_map_.end());
}

void AuthHandlerRegistryFactory::RegisterSchemeFactory(
    const std::string& scheme,
    AuthHandlerFactory* factory) {
  std::string lower_scheme = StringToLowerASCII(scheme);
  FactoryMap::iterator it = factory_map_.find(lower_scheme);
  if (it != factory_map_.end()) {
    delete it->second;
  }
  if (factory)
    factory_map_[lower_scheme] = factory;
  else
    factory_map_.erase(it);
}

AuthHandlerFactory* AuthHandlerRegistryFactory::GetSchemeFactory(
    const std::string& scheme) const {
  std::string lower_scheme = StringToLowerASCII(scheme);
  FactoryMap::const_iterator it = factory_map_.find(lower_scheme);
  if (it == factory_map_.end()) {
    return NULL; // |scheme| is not registered.
  }
  return it->second;
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
        "digest", new AuthHandlerDigest::Factory());
  return registry_factory;
}

int AuthHandlerRegistryFactory::CreateAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    CreateReason reason,
    int digest_nonce_count,
    const net::BoundNetLog& net_log,
    scoped_ptr<AuthHandler>* handler) {
  std::string scheme = challenge.scheme();
  if (scheme.empty()) {
    handler->reset();
    return net::ERR_INVALID_RESPONSE;
  }
  std::string lower_scheme = StringToLowerASCII(scheme);
  FactoryMap::iterator it = factory_map_.find(lower_scheme);
  if (it == factory_map_.end()) {
    handler->reset();
    return net::ERR_UNSUPPORTED_AUTH_SCHEME;
  }
  DCHECK(it->second);
  return it->second->CreateAuthHandler(challenge, target, origin, reason,
                                       digest_nonce_count, net_log, handler);
}

} // namespace sippet