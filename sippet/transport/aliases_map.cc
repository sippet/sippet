// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/aliases_map.h"

#include <functional>

namespace sippet {

struct AliasesMap::remove_from
  : std::unary_function<void, EndPoint> {
  ReverseMap &map_;
  explicit remove_from(ReverseMap &from) : map_(from) {}
  void operator()(const EndPoint &alias) {
    map_.erase(alias);
  }
};

AliasesMap::AliasesMap() {
}

AliasesMap::~AliasesMap() {
}

void AliasesMap::AddAlias(const EndPoint &target, const EndPoint &alias) {
  DCHECK(target.protocol().Equals(alias.protocol()));
  reverse_map_[alias] = target;
  forward_map_[target].push_back(alias);
}

EndPoint AliasesMap::TargetOf(const EndPoint &alias) {
  reverse_iterator it = reverse_map_.find(alias);
  if (it == reverse_map_.end())
    return EndPoint();
  return it->second;
}

void AliasesMap::RemoveAliases(const EndPoint &target) {
  forward_iterator fw_it = forward_map_.find(target);
  if (fw_it != forward_map_.end()) {
    std::for_each(fw_it->second.begin(), fw_it->second.end(),
      remove_from(reverse_map_));
    forward_map_.erase(fw_it);
  }
}

}  // namespace sippet
