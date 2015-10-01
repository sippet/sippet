// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/settings.h"

#include "base/strings/string_util.h"

namespace sippet {
namespace phone {

Settings::Settings() :
  disable_encryption_(false),
  disable_sctp_data_channels_(false),
  register_expires_(600) {
}

Settings::~Settings() {
}

bool Settings::is_valid() const {
  if (uri_.is_empty())
    return false;

  if (!uri_.SchemeIs("sip") && !uri_.SchemeIs("sips"))
    return false;

  if (!registrar_server_.is_empty()) {
    if (!registrar_server_.SchemeIs("sip")
        && !registrar_server_.SchemeIs("sips"))
      return false;
  }

  if (route_set_.size() > 0) {
    for (RouteSet::const_iterator i = route_set_.begin(),
         ie = route_set_.end(); i != ie; i++) {
      if (!i->SchemeIs("sip") && i->SchemeIs("sips"))
        return false;
    }
  }

  if (ice_servers_.size() > 0) {
    for (IceServers::const_iterator i = ice_servers_.begin(),
         ie = ice_servers_.end(); i != ie; i++) {
      if (!base::StartsWith(i->uri(), "stun", base::CompareCase::SENSITIVE)
          && !base::StartsWith(i->uri(), "turn", base::CompareCase::SENSITIVE))
        return false;
    }
  }

  return true;
}

} // namespace phone
} // namespace sippet
