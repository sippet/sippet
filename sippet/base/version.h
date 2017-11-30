// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_VERSION_H_
#define SIPPET_BASE_VERSION_H_

#include <string>

namespace sippet {

// Returns the Sippet version, in the form "major.minor (branch@revision)".
std::string GetSippetVersion();

// The following 2 functions return the major and minor sippet versions.
int GetSippetMajorVersion();
int GetSippetMinorVersion();

std::string GetSippetRevision();

} // namespace sippet

#endif // SIPPET_BASE_VERSION_H_
