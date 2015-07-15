// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_SETTINGS_H_
#define SIPPET_PHONE_SETTINGS_H_

#include <string>
#include <vector>

#include "url/gurl.h"

#include "sippet/phone/ice_server.h"

namespace sippet {
namespace phone {

// Phone settings
class Settings {
 public:
  typedef std::vector<IceServer> IceServers;
  typedef std::vector<GURL> RouteSet;

  Settings();
  ~Settings();

  bool is_valid() const;

  // Enable/disable streaming encryption
  void set_disable_encryption(bool value) { disable_encryption_ = value; }
  bool disable_encryption() const { return disable_encryption_; }

  // Enable/disable SCTP data channels
  void set_disable_sctp_data_channels(bool value) {
    disable_sctp_data_channels_ = value;
  }
  bool disable_sctp_data_channels() const {
    return disable_sctp_data_channels_;
  }

  // ICE servers list.
  IceServers &ice_servers() {
    return ice_servers_;
  }
  const IceServers &ice_servers() const {
    return ice_servers_;
  }

  // List of SIP URIs and its goal is to contain all the proxies that route all
  // requests outside dialogs.
  RouteSet &route_set() {
    return route_set_;
  }
  const RouteSet &route_set() const {
    return route_set_;
  }

  // SIP URI associated to the User Agent. This is a SIP address given to you
  // by your provider.
  const GURL& uri() const {
    return uri_;
  }
  void set_uri(const GURL& value) {
    uri_ = value;
  }

  // If this is set then the User-Agent header will have this string appended
  // after sippet name and version.
  const std::string& user_agent() const {
    return user_agent_;
  }
  void set_user_agent(const std::string& value) {
    user_agent_ = value;
  }

  // Username to use when generating authentication credentials. If not defined
  // the value in uri parameter is used.
  const std::string& authorization_user() const {
    return authorization_user_;
  }
  void set_authorization_user(const std::string& value) {
    authorization_user_ = value;
  }

  // Descriptive name (String) to be shown to the called party when calling or
  // sending IM messages. It must NOT be enclosed between double quotes.
  const std::string &display_name() const {
    return display_name_;
  }
  void set_display_name(const std::string &value) {
    display_name_ = value;
  }

  // SIP Authentication password.
  const std::string &password() const {
    return password_;
  }
  void set_password(const std::string &value) {
    password_ = value;
  }

  // Registration expiry time (in seconds). Default value is 600.
  unsigned register_expires() const {
    return register_expires_;
  }
  void set_register_expires(unsigned value) {
    register_expires_ = value;
  }

  // Set the SIP registrar URI. Valid value is a SIP URI without username.
  // If empty (default) then the registrar URI is taken from the uri parameter
  // (by removing the username).
  const GURL& registrar_server() const {
    return registrar_server_;
  }
  void set_registrar_server(const GURL& value) {
    registrar_server_ = value;
  }
 
 private:
  IceServers ice_servers_;
  bool disable_encryption_;
  bool disable_sctp_data_channels_;
  RouteSet route_set_;
  GURL uri_;
  std::string user_agent_;
  std::string authorization_user_;
  std::string display_name_;
  std::string password_;
  unsigned register_expires_;
  GURL registrar_server_;
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_SETTINGS_H_
