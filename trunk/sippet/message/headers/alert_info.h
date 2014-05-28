// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_
#define SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"
#include "url/gurl.h"

namespace sippet {

class AlertParam :
  public single_value<GURL>,
  public has_parameters {
public:
  AlertParam() {}
  AlertParam(const AlertParam &other)
    : has_parameters(other), single_value(other) {}
  explicit AlertParam(const single_value::value_type &type)
    : single_value(type) {}

  ~AlertParam() {}

  AlertParam &operator=(const AlertParam &other) {
    single_value::operator=(other);
    has_parameters::operator=(other);
    return *this;
  }

  void print(raw_ostream &os) const {
    os << "<" << value().spec() << ">";
    has_parameters::print(os);
  }
};

inline
raw_ostream &operator<<(raw_ostream &os, const AlertParam &p) {
  p.print(os);
  return os;
}

class AlertInfo :
  public Header,
  public has_multiple<AlertParam> {
private:
  DISALLOW_ASSIGN(AlertInfo);
  AlertInfo(const AlertInfo &other) : Header(other), has_multiple(other) {}
  virtual AlertInfo *DoClone() const OVERRIDE {
    return new AlertInfo(*this);
  }
public:
  AlertInfo() : Header(Header::HDR_ALERT_INFO) {}

  scoped_ptr<AlertInfo> Clone() const {
    return scoped_ptr<AlertInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_
