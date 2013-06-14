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
  RecordRoute(const RecordRoute &other)
    : Header(other), has_multiple(other) {}
  virtual RecordRoute *DoClone() const {
    return new RecordRoute(*this);
  }
public:
  RecordRoute()
    : Header(Header::HDR_RECORD_ROUTE) {}
  RecordRoute(const RouteParam &param)
    : Header(Header::HDR_RECORD_ROUTE) { push_back(param); }

  scoped_ptr<RecordRoute> Clone() const {
    return scoped_ptr<RecordRoute>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Record-Route");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_RECORD_ROUTE_H_
