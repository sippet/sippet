// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/contact.h"

namespace sippet {

ContactBase::ContactBase() {
}

ContactBase::ContactBase(const ContactBase &other)
  : has_parameters(other), address_(other.address_),
    display_name_(other.display_name_) {
}

ContactBase::ContactBase(const GURL &address,
                         const std::string &displayName)
  : address_(address), display_name_(displayName) {
}

ContactBase::~ContactBase() {
}

ContactBase &ContactBase::operator=(const ContactBase &other) {
  address_ = other.address_;
  display_name_ = other.display_name_;
  has_parameters::operator=(other);
  return *this;
}

void ContactBase::print(raw_ostream &os) const {
  if (!display_name_.empty()) {
    os << "\"";
    os.write_escaped(display_name_);
    os << "\" ";
  }
  os << "<" << address_.spec() << ">";
  has_parameters::print(os);
}

ContactInfo::ContactInfo() {
}

ContactInfo::ContactInfo(const ContactInfo &other)
  : ContactBase(other) {
}

ContactInfo::ContactInfo(const GURL &address,
                         const std::string &displayName)
  : ContactBase(address, displayName) {
}

ContactInfo::~ContactInfo() {}

ContactInfo &ContactInfo::operator=(const ContactInfo &other) {
  ContactBase::operator=(other);
  return *this;
}

Contact::Contact(const Contact &other)
  : Header(other), has_multiple(other), star_(other.star_) {
}

Contact *Contact::DoClone() const {
  return new Contact(*this);
}

Contact::Contact()
  : Header(Header::HDR_CONTACT), star_(false) {
}

Contact::Contact(_All)
  : Header(Header::HDR_CONTACT), star_(true) {
}

Contact::Contact(const ContactInfo &info)
  : Header(Header::HDR_CONTACT), star_(false) {
  push_back(info);
}

Contact::Contact(const GURL &address,
                 const std::string &displayName)
  : Header(Header::HDR_CONTACT), star_(false) {
  push_back(ContactInfo(address, displayName));
}

Contact::~Contact() {
}

void Contact::print(raw_ostream &os) const {
  Header::print(os);
  if (star_)
    os << "*";
  else
    has_multiple::print(os);
}

} // End of sippet namespace
