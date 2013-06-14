// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
#define SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"
#include "googleurl/src/gurl.h"

namespace sippet {

class ErrorUri :
  public single_value<GURL>,
  public has_parameters {
public:
  ErrorUri() {}
  ErrorUri(const ErrorUri &other)
    : has_parameters(other), single_value(other) {}
  explicit ErrorUri(const single_value::value_type &type)
    : single_value(type) {}

  ~ErrorUri() {}

  ErrorUri &operator=(const ErrorUri &other) {
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
raw_ostream &operator<<(raw_ostream &os, const ErrorUri &u) {
  u.print(os);
  return os;
}

class ErrorInfo :
  public Header,
  public has_multiple<ErrorUri> {
private:
  DISALLOW_ASSIGN(ErrorInfo);
  ErrorInfo(const ErrorInfo &other) : Header(other), has_multiple(other) {}
  virtual ErrorInfo *DoClone() const {
    return new ErrorInfo(*this);
  }
public:
  ErrorInfo() : Header(Header::HDR_ERROR_INFO) {}

  scoped_ptr<ErrorInfo> Clone() const {
    return scoped_ptr<ErrorInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Error-Info");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
