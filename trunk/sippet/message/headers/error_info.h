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

#ifndef SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
#define SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"
#include "googleurl/src/gurl.h"

namespace sippet {

class ErrorUri :
  public single_value<GURL>,
  public has_parameters {
public:
  ErrorUri() {}
  ErrorUri(const ErrorUri &other)
    : has_parameters(other), single_value(other) {}
  explicit ErrorUri(const single_value::value_type &type)
    : single_value(type) {}

  ~ErrorUri() {}

  ErrorUri &operator=(const ErrorUri &other) {
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
raw_ostream &operator<<(raw_ostream &os, const ErrorUri &u) {
  u.print(os);
  return os;
}

class ErrorInfo :
  public Header,
  public has_multiple<ErrorUri> {
private:
  DISALLOW_ASSIGN(ErrorInfo);
  ErrorInfo(const ErrorInfo &other) : Header(other), has_multiple(other) {}
  virtual ErrorInfo *DoClone() const {
    return new ErrorInfo(*this);
  }
public:
  ErrorInfo() : Header(Header::HDR_ERROR_INFO) {}

  scoped_ptr<ErrorInfo> Clone() const {
    return scoped_ptr<ErrorInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Error-Info");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
