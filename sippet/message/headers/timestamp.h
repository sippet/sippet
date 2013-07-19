// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_
#define SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_

#include <string>
#include <cmath>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"
#include "base/strings/string_number_conversions.h"

namespace sippet {

class Timestamp :
  public Header {
private:
  DISALLOW_ASSIGN(Timestamp);
  Timestamp(const Timestamp &other)
    : Header(other), timestamp_(other.timestamp_), delay_(other.delay_) {}
  virtual Timestamp *DoClone() const OVERRIDE {
    return new Timestamp(*this);
  }
public:
  Timestamp() : Header(Header::HDR_TIMESTAMP), timestamp_(0), delay_(0) {}
  Timestamp(const double &timestamp, const double &delay=.0)
    : Header(Header::HDR_TIMESTAMP), timestamp_(timestamp), delay_(delay) {}

  scoped_ptr<Timestamp> Clone() const {
    return scoped_ptr<Timestamp>(DoClone());
  }

  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  double timestamp() { return timestamp_; }

  void set_delay(double delay) { delay_ = delay; }
  double delay() { return delay_; }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    print_double(os, timestamp_);
    if (delay_ != 0) {
      os << " ";
      print_double(os, delay_);
    }
  }

private:
  double timestamp_;
  double delay_;

  static void print_double(raw_ostream &os, double v) {
    double i;
    double frac = modf(v, &i);
    if (frac == 0)
      os << static_cast<int>(i);
    else
      os << base::DoubleToString(v);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_
