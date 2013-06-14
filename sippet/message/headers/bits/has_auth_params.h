// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_BITS_HAS_AUTH_PARAMS_H_
#define SIPPET_MESSAGE_HEADERS_BITS_HAS_AUTH_PARAMS_H_

#include "sippet/message/headers/bits/has_parameters.h"

namespace sippet {

class has_auth_params :
  public has_parameters {
public:
  enum Scheme {
    Digest = 0
  };

  has_auth_params() : has_scheme_(false) {}
  has_auth_params(Scheme s) { set_scheme(s); }
  has_auth_params(const std::string &scheme)
    : scheme_(scheme), has_scheme_(true) {}
  has_auth_params(const has_auth_params &other)
    : scheme_(other.scheme_), has_scheme_(other.has_scheme_) {}
  ~has_auth_params() {}

  has_auth_params &operator=(const has_auth_params &other) {
    scheme_ = other.scheme_;
    has_scheme_ = other.has_scheme_;
    has_parameters::operator=(other);
    return *this;
  }

  bool HasScheme() const { return has_scheme_; }
  std::string scheme() const {
    assert(HasScheme() && "Cannot read scheme");
    return scheme_;
  }
  void set_scheme(const std::string &scheme) {
    assert(!scheme.empty() && "Invalid scheme");
    scheme_ = scheme;
    has_scheme_ = true;
  }
  void set_scheme(Scheme s) {
    const char *rep[] = { "Digest" };
    set_scheme(rep[static_cast<int>(s)]);
  }

  void print(raw_ostream &os) const {
    if (has_scheme_)
      os << scheme_ << " ";
    for (const_param_iterator i = param_begin(), ie = param_end(); i != ie; ++i) {
      if (i != param_begin())
        os << ", ";
      os << i->first << "=" << i->second;
    }
  }
private:
  bool has_scheme_;
  std::string scheme_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_HAS_AUTH_PARAMS_H_
