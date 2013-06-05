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
#include "sippet/base/raw_ostream.h"

namespace sippet {

class contact_info :
  public has_parameters {
public:
  contact_info() {}
  contact_info(const contact_info &other)
    : has_parameters(other), address_(other.address_),
      displayName_(other.displayName_) {}
  explicit contact_info(const std::string &address,
                        const std::string &displayName="")
    : address_(address), displayName_(displayName) {}

  ~contact_info() {}

  contact_info &operator=(const contact_info &other) {
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
    os << "<" << address_ << ">";
    has_parameters::print(os);
  }
private:
  std::string address_;
  std::string displayName_;
};

class Contact :
  public Header,
  public contact_info {
private:
  Contact(const Contact &other)
    : Header(other), contact_info(other), star_(other.star_) {}
  Contact &operator=(const Contact &);
  virtual Contact *DoClone() const {
    return new Contact(*this);
  }
public:
  enum Star { STAR };

  Contact() : Header(Header::HDR_CONTACT), star_(false) {}
  Contact(Star)
    : Header(Header::HDR_CONTACT), star_(true) {}
  Contact(const std::string &address, const std::string &displayName="")
    : Header(Header::HDR_CONTACT), star_(false),
      contact_info(address, displayName) {}

  scoped_ptr<Contact> Clone() const {
    return scoped_ptr<Contact>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Contact");
    if (star_)
      os << "*";
    else
      contact_info::print(os);
  }
private:
  bool star_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTACT_H_
