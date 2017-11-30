// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_BRANCH_FACTORY_H_
#define SIPPET_TRANSPORT_BRANCH_FACTORY_H_

#include <string>

namespace sippet {

class BranchFactory {
 public:
  virtual ~BranchFactory() {}
  virtual std::string CreateBranch() = 0;

  // Returns the default BranchFactory.
  static BranchFactory *GetDefaultBranchFactory();
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_BRANCH_FACTORY_H_
