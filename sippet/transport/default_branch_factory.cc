// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/default_branch_factory.h"
#include "sippet/transport/network_layer.h"

#include "base/rand_util.h"
#include "base/base64.h"

namespace sippet {

DefaultBranchFactory::DefaultBranchFactory() {}

DefaultBranchFactory::~DefaultBranchFactory() {}

std::string DefaultBranchFactory::CreateBranch() {
  // Base64 will generate a shorter string than hex
  uint64 sixteen_bytes[2] = { base::RandUint64(), base::RandUint64() };
  // An input of 15 bytes will generate 20 characters on output
  base::StringPiece part(reinterpret_cast<char*>(sixteen_bytes),
    sizeof(sixteen_bytes)-1);
  std::string random_string;
  base::Base64Encode(part, &random_string);
  return NetworkLayer::kMagicCookie + random_string;
}

} // End of sippet namespace
