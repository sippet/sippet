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

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Disposition :
  public has_parameters {
public:
  enum Type {
    render, session, icon, alert
  };

  Disposition() {}
  Disposition(const Disposition &other)
    : has_parameters(other), type_(other.type_) {}
  explicit Disposition(Type t) { SetType(t); }
  explicit Disposition(const std::string &type)
    : type_(type) {}

  ~Disposition() {}

  Disposition &operator=(const Disposition &other) {
    type_ = other.type_;
    has_parameters::operator=(other);
    return *this;
  }

  std::string GetType() const { return type_; }

  void SetType(Type t) {
    static const char *rep[] = { "render", "session", "icon", "alert" };
    type_ = rep[static_cast<int>(t)];
  }

  void SetType(const std::string &type) { type_ = type; }

  void print(raw_ostream &os) const {
    os << GetType();
    has_parameters::print(os);
  }
private:
  std::string type_;
};

class ContentDisposition :
  public Header,
  public Disposition {
private:
  ContentDisposition(const ContentDisposition &other)
    : Header(other), Disposition(other) {}
  ContentDisposition &operator=(ContentDisposition &other);
  virtual ContentDisposition *DoClone() const {
    return new ContentDisposition(*this);
  }
public:
  ContentDisposition() : Header(Header::HDR_CONTENT_DISPOSITION) {}
  ContentDisposition(const std::string &type)
    : Header(Header::HDR_CONTENT_DISPOSITION), Disposition(type) {}

  scoped_ptr<ContentDisposition> Clone() const {
    return scoped_ptr<ContentDisposition>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Content-Disposition");
    Disposition::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_
