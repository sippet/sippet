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
#include "sippet/base/raw_ostream.h"

namespace sippet {

class has_realm {
public:
  has_realm() {}
  has_realm(const has_realm &other) : realm_(other.realm_) {}
  ~has_realm() {}

  has_realm &operator=(const has_realm &other) {
    realm_ = other.realm_;
    return *this;
  }

  std::string realm() const { return realm_; }
  void set_realm(const std::string &realm) { realm_ = realm; }

  void print(raw_ostream &os) const {
    os << "realm=\"" << realm_ << "\"";
  }
private:
  std::string realm_;
};

class has_username {
public:
  has_username() {}
  has_username(const has_username &other) : username_(other.username_) {}
  ~has_username() {}

  has_username &operator=(const has_username &other) {
    username_ = other.username_;
    return *this;
  }

  std::string username() const { return username_; }
  void set_username(const std::string &username) { username_ = username; }

  void print(raw_ostream &os) const {
    os << "username=\"" << username_ << "\"";
  }
private:
  std::string username_;
};

class has_nonce {
public:
  has_nonce() {}
  has_nonce(const has_nonce &other) : nonce_(other.nonce_) {}
  ~has_nonce() {}

  has_nonce &operator=(const has_nonce &other) {
    nonce_ = other.nonce_;
    return *this;
  }

  std::string nonce() const { return nonce_; }
  void set_nonce(const std::string &nonce) { nonce_ = nonce; }

  void print(raw_ostream &os) const {
    os << "nonce=\"" << nonce_ << "\"";
  }
private:
  std::string nonce_;
};

class has_uri {
public:
  has_uri() {}
  has_uri(const has_uri &other) : uri_(other.uri_) {}
  ~has_uri() {}

  has_uri &operator=(const has_uri &other) {
    uri_ = other.uri_;
    return *this;
  }

  std::string uri() const { return uri_; }
  void set_uri(const std::string &uri) { uri_ = uri; }

  void print(raw_ostream &os) const {
    os << "uri=\"" << uri_ << "\"";
  }
private:
  std::string uri_;
};

class has_response {
public:
  has_response() {}
  has_response(const has_response &other) : response_(other.response_) {}
  ~has_response() {}

  has_response &operator=(const has_response &other) {
    response_ = other.response_;
    return *this;
  }

  std::string response() const { return response_; }
  void set_response(const std::string &response) { response_ = response; }

  void print(raw_ostream &os) const {
    os << "response=\"" << response_ << "\"";
  }
private:
  std::string response_;
};

class has_algorithm {
public:
  has_algorithm() {}
  has_algorithm(const has_algorithm &other) : algorithm_(other.algorithm_) {}
  ~has_algorithm() {}

  has_algorithm &operator=(const has_algorithm &other) {
    algorithm_ = other.algorithm_;
    return *this;
  }

  std::string algorithm() const { return algorithm_; }
  void set_algorithm(const std::string &algorithm) { algorithm_ = algorithm; }

  void print(raw_ostream &os) const {
    os << "algorithm=" << algorithm_;
  }
private:
  std::string algorithm_;
};

class has_cnonce {
public:
  has_cnonce() {}
  has_cnonce(const has_cnonce &other) : cnonce_(other.cnonce_) {}
  ~has_cnonce() {}

  has_cnonce &operator=(const has_cnonce &other) {
    cnonce_ = other.cnonce_;
    return *this;
  }

  std::string cnonce() const { return cnonce_; }
  void set_cnonce(const std::string &cnonce) { cnonce_ = cnonce; }

  void print(raw_ostream &os) const {
    os << "cnonce=\"" << cnonce_ << "\"";
  }
private:
  std::string cnonce_;
};

class has_opaque {
public:
  has_opaque() {}
  has_opaque(const has_opaque &other) : opaque_(other.opaque_) {}
  ~has_opaque() {}

