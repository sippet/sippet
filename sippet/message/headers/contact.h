/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

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
      displayName_(other.displayName_) {}
  explicit ContactBase(const GURL &address,
                       const std::string &displayName="")
    : address_(address), displayName_(displayName) {}

  ~ContactBase() {}

  ContactBase &operator=(const ContactBase &other) {
    address_ = other.address_;
    displayName_ = other.displayName_;
    has_parameters::operator=(other);
    return *this;
  }

  void print(raw_ostream &os) const {
    if (!displayName_.empty()) {
      os << "\"";
      os.write_escaped(displayName_);
      os << "\" ";
    }
    os << "<" << address_.spec() << ">";
    has_parameters::print(os);
  }
private:
  GURL address_;
  std::string displayName_;
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
