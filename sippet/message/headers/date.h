// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_DATE_H_
#define SIPPET_MESSAGE_HEADERS_DATE_H_

#include "base/time.h"
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Date :
  public Header,
  public single_value<base::Time> {
private:
  DISALLOW_ASSIGN(Date);
  Date(const Date &other) : Header(other), single_value(other) {}
  virtual Date *DoClone() const {
    return new Date(*this);
  }
public:
  Date() : Header(Header::HDR_DATE) {}
  Date(const single_value::value_type &date)
    : Header(Header::HDR_DATE), single_value(date) {}

  scoped_ptr<Date> Clone() const {
    return scoped_ptr<Date>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    const char *wkday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char *month[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    base::Time::Exploded exploded;
    value().UTCExplode(&exploded);

    os.write_hname("Date");
    os << wkday[exploded.day_of_week]
       << ", "
       << format("%.2d %s %d", exploded.day_of_month, month[exploded.month-1], exploded.year)
       << " "
       << format("%.2d:%.2d:%.2d", exploded.hour, exploded.minute, exploded.second)
       << " GMT";
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_DATE_H_