  has_opaque &operator=(const has_opaque &other) {
    opaque_ = other.opaque_;
    return *this;
  }

  std::string opaque() const { return opaque_; }
  void set_opaque(const std::string &opaque) { opaque_ = opaque; }

  void print(raw_ostream &os) const {
    os << "opaque=\"" << opaque_ << "\"";
  }
private:
  std::string opaque_;
};

class has_qop {
public:
  enum QopType {
    auth = 0, auth_int
  };

  has_qop() {}
  has_qop(const has_qop &other) : qop_(other.qop_) {}
  ~has_qop() {}

  has_qop &operator=(const has_qop &other) {
    qop_ = other.qop_;
    return *this;
  }

  std::string qop() const { return qop_; }
  void set_qop(const std::string &qop) { qop_ = qop; }
  void set_qop(QopType t) {
    const char *rep[] = { "auth", "auth-int" };
    set_qop(rep[static_cast<int>(t)]);
  }

  void print(raw_ostream &os) const {
    os << "qop=" << qop_;
  }
private:
  std::string qop_;
};

class has_qop_options {
public:
  has_qop_options() {}
  has_qop_options(const has_qop_options &other) : qop_options_(other.qop_options_) {}
  ~has_qop_options() {}

  has_qop_options &operator=(const has_qop_options &other) {
    qop_options_ = other.qop_options_;
    return *this;
  }

  std::string qop_options() const { return qop_options_; }
  void set_qop_options(const std::string &qop_options) { qop_options_ = qop_options; }

  void print(raw_ostream &os) const {
    os << "qop=\"" << qop_options_ << "\"";
  }
private:
  std::string qop_options_;
};

class has_nc {
public:
  has_nc() : nc_(0) {}
  has_nc(const has_nc &other) : nc_(other.nc_) {}
  ~has_nc() {}

  has_nc &operator=(const has_nc &other) {
    nc_ = other.nc_;
    return *this;
  }

  unsigned nc() const { return nc_; }
  void set_nc(unsigned nc) { nc_ = nc; }

  void print(raw_ostream &os) const {
    os << "nc=" << format("%08x", nc_);
  }
private:
  unsigned nc_;
};

class has_rspauth {
public:
  has_rspauth() {}
  has_rspauth(const has_rspauth &other) : rspauth_(other.rspauth_) {}
  ~has_rspauth() {}

  has_rspauth &operator=(const has_rspauth &other) {
    rspauth_ = other.rspauth_;
    return *this;
  }

  std::string rspauth() const { return rspauth_; }
  void set_rspauth(const std::string &rspauth) { rspauth_ = rspauth; }

  void print(raw_ostream &os) const {
    os << "rspauth=\"" << rspauth_ << "\"";
  }
private:
  std::string rspauth_;
};

class has_nextnonce {
public:
  has_nextnonce() {}
  has_nextnonce(const has_nextnonce &other) : nextnonce_(other.nextnonce_) {}
  ~has_nextnonce() {}

  has_nextnonce &operator=(const has_nextnonce &other) {
    nextnonce_ = other.nextnonce_;
    return *this;
  }

  std::string nextnonce() const { return nextnonce_; }
  void set_nextnonce(const std::string &nextnonce) { nextnonce_ = nextnonce; }

  void print(raw_ostream &os) const {
    os << "nextnonce=\"" << nextnonce_ << "\"";
  }
private:
  std::string nextnonce_;
};

class has_stale {
public:
  has_stale() {}
  has_stale(const has_stale &other) : stale_(other.stale_) {}
  ~has_stale() {}

  has_stale &operator=(const has_stale &other) {
    stale_ = other.stale_;
    return *this;
  }

  bool stale() const { return stale_; }
  void set_stale(bool stale) { stale_ = stale; }

  void print(raw_ostream &os) const {
    os << "stale=" << (stale_ ? "true" : "false");
  }
private:
  bool stale_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_AUTH_SETTERS_H_
