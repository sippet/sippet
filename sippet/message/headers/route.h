// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ROUTE_H_
#define SIPPET_MESSAGE_HEADERS_ROUTE_H_

#include "sippet/message/headers/contact.h"

namespace sippet {

class RouteParam :
  public ContactBase {
public:
  RouteParam() {}
  RouteParam(const RouteParam &other)
    : ContactBase(other) {}
  explicit RouteParam(const GURL &address,
                      const std::string &displayName="")
    : ContactBase(address, displayName) {}

  ~RouteParam() {}

  RouteParam &operator=(const RouteParam &other) {
    ContactBase::operator=(other);
    return *this;
  }
};

class Route :
  public Header,
  public has_multiple<RouteParam> {
private:
  DISALLOW_ASSIGN(Route);
  Route(const Route &other)
    : Header(other), has_multiple(other) {}
  virtual Route *DoClone() const {
    return new Route(*this);
  }
public:
  Route()
    : Header(Header::HDR_ROUTE) {}
  Route(const RouteParam &param)
    : Header(Header::HDR_ROUTE) { push_back(param); }

  scoped_ptr<Route> Clone() const {
    return scoped_ptr<Route>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Route");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ROUTE_H_
