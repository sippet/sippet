// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CONTACT_H_
#define SIPPET_MESSAGE_HEADERS_CONTACT_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"
#include "googleurl/src/gurl.h"

namespace sippet {

class ContactBase :
  public has_parameters {
public:
  ContactBase() {}
  ContactBase(const ContactBase &other)
    : has_parameters(other), address_(other.address_),
      display_name_(other.display_name_) {}
  explicit ContactBase(const GURL &address,
                       const std::string &displayName="")
    : address_(address), display_name_(displayName) {}

  ~ContactBase() {}

  ContactBase &operator=(const ContactBase &other) {
    address_ = other.address_;
    display_name_ = other.display_name_;
    has_parameters::operator=(other);
    return *this;
  }

  std::string display_name() const {
    return display_name_;
  }
  void set_display_name(const std::string &display_name) {
    display_name_ = display_name;
  }

  GURL address() const {
    return address_;
  }
  void set_address(const GURL &address) {
    address_ = address;
  }

  void print(raw_ostream &os) const {
    if (!display_name_.empty()) {
      os << "\"";
      os.write_escaped(display_name_);
      os << "\" ";
    }
    os << "<" << address_.spec() << ">";
    has_parameters::print(os);
  }
private:
  GURL address_;
  std::string display_name_;
};

inline
raw_ostream &operator<<(raw_ostream &os, const ContactBase &b) {
  b.print(os);
  return os;
}

class ContactInfo :
  public ContactBase,
  public has_qvalue<ContactInfo>,
  public has_expires<ContactInfo> {
public:
  ContactInfo() {}
  ContactInfo(const ContactInfo &other)
    : ContactBase(other) {}
  explicit ContactInfo(const GURL &address,
                       const std::string &displayName="")
    : ContactBase(address, displayName) {}

  ~ContactInfo() {}

  ContactInfo &operator=(const ContactInfo &other) {
    ContactBase::operator=(other);
    return *this;
  }
};

class Contact :
  public Header,
  public has_multiple<ContactInfo> {
private:
  DISALLOW_ASSIGN(Contact);
  Contact(const Contact &other)
    : Header(other), has_multiple(other), star_(other.star_) {}
  virtual Contact *DoClone() const {
    return new Contact(*this);
  }
public:
  enum _All { All };

  Contact()
    : Header(Header::HDR_CONTACT), star_(false) {}
  Contact(_All)
    : Header(Header::HDR_CONTACT), star_(true) {}
  explicit Contact(const ContactInfo &info)
    : Header(Header::HDR_CONTACT), star_(false) {
    push_back(info);
  }
  Contact(const GURL &address,
          const std::string &displayName="")
    : Header(Header::HDR_CONTACT), star_(false) {
    push_back(ContactInfo(address, displayName));
  }

  scoped_ptr<Contact> Clone() const {
    return scoped_ptr<Contact>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Contact");
    if (star_)
      os << "*";
    else
      has_multiple::print(os);
  }
private:
  bool star_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTACT_H_
