// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/date.h"

namespace sippet {

Date::Date()
  : Header(Header::HDR_DATE) {
}

Date::Date(const single_value::value_type &date)
  : Header(Header::HDR_DATE), single_value(date) {
}

Date::Date(const Date &other)
  : Header(other), single_value(other) {
}

Date::~Date() {
}

Date *Date::DoClone() const {
  return new Date(*this);
}

void Date::print(raw_ostream &os) const {
  const char *wkday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  const char *month[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  base::Time::Exploded exploded;
  value().UTCExplode(&exploded);

  Header::print(os);
  os << wkday[exploded.day_of_week]
     << ", "
     << format("%.2d %s %d", exploded.day_of_month,
               month[exploded.month-1], exploded.year)
     << " "
     << format("%.2d:%.2d:%.2d", exploded.hour,
               exploded.minute, exploded.second)
     << " GMT";
}

}  // namespace sippet
