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

  struct remove_from : std::unary_function<void, EndPoint> {
    ReverseMap &map_;
    remove_from(ReverseMap &from) : map_(from) {}
    void operator()(const EndPoint &alias) {
      map_.erase(alias);
    }
  };

 public:
  AliasesMap() {}
  ~AliasesMap() {}

  void AddAlias(const EndPoint &target, const EndPoint &alias) {
    DCHECK(target.protocol().Equals(alias.protocol()));
    reverse_map_[alias] = target;
    forward_map_[target].push_back(alias);
  }

  EndPoint TargetOf(const EndPoint &alias) {
    reverse_iterator it = reverse_map_.find(alias);
    if (it == reverse_map_.end())
      return EndPoint();
    return it->second;
  }

  void RemoveAliases(const EndPoint &target) {
    forward_iterator fw_it = forward_map_.find(target);
    if (fw_it != forward_map_.end()) {
      std::for_each(fw_it->second.begin(), fw_it->second.end(),
        remove_from(reverse_map_));
      forward_map_.erase(fw_it);
    }
  }

private:
  ReverseMap reverse_map_;
  ForwardMap forward_map_;
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_ALIASES_MAP_H_
