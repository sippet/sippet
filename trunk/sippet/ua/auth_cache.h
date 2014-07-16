// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_AUTH_CACHE_H_
#define SIPPET_UA_AUTH_CACHE_H_

#include "net/base/auth.h"
#include "sippet/ua/auth.h"
#include "url/gurl.h"

namespace sippet {

// Based on net/http/http_auth_cache.h,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// AuthCache stores SIP authentication identities and challenge info.
// For each (origin, realm, scheme) triple the cache stores a
// AuthCache::Entry, which holds:
//   - the origin server {protocol scheme, host, port}
//   - the last identity used (username/password)
//   - the last auth handler used (contains realm and authentication scheme)
// Entries can be looked up by (origin, realm, scheme).
class AuthCache {
 public:
  class Entry {
   public:
    ~Entry();

    // The origin represents the {protocol, host, port}.
    const GURL& origin() const {
      return origin_;
    }

    // The case-sensitive realm string of the challenge.
    const std::string realm() const {
      return realm_;
    }

    // The authentication scheme of the challenge.
    Auth::Scheme scheme() const {
      return scheme_;
    }

    // The login credentials.
    const net::AuthCredentials& credentials() const {
      return credentials_;
    }

    // Increment the nonce count.
    int IncrementNonceCount() {
      return ++nonce_count_;
    }

    void UpdateStaleChallenge();

   private:
    friend class AuthCache;

    Entry();

    // |origin_| contains the {protocol, host, port} of the server.
    GURL origin_;
    std::string realm_;
    Auth::Scheme scheme_;

    // Identity.
    net::AuthCredentials credentials_;

    // Nonce count.
    int nonce_count_;
  };

  // Prevent unbounded memory growth. These are safeguards for abuse; it is
  // not expected that the limits will be reached in ordinary usage.
  // This also defines the worst-case lookup times (which grow linearly
  // with number of elements in the cache).
  enum { kMaxNumRealmEntries = 10 };

  AuthCache();
  ~AuthCache();

  // Find the realm entry on server |origin| for realm |realm| and
  // scheme |scheme|.
  //   |origin| - the {scheme, host, port} of the server.
  //   |realm|  - case sensitive realm string.
  //   |scheme| - the authentication scheme (i.e. basic, negotiate).
  //   returns  - the matched entry or NULL.
  Entry* Lookup(const GURL& origin,
                const std::string& realm,
                Auth::Scheme scheme);

  // Add an entry on server |origin| for realm |handler->realm()| and
  // scheme |handler->scheme()|.  If an entry for this (realm,scheme)
  // already exists, update it rather than replace it.
  //   |origin|   - the {scheme, host, port} of the server.
  //   |realm|    - the auth realm for the challenge.
  //   |scheme|   - the authentication scheme (i.e. basic, negotiate).
  //   |credentials| - login information for the realm.
  //   returns    - the entry that was just added/updated.
  Entry* Add(const GURL& origin,
             const std::string& realm,
             Auth::Scheme scheme,
             const net::AuthCredentials& credentials);

  // Remove entry on server |origin| for realm |realm| and scheme |scheme|
  // if one exists AND if the cached credentials matches |credentials|.
  //   |origin|   - the {scheme, host, port} of the server.
  //   |realm|    - case sensitive realm string.
  //   |scheme|   - the authentication scheme (i.e. basic, negotiate).
  //   |credentials| - the credentials to match.
  //   returns    - true if an entry was removed.
  bool Remove(const GURL& origin,
              const std::string& realm,
              Auth::Scheme scheme,
              const net::AuthCredentials& credentials);

  // Updates a stale digest entry on server |origin| for realm |realm| and
  // scheme |scheme|. The cached auth challenge is replaced with
  // |auth_challenge| and the nonce count is reset.
  // |UpdateStaleChallenge()| returns true if a matching entry exists in the
  // cache, false otherwise.
  bool UpdateStaleChallenge(const GURL& origin,
                            const std::string& realm,
                            Auth::Scheme scheme);

  // Copies all entries from |other| cache.
  void UpdateAllFrom(const AuthCache& other);

 private:
  typedef std::list<Entry> EntryList;
  EntryList entries_;
};

} // namespace sippet

#endif // SIPPET_UA_AUTH_CACHE_H_