// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
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

#ifndef SIPPET_MESSAGE_HEADERS_BITS_AUTH_SETTERS_H_
#define SIPPET_MESSAGE_HEADERS_BITS_AUTH_SETTERS_H_

#include <string>
#include "base/strings/stringprintf.h"

namespace sippet {

#define string_dquote_param_template(cls, Name, Capitalized)        \
template<class T>                                                   \
class cls {                                                         \
public:                                                             \
  bool Has##Capitalized() const {                                   \
    return static_cast<const T*>(this)->param_find(#Name) !=        \
           static_cast<const T*>(this)->param_end();                \
  }                                                                 \
  std::string Name() const {                                        \
    assert(Has##Capitalized() && "Cannot read " #Name);             \
    return static_cast<const T*>(this)->param_find(#Name)->second;  \
  }                                                                 \
  void set_##Name(const std::string &Name) {                        \
    static_cast<T*>(this)->param_set(#Name, "\"" + Name + "\"");    \
  }                                                                 \
};

string_dquote_param_template(has_username,    username,    Username )
string_dquote_param_template(has_realm,       realm,       Realm    )
string_dquote_param_template(has_nonce,       nonce,       Nonce    )
string_dquote_param_template(has_uri,         uri,         Uri      )
string_dquote_param_template(has_response,    response,    Response )
string_dquote_param_template(has_cnonce,      cnonce,      Cnonce   )
string_dquote_param_template(has_opaque,      opaque,      Opaque   )
string_dquote_param_template(has_qop_options, qop,         Qop      )
string_dquote_param_template(has_rspauth,     rspauth,     RspAuth  )
string_dquote_param_template(has_nextnonce,   nextnonce,   NextNonce)
string_dquote_param_template(has_domain,      domain,      Domain   )

#undef string_dquote_param_template

template<class T>
class has_qop {
public:
  enum QopType {
    auth = 0, auth_int
  };

  bool HasQop() const {
    return static_cast<const T*>(this)->param_find("qop") !=
           static_cast<const T*>(this)->param_end();
  }
  std::string qop() const {
    assert(HasQop() && "Cannot read qop");
    return static_cast<const T*>(this)->param_find("qop")->second;
  }
  void set_qop(const std::string &qop) {
    static_cast<T*>(this)->param_set("qop", qop);
  }
  void set_qop(QopType t) {
    const char *rep[] = { "auth", "auth-int" };
    set_qop(rep[static_cast<int>(t)]);
  }
};

template<class T>
class has_algorithm {
public:
  enum Algorithm {
    MD5 = 0, MD5_sess
  };

  bool HasAlgorithm() const {
    return static_cast<T*>(this)->param_find("algorithm") !=
           static_cast<T*>(this)->param_end();
  }
  std::string algorithm() const {
    assert(HasAlgorithm() && "Cannot read algorithm");
    return static_cast<T*>(this)->param_find("algorithm")->second;
  }
  void set_algorithm(const std::string &algorithm) {
    static_cast<T*>(this)->param_set("algorithm", algorithm);
  }
  void set_algorithm(Algorithm a) {
    const char *rep[] = { "MD5", "MD5-sess" };
    static_cast<T*>(this)->param_set("algorithm", rep[static_cast<int>(a)]);
  }
};

template<class T>
class has_nc {
public:
  bool HasNc() const {
    return static_cast<T*>(this)->param_find("nc") !=
           static_cast<T*>(this)->param_end();
  }
  unsigned nc() const {
    assert(HasNc() && "Cannot read nc");
    int output;
    if (base::HexStringToInt(static_cast<T*>(this)->param_find("nc")->second, &output))
      return static_cast<unsigned>(output);
    return 0;
  }
  void set_nc(unsigned nc) {
    static_cast<T*>(this)->param_set("nc", base::StringPrintf("%08x", nc));
  }
};

template<class T>
class has_stale {
public:
  bool HasStale() const {
    return static_cast<T*>(this)->param_find("stale") !=
           static_cast<T*>(this)->param_end();
  }
  bool stale() const {
    assert(HasStale() && "Cannot read stale");
    return (static_cast<T*>(this)->param_find("stale")->second == "true");
  }
  void set_stale(bool stale) {
    static_cast<T*>(this)->param_set("stale", stale ? "true" : "false");
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_AUTH_SETTERS_H_
