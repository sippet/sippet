// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_HANDLER_FACTORY_H_
#define SIPPET_UA_AUTH_HANDLER_FACTORY_H_

#include <map>

#include "base/memory/scoped_ptr.h"
#include "net/dns/host_resolver.h"
#include "sippet/ua/auth.h"

namespace sippet {

// Based on net/http/http_auth_handler_factory.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Challenge;
class AuthHandler;
class AuthHandlerRegistryFactory;

// An AuthHandlerFactory is used to create AuthHandler objects.
// The AuthHandlerFactory object _must_ outlive any of the AuthHandler
// objects that it creates.
class AuthHandlerFactory {
 public:
  enum CreateReason {
    CREATE_CHALLENGE,     // Create a handler in response to a challenge.
    CREATE_PREEMPTIVE,    // Create a handler preemptively.
  };

  AuthHandlerFactory() {}
  virtual ~AuthHandlerFactory() {}

  // Creates an AuthHandler object based on the authentication
  // challenge specified by |challenge|.
  //
  // If an AuthHandler object is successfully created it is passed back to
  // the caller through |*handler| and OK is returned.
  //
  // If |challenge| specifies an unsupported authentication scheme, |*handler|
  // is set to NULL and |net::ERR_UNSUPPORTED_AUTH_SCHEME| is returned.
  //
  // |create_reason| indicates why the handler is being created. This is used
  // since NTLM and Negotiate schemes do not support preemptive creation.
  //
  // |digest_nonce_count| is specifically intended for the Digest authentication
  // scheme, and indicates the number of handlers generated for a particular
  // server nonce challenge.
  //
  // For the NTLM and Negotiate handlers:
  // If |origin| does not match the authentication method's filters for
  // the specified |target|, |net::ERR_INVALID_AUTH_CREDENTIALS| is returned.
  // NOTE: This will apply to ALL |origin| values if the filters are empty.
  virtual int CreateAuthHandler(const Challenge &challenge,
                                Auth::Target target,
                                const GURL& origin,
                                CreateReason create_reason,
                                int digest_nonce_count,
                                const net::BoundNetLog& net_log,
                                scoped_ptr<AuthHandler>* handler) = 0;

  // Creates a SIP authentication handler based on the authentication
  // |challenge|. See |CreateAuthHandler| for more details on return values.
  int CreateChallengeAuthHandler(const Challenge &challenge,
                                 Auth::Target target,
                                 const GURL& origin,
                                 const net::BoundNetLog& net_log,
                                 scoped_ptr<AuthHandler>* handler);

  // Creates a SIP authentication handler based on the authentication
  // |challenge|. See |CreateAuthHandler| for more details on return values.
  int CreatePreemptiveAuthHandler(const Challenge &challenge,
                                  Auth::Target target,
                                  const GURL& origin,
                                  int digest_nonce_count,
                                  const net::BoundNetLog& net_log,
                                  scoped_ptr<AuthHandler>* handler);

  // Creates a standard AuthHandlerRegistryFactory. The caller is responsible
  // for deleting the factory.
  // The default factory supports Digest, NTLM, and Negotiate schemes.
  //
  // |resolver| is used by the Negotiate authentication handler to perform
  // CNAME lookups to generate a Kerberos SPN for the server. It must be
  // non-NULL. |resolver| must remain valid for the lifetime of the
  // AuthHandlerRegistryFactory and any AuthHandlers created by said
  // factory.
  static AuthHandlerRegistryFactory* CreateDefault(
      net::HostResolver* resolver);

 private:
  DISALLOW_COPY_AND_ASSIGN(AuthHandlerFactory);
};

// The HttpAuthHandlerRegistryFactory dispatches create requests out
// to other factories based on the auth scheme.
class AuthHandlerRegistryFactory
    : public AuthHandlerFactory {
 public:
  AuthHandlerRegistryFactory();
  ~AuthHandlerRegistryFactory() override;

  // Registers a |factory| that will be used for a particular SIP
  // authentication scheme such as Digest, or Negotiate.
  // The |*factory| object is assumed to be new-allocated, and its lifetime
  // will be managed by this AuthHandlerRegistryFactory object (including
  // deleting it when it is no longer used.
  // A NULL |factory| value means that AuthHandlers's will not be created
  // for |scheme|. If a factory object used to exist for |scheme|, it will be
  // deleted.
  void RegisterSchemeFactory(const std::string& scheme,
                             AuthHandlerFactory* factory);

  // Retrieve the factory for the specified |scheme|. If no factory exists
  // for the |scheme|, NULL is returned. The returned factory must not be
  // deleted by the caller, and it is guaranteed to be valid until either
  // a new factory is registered for the same scheme, or until this
  // registry factory is destroyed.
  AuthHandlerFactory* GetSchemeFactory(const std::string& scheme) const;

  // Creates an AuthHandlerRegistryFactory.
  //
  // |supported_schemes| is a list of authentication schemes. Valid values
  // include "digest", "ntlm", and "negotiate", where case matters.
  //
  // |security_manager| is used by the NTLM and Negotiate authenticators
  // to determine which servers Integrated Authentication can be used with. If
  // NULL, Integrated Authentication will not be used with any server.
  //
  // |host_resolver| is used by the Negotiate authentication handler to perform
  // CNAME lookups to generate a Kerberos SPN for the server. If the "negotiate"
  // scheme is used and |negotiate_disable_cname_lookup| is false,
  // |host_resolver| must not be NULL.
  //
  // |gssapi_library_name| specifies the name of the GSSAPI library that will
  // be loaded on all platforms except Windows.
  //
  // |negotiate_disable_cname_lookup| and |negotiate_enable_port| both control
  // how Negotiate does SPN generation, by default these should be false.
  static AuthHandlerRegistryFactory* Create(
      const std::vector<std::string>& supported_schemes,
      net::HostResolver* host_resolver,
      const std::string& gssapi_library_name,
      bool negotiate_disable_cname_lookup,
      bool negotiate_enable_port);

  // Creates an auth handler by dispatching out to the registered factories
  // based on the first token in |challenge|.
  int CreateAuthHandler(const Challenge &challenge,
                        Auth::Target target,
                        const GURL& origin,
                        CreateReason reason,
                        int digest_nonce_count,
                        const net::BoundNetLog& net_log,
                        scoped_ptr<AuthHandler>* handler) override;

 private:
  typedef std::map<std::string, AuthHandlerFactory*> FactoryMap;

  FactoryMap factory_map_;
  DISALLOW_COPY_AND_ASSIGN(AuthHandlerRegistryFactory);
};

} // namespace sippet

#endif  // SIPPET_UA_AUTH_HANDLER_FACTORY_H_
