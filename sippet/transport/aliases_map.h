// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_ALIASES_MAP_H_
#define SIPPET_TRANSPORT_ALIASES_MAP_H_

#include <vector>
#include <map>
#include <algorithm>
#include "sippet/transport/end_point.h"
#include "base/logging.h"

namespace sippet {

class AliasesMap {
 private:
  typedef std::map<EndPoint, EndPoint, EndPointLess> ReverseMap;
  typedef std::map<EndPoint, std::vector<EndPoint>, EndPointLess> ForwardMap;
  typedef ReverseMap::iterator reverse_iterator;
  typedef ForwardMap::iterator forward_iterator;

 public:
  AliasesMap();
  ~AliasesMap();

  void AddAlias(const EndPoint &target, const EndPoint &alias);
  EndPoint TargetOf(const EndPoint &alias);
  void RemoveAliases(const EndPoint &target);

private:
  ReverseMap reverse_map_;
  ForwardMap forward_map_;

  struct remove_from;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_ALIASES_MAP_H_
