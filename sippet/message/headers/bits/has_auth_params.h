/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

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
