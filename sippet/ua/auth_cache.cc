// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_cache.h"
#include "sippet/uri/uri.h"

namespace sippet {

// Based on net/http/http_auth_cache.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace {

// Debug helper to check that |origin| arguments are properly formed.
void CheckOriginIsValid(const GURL& origin) {
  DCHECK(origin.SchemeIs("sip") || origin.SchemeIs("sips"));
  SipURI origin_uri(origin);
  DCHECK(origin_uri.is_valid());
  DCHECK(origin_uri.GetOrigin() == origin_uri);
}

} // namespace

AuthCache::Entry::Entry()
  : scheme_(net::HttpAuth::AUTH_SCHEME_MAX),
    nonce_count_(0) {
}

AuthCache::Entry::~Entry() {
}

void AuthCache::Entry::UpdateStaleChallenge() {
  nonce_count_ = 1;
}

AuthCache::AuthCache() {
}

AuthCache::~AuthCache() {
}

AuthCache::Entry* AuthCache::Lookup(const std::string& realm,
                                    Auth::Scheme scheme) {
  // Linear scan through the realm entries.
  for (EntryList::iterator it = entries_.begin(); it != entries_.end(); ++it) {
    if (it->realm() == realm && it->scheme() == scheme)
      return &(*it);
  }
  return NULL;  // No realm entry found.
}

AuthCache::Entry* AuthCache::Add(const std::string& realm,
                                 Auth::Scheme scheme,
                                 const net::AuthCredentials& credentials) {
  // Check for existing entry (we will re-use it if present).
  AuthCache::Entry* entry = Lookup(realm, scheme);
  if (!entry) {
    // Failsafe to prevent unbounded memory growth of the cache.
    if (entries_.size() >= kMaxNumRealmEntries) {
      LOG(WARNING) << "Num auth cache entries reached limit -- evicting";
      entries_.pop_back();
    }

    entries_.push_front(Entry());
    entry = &entries_.front();
    entry->realm_ = realm;
    entry->scheme_ = scheme;
  }
  DCHECK_EQ(realm, entry->realm_);
  DCHECK_EQ(scheme, entry->scheme_);

  entry->credentials_ = credentials;
  entry->nonce_count_ = 1;

  return entry;
}

bool AuthCache::Remove(const std::string& realm,
                       Auth::Scheme scheme,
                       const net::AuthCredentials& credentials) {
  for (EntryList::iterator it = entries_.begin(); it != entries_.end(); ++it) {
    if (it->realm() == realm && it->scheme() == scheme) {
      if (credentials.Equals(it->credentials())) {
        entries_.erase(it);
        return true;
      }
      return false;
    }
  }
  return false;
}

bool AuthCache::UpdateStaleChallenge(const std::string& realm,
                                     Auth::Scheme scheme) {
  AuthCache::Entry* entry = Lookup(realm, scheme);
  if (!entry)
    return false;
  entry->UpdateStaleChallenge();
  return true;
}

void AuthCache::UpdateAllFrom(const AuthCache& other) {
  for (EntryList::const_iterator it = other.entries_.begin();
       it != other.entries_.end(); ++it) {
    Entry* entry = Add(it->realm(), it->scheme(), it->credentials());
    // Copy nonce count (for digest authentication).
    entry->nonce_count_ = it->nonce_count_;
  }
}

} // namespace sippet