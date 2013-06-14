// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ACCEPT_LANGUAGE_H_
#define SIPPET_MESSAGE_HEADERS_ACCEPT_LANGUAGE_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class LanguageRange :
  public single_value<std::string>,
  public has_parameters,
  public has_qvalue<LanguageRange> {
public:
  LanguageRange() {}
  LanguageRange(const LanguageRange &other)
    : has_parameters(other), single_value(other) {}
  explicit LanguageRange(const std::string &value)
    : single_value(value) { /* TODO: convert to lower case */ }

  ~LanguageRange() {}

  LanguageRange &operator=(const LanguageRange &other) {
    single_value::operator=(other);
    has_parameters::operator=(other);
    return *this;
  }

  bool AllowsAll() const { return value() == "*"; }

  void print(raw_ostream &os) const {
    os << value();
    has_parameters::print(os);
  }
};

inline
raw_ostream &operator << (raw_ostream &os, const LanguageRange &l) {
  l.print(os);
  return os;
}

class AcceptLanguage :
  public Header,
  public has_multiple<LanguageRange> {
private:
  DISALLOW_ASSIGN(AcceptLanguage);
  AcceptLanguage(const AcceptLanguage &other)
    : Header(other), has_multiple(other) {}
  virtual AcceptLanguage *DoClone() const {
    return new AcceptLanguage(*this);
  }
public:
  AcceptLanguage() : Header(Header::HDR_ACCEPT_LANGUAGE) {}

  scoped_ptr<AcceptLanguage> Clone() const {
    return scoped_ptr<AcceptLanguage>(DoClone());
  }

  bool AllowsAll() const {
    return !empty() && front().AllowsAll();
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Accept-Language");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_LANGUAGE_H_
