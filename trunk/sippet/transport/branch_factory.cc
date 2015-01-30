// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/branch_factory.h"
#include "sippet/transport/network_layer.h"
#include "sippet/base/tags.h"

#include "base/rand_util.h"
#include "base/base64.h"
#include "base/lazy_instance.h"
#include "base/strings/string_util.h"

namespace sippet {

namespace {

class DefaultBranchFactory : public BranchFactory {
 public:
  DefaultBranchFactory() {}
  virtual ~DefaultBranchFactory() {}
  virtual std::string CreateBranch() override {
    return sippet::CreateBranch();
  }
};

static base::LazyInstance<DefaultBranchFactory>::Leaky
  g_default_branch_factory = LAZY_INSTANCE_INITIALIZER;

} // End of empty namespace

BranchFactory *BranchFactory::GetDefaultBranchFactory() {
  return g_default_branch_factory.Pointer();
}

} // End of sippet namespace
