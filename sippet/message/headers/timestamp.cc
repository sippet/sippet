// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/timestamp.h"

namespace sippet {

namespace {

void print_double(raw_ostream *os, double v) {
  double i;
  double frac = modf(v, &i);
  if (frac == 0)
    *os << static_cast<int>(i);
  else
    *os << base::DoubleToString(v);
}

}  // namespace

Timestamp::Timestamp()
  : Header(Header::HDR_TIMESTAMP), timestamp_(0), delay_(0) {
}

Timestamp::Timestamp(const double &timestamp, const double &delay)
  : Header(Header::HDR_TIMESTAMP), timestamp_(timestamp), delay_(delay) {
}

Timestamp::Timestamp(const Timestamp &other)
  : Header(other), timestamp_(other.timestamp_), delay_(other.delay_) {
}

Timestamp::~Timestamp() {
}

Timestamp *Timestamp::DoClone() const {
  return new Timestamp(*this);
}

void Timestamp::print(raw_ostream &os) const {
  Header::print(os);
  print_double(&os, timestamp_);
  if (delay_ != 0) {
    os << " ";
    print_double(&os, delay_);
  }
}

}  // namespace sippet
