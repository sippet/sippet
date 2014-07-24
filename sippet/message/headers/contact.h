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
#include "url/gurl.h"

namespace sippet {

class ContactBase :
  public has_parameters {
 public:
  ContactBase();
  ContactBase(const ContactBase &other);
  explicit ContactBase(const GURL &address,
                       const std::string &displayName="");
  ~ContactBase();

  ContactBase &operator=(const ContactBase &other);

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

  void print(raw_ostream &os) const;

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
  ContactInfo();
  ContactInfo(const ContactInfo &other);
  explicit ContactInfo(const GURL &address,
                       const std::string &displayName="");
  ~ContactInfo();

  ContactInfo &operator=(const ContactInfo &other);
};

class Contact :
  public Header,
  public has_multiple<ContactInfo> {
 private:
  DISALLOW_ASSIGN(Contact);
  Contact(const Contact &other);
  virtual Contact *DoClone() const OVERRIDE;

 public:
  enum _All { All };

  Contact();
  Contact(_All);
  explicit Contact(const ContactInfo &info);
  Contact(const GURL &address,
          const std::string &displayName="");

  scoped_ptr<Contact> Clone() const {
    return scoped_ptr<Contact>(DoClone());
  }

  bool is_all() const {
    return star_;
  }
  void set_all(bool value) {
    star_ = value;
  }

  virtual void print(raw_ostream &os) const OVERRIDE;

private:
  bool star_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTACT_H_
