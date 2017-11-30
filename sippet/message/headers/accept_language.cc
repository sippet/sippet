// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/accept_language.h"
#include "base/strings/string_util.h"

namespace sippet {

LanguageRange::LanguageRange() {
}

LanguageRange::LanguageRange(const LanguageRange &other)
  : single_value(other), has_parameters(other) {
}

LanguageRange::LanguageRange(const std::string &value)
  : single_value(base::ToLowerASCII(value)) {
}

LanguageRange::~LanguageRange() {
}

LanguageRange &LanguageRange::operator=(const LanguageRange &other) {
  single_value::operator=(other);
  has_parameters::operator=(other);
  return *this;
}

void LanguageRange::print(raw_ostream &os) const {
  os << value();
  has_parameters::print(os);
}

AcceptLanguage::AcceptLanguage()
  : Header(Header::HDR_ACCEPT_LANGUAGE) {
}

AcceptLanguage::AcceptLanguage(const AcceptLanguage &other)
  : Header(other), has_multiple(other) {
}

AcceptLanguage::~AcceptLanguage() {
}

AcceptLanguage *AcceptLanguage::DoClone() const {
  return new AcceptLanguage(*this);
}

void AcceptLanguage::print(raw_ostream &os) const {
  Header::print(os);
  has_multiple::print(os);
}

}  // namespace sippet
