// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_RECORD_ROUTE_H_
#define SIPPET_MESSAGE_HEADERS_RECORD_ROUTE_H_

#include "sippet/message/headers/route.h"

namespace sippet {

class RecordRoute :
  public Header,
  public has_multiple<RouteParam> {
 private:
  DISALLOW_ASSIGN(RecordRoute);
  RecordRoute(const RecordRoute &other);
  RecordRoute *DoClone() const override;

 public:
  RecordRoute();
  RecordRoute(const RouteParam &param);
  ~RecordRoute() override;

  scoped_ptr<RecordRoute> Clone() const {
    return scoped_ptr<RecordRoute>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_RECORD_ROUTE_H_
