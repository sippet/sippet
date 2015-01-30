// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_settings.h"
#include "sippet/base/user_agent_utils.h"

namespace sippet {

std::string NetworkSettings::GetDefaultSoftwareName() {
  return BuildUserAgentFromProduct("");
}

} // End of sippet namespace
