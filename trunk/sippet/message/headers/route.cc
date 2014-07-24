// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/route.h"

namespace sippet {

RouteParam::RouteParam() {
}

RouteParam::RouteParam(const RouteParam &other)
  : ContactBase(other) {
}

RouteParam::RouteParam(const GURL &address,
                       const std::string &displayName)
  : ContactBase(address, displayName) {
}

RouteParam::~RouteParam() {
}

RouteParam &RouteParam::operator=(const RouteParam &other) {
  ContactBase::operator=(other);
  return *this;
}

Route::Route()
  : Header(Header::HDR_ROUTE) {
}

Route::Route(const RouteParam &param)
  : Header(Header::HDR_ROUTE) {
  push_back(param);
}

Route::Route(const Route &other)
  : Header(other), has_multiple(other) {
}

Route::~Route() {
}

Route *Route::DoClone() const {
  return new Route(*this);
}

void Route::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

} // End of sippet namespace
