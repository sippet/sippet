// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_USER_AGENT_UTILS_H_
#define SIPPET_BASE_USER_AGENT_UTILS_H_

#include <string>

namespace sippet {

// Builds a User-agent compatible string that describes the OS and CPU type.
std::string BuildOSCpuInfo(const std::string& device = "");

// Helper function to generate a full user agent string from a short
// product name.
std::string BuildUserAgentFromProduct(
    const std::string& product,
    const std::string& device = "");

} // namespace sippet

#endif // SIPPET_BASE_USER_AGENT_UTILS_H_
