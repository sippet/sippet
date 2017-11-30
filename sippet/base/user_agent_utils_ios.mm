// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/base/user_agent_utils.h"
#include "sippet/base/version.h"

#import <UIKit/UIKit.h>

#include <sys/sysctl.h>
#include <string>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/sys_string_conversions.h"
#include "base/sys_info.h"

namespace sippet {

std::string BuildOSCpuInfo(const std::string& device) {
  int32_t os_major_version = 0;
  int32_t os_minor_version = 0;
  int32_t os_bugfix_version = 0;
  base::SysInfo::OperatingSystemVersionNumbers(&os_major_version,
                                               &os_minor_version,
                                               &os_bugfix_version);
  std::string os_version;
  if (os_bugfix_version == 0) {
    base::StringAppendF(&os_version,
                        "%d_%d",
                        os_major_version,
                        os_minor_version);
  } else {
    base::StringAppendF(&os_version,
                        "%d_%d_%d",
                        os_major_version,
                        os_minor_version,
                        os_bugfix_version);
  }

  // Remove the end of the platform name. For example "iPod touch" becomes
  // "iPod".
  std::string platform = base::SysNSStringToUTF8(
      [[UIDevice currentDevice] model]);
  size_t position = platform.find_first_of(" ");
  if (position != std::string::npos)
    platform = platform.substr(0, position);

  std::string os_cpu;
  if (!device.empty()) {
    base::StringAppendF(
        &os_cpu,
        "%s; %s; CPU %s %s like Mac OS X",
        platform.c_str(),
        device.c_str(),
        (platform == "iPad") ? "OS" : "iPhone OS",
        os_version.c_str());
  }
  else {
    base::StringAppendF(
        &os_cpu,
        "%s; CPU %s %s like Mac OS X",
        platform.c_str(),
        (platform == "iPad") ? "OS" : "iPhone OS",
        os_version.c_str());
  }

  return os_cpu;
}

std::string BuildUserAgentFromProduct(const std::string& product,
                                      const std::string& device) {
  // Retrieve the kernel build number.
  int mib[2] = {CTL_KERN, KERN_OSVERSION};
  unsigned int namelen = sizeof(mib) / sizeof(mib[0]);
  size_t bufferSize = 0;
  sysctl(mib, namelen, nullptr, &bufferSize, nullptr, 0);
  char kernel_version[bufferSize];
  int result = sysctl(mib, namelen, kernel_version, &bufferSize, nullptr, 0);
  DCHECK(result == 0);

  std::string user_agent;
  base::StringAppendF(
      &user_agent,
      "Sippet/%d.%d (%s)",
      GetSippetMajorVersion(),
      GetSippetMinorVersion(),
      BuildOSCpuInfo(device).c_str());

  if (!product.empty()) {
    base::StringAppendF(
        &user_agent,
        " %s",
        product.c_str());
  }

  base::StringAppendF(
      &user_agent,
      " Mobile/%s",
      kernel_version);

  return user_agent;
}

}  // namespace webkit_glue
