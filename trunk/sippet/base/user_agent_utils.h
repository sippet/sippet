// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_USER_AGENT_UTILS_H_
#define SIPPET_BASE_USER_AGENT_UTILS_H_

#include <string>
#include "base/basictypes.h"

namespace sippet {

// Builds a User-agent compatible string that describes the OS and CPU type.
std::string BuildOSCpuInfo();

// Helper function to generate a full user agent string from a short
// product name.
std::string BuildUserAgentFromProduct(
    const std::string& product);

} // namespace sippet

#endif // SIPPET_BASE_USER_AGENT_UTILS_H_
