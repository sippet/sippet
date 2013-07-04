// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_
#define SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Encoding :
  public single_value<std::string>,
  public has_parameters,
  public has_qvalue<Encoding> {
public:
  Encoding() {}
  Encoding(const Encoding &other)
    : has_parameters(other), single_value(other) {}
  explicit Encoding(const single_value::value_type &value)
    : single_value(value) { /* TODO: convert to lower case */ }

  Encoding &operator=(const Encoding &other) {
    single_value::operator=(other);
    has_parameters::operator=(other);
    return *this;
  }

  ~Encoding() {}

  bool AllowsAll() const { return value() == "*"; }

  void print(raw_ostream &os) const {
    os << value();
    has_parameters::print(os);
  }
};

inline
raw_ostream &operator << (raw_ostream &os, const Encoding &e) {
  e.print(os);
  return os;
}

class AcceptEncoding :
  public Header,
  public has_multiple<Encoding> {
private:
  DISALLOW_ASSIGN(AcceptEncoding);
  AcceptEncoding(const AcceptEncoding &other)
    : Header(other), has_multiple(other) {}
  virtual AcceptEncoding *DoClone() const {
    return new AcceptEncoding(*this);
  }
public:
  AcceptEncoding() : Header(Header::HDR_ACCEPT_ENCODING) {}

  scoped_ptr<AcceptEncoding> Clone() const {
    return scoped_ptr<AcceptEncoding>(DoClone());
  }

  bool AllowsAll() const {
    return !empty() && front().AllowsAll();
  }

  virtual void print(raw_ostream &os) const {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_
