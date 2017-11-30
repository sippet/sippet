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
  RouteParam();
  RouteParam(const RouteParam &other);
  explicit RouteParam(const GURL &address,
                      const std::string &displayName="");
  ~RouteParam();

  RouteParam &operator=(const RouteParam &other);
};

class Route :
  public Header,
  public has_multiple<RouteParam> {
 private:
  DISALLOW_ASSIGN(Route);
  Route(const Route &other);
  Route *DoClone() const override;

 public:
  Route();
  Route(const RouteParam &param);
  ~Route() override;

  std::unique_ptr<Route> Clone() const {
    return std::unique_ptr<Route>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ROUTE_H_
