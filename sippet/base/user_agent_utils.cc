// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/base/user_agent_utils.h"
#include "sippet/base/version.h"

#if defined(OS_POSIX) && !defined(OS_MACOSX)
#include <sys/utsname.h>
#endif

#include "base/lazy_instance.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/sys_info.h"

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

namespace sippet {

std::string BuildOSCpuInfo() {
  std::string os_cpu;

  const char kUserAgentPlatform[] =
#if defined(OS_WIN)
      "";
#elif defined(OS_MACOSX)
      "Macintosh; ";
#elif defined(USE_X11)
      "X11; ";           // strange, but that's what Firefox uses
#elif defined(OS_ANDROID)
      "Linux; ";
#else
      "Unknown; ";
#endif

#if defined(OS_WIN) || defined(OS_MACOSX) || defined(OS_CHROMEOS) ||\
    defined(OS_ANDROID)
  int32 os_major_version = 0;
  int32 os_minor_version = 0;
  int32 os_bugfix_version = 0;
  base::SysInfo::OperatingSystemVersionNumbers(&os_major_version,
                                               &os_minor_version,
                                               &os_bugfix_version);
#endif

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
  // Should work on any Posix system.
  struct utsname unixinfo;
  uname(&unixinfo);

  std::string cputype;
  // special case for biarch systems
  if (strcmp(unixinfo.machine, "x86_64") == 0 &&
      sizeof(void*) == sizeof(int32)) {  // NOLINT
    cputype.assign("i686 (x86_64)");
  } else {
    cputype.assign(unixinfo.machine);
  }
#endif

#if defined(OS_WIN)
  std::string architecture_token;
  base::win::OSInfo* os_info = base::win::OSInfo::GetInstance();
  if (os_info->wow64_status() == base::win::OSInfo::WOW64_ENABLED) {
    architecture_token = "; WOW64";
  } else {
    base::win::OSInfo::WindowsArchitecture windows_architecture =
        os_info->architecture();
    if (windows_architecture == base::win::OSInfo::X64_ARCHITECTURE)
      architecture_token = "; Win64; x64";
    else if (windows_architecture == base::win::OSInfo::IA64_ARCHITECTURE)
      architecture_token = "; Win64; IA64";
  }
#endif

#if defined(OS_ANDROID)
  std::string android_version_str;
  base::StringAppendF(
      &android_version_str, "%d.%d", os_major_version, os_minor_version);
  if (os_bugfix_version != 0)
    base::StringAppendF(&android_version_str, ".%d", os_bugfix_version);

  std::string android_info_str;

  // Send information about the device.
  bool semicolon_inserted = false;
  std::string android_build_codename = base::SysInfo::GetAndroidBuildCodename();
  std::string android_device_name = base::SysInfo::HardwareModelName();
  if ("REL" == android_build_codename && android_device_name.size() > 0) {
    android_info_str += "; " + android_device_name;
    semicolon_inserted = true;
  }

  // Append the build ID.
  std::string android_build_id = base::SysInfo::GetAndroidBuildID();
  if (android_build_id.size() > 0) {
    if (!semicolon_inserted) {
      android_info_str += ";";
    }
    android_info_str += " Build/" + android_build_id;
  }
#endif

  base::StringAppendF(
      &os_cpu,
#if defined(OS_WIN)
      "Windows NT %d.%d%s",
      os_major_version,
      os_minor_version,
      architecture_token.c_str()
#elif defined(OS_MACOSX)
      "Intel Mac OS X %d_%d_%d",
      os_major_version,
      os_minor_version,
      os_bugfix_version
#elif defined(OS_CHROMEOS)
      "CrOS "
      "%s %d.%d.%d",
      cputype.c_str(),   // e.g. i686
      os_major_version,
      os_minor_version,
      os_bugfix_version
#elif defined(OS_ANDROID)
      "Android %s%s",
      android_version_str.c_str(),
      android_info_str.c_str()
#else
      "%s %s",
      unixinfo.sysname,  // e.g. Linux
      cputype.c_str()    // e.g. i686
#endif
  );  // NOLINT

  std::string result;
  base::StringAppendF(&result, "%s%s", kUserAgentPlatform, os_cpu.c_str());
  return result;
}

std::string BuildUserAgentFromProduct(
    const std::string& product) {
  std::string user_agent;
  base::StringAppendF(
      &user_agent,
      "Sippet/%d.%d (%s)",
      GetSippetMajorVersion(),
      GetSippetMinorVersion(),
      BuildOSCpuInfo().c_str());

  if (!product.empty()) {
    base::StringAppendF(
        &user_agent,
        " %s",
        product.c_str());
  }

  return user_agent;
}

}  // namespace sippet
