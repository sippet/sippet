// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_DEFAULT_BRANCH_FACTORY_H_
#define SIPPET_TRANSPORT_DEFAULT_BRANCH_FACTORY_H_

#include "sippet/transport/branch_factory.h"
#include "base/compiler_specific.h"

namespace sippet {

class DefaultBranchFactory : public BranchFactory {
 public:
  DefaultBranchFactory();
  virtual ~DefaultBranchFactory();
  virtual std::string CreateBranch() OVERRIDE;
 private:
  DISALLOW_COPY_AND_ASSIGN(DefaultBranchFactory);
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_DEFAULT_BRANCH_FACTORY_H_
