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
  Timestamp(const Timestamp &other);
  Timestamp *DoClone() const override;

 public:
  Timestamp();
  Timestamp(const double &timestamp, const double &delay=.0);
  ~Timestamp() override;

  std::unique_ptr<Timestamp> Clone() const {
    return std::unique_ptr<Timestamp>(DoClone());
  }

  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  double timestamp() { return timestamp_; }

  void set_delay(double delay) { delay_ = delay; }
  double delay() { return delay_; }

  void print(raw_ostream &os) const override;

private:
  double timestamp_;
  double delay_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_
