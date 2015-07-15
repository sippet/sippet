// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ICE_SERVER_H_
#define SIPPET_PHONE_ICE_SERVER_H_

#include <string>

namespace sippet {
namespace phone {

class IceServer {
 public:
  IceServer() {}
  ~IceServer() {}

  explicit IceServer(const std::string& uri) :
    uri_(uri) {
  }
  
  explicit IceServer(const std::string& uri,
                     const std::string& username,
                     const std::string& password) :
    uri_(uri),
    username_(username),
    password_(password) {
  }

  // URI example: stun:stun.l.google.com:19302
  const std::string& uri() const { return uri_; }
  void set_uri(const std::string& value) { uri_ = value; }

  // STUN/TURN username
  const std::string& username() const { return username_; }
  void set_username(const std::string& value) { username_ = value; }

  // STUN/TURN password
  const std::string& password() const { return password_; }
  void set_password(const std::string& value) { password_ = value; }

 private:
  std::string uri_; 
  std::string username_;
  std::string password_;
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_ICE_SERVER_H_
