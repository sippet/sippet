// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CALL_INFO_H_
#define SIPPET_MESSAGE_HEADERS_CALL_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"
#include "googleurl/src/gurl.h"

namespace sippet {

class Info :
  public single_value<GURL>,
  public has_parameters,
  public has_purpose<Info> {
public:
  Info() {}
  Info(const Info &other)
    : has_parameters(other), single_value(other) {}
  explicit Info(const single_value::value_type &type)
    : single_value(type) {}

  ~Info() {}

  Info &operator=(const Info &other) {
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
raw_ostream &operator<<(raw_ostream &os, const Info &i) {
  i.print(os);
  return os;
}

class CallInfo :
  public Header,
  public has_multiple<Info> {
private:
  DISALLOW_ASSIGN(CallInfo);
  CallInfo(const CallInfo &other) : Header(other), has_multiple(other) {}
  virtual CallInfo *DoClone() const {
    return new CallInfo(*this);
  }
public:
  CallInfo() : Header(Header::HDR_CALL_INFO) {}

  scoped_ptr<CallInfo> Clone() const {
    return scoped_ptr<CallInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Call-Info");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CALL_INFO_H_
