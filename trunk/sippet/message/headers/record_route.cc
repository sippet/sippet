// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/record_route.h"

namespace sippet {

RecordRoute::RecordRoute()
  : Header(Header::HDR_RECORD_ROUTE) {
}

RecordRoute::RecordRoute(const RouteParam &param)
  : Header(Header::HDR_RECORD_ROUTE) {
  push_back(param);
}

RecordRoute::RecordRoute(const RecordRoute &other)
  : Header(other), has_multiple(other) {
}

RecordRoute::~RecordRoute() {
}

RecordRoute *RecordRoute::DoClone() const {
  return new RecordRoute(*this);
}

void RecordRoute::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
