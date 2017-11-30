// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/base/version.h"
#include "base/strings/stringprintf.h"

// Generated
#include "sippet/sippet_version.h"

namespace sippet {

std::string GetSippetVersion() {
  return base::StringPrintf("%d.%d (%s)",
                            SIPPET_VERSION_MAJOR,
                            SIPPET_VERSION_MINOR,
                            SIPPET_SVN_REVISION);
}

int GetSippetMajorVersion() {
  return SIPPET_VERSION_MAJOR;
}

int GetSippetMinorVersion() {
  return SIPPET_VERSION_MINOR;
}

std::string GetSippetRevision() {
  return SIPPET_SVN_REVISION;
}

}  // namespace sippet
