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

#ifndef SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_
#define SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/has_multiple.h"
#include "sippet/message/headers/has_parameters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class encoding :
  public has_parameters {
public:
  encoding() {}
  encoding(const encoding &other)
    : has_parameters(other), encoding_(other.encoding_) {}
  explicit encoding(const std::string &enc)
    : encoding_(enc)
  { /* TODO: convert to lower case */ }

  encoding &operator=(const encoding &other) {
    encoding_ = other.encoding_;
    has_parameters::operator=(other);
    return *this;
  }

  ~encoding() {}

  std::string value() const { return encoding_; }

  bool allowsAll() { return encoding_ == "*"; }

  void print(raw_ostream &os) const {
    os << value();
    has_parameters::print(os);
  }
private:
  std::string encoding_;
};

inline
raw_ostream &operator << (raw_ostream &os, const encoding &e) {
  e.print(os);
  return os;
}

class AcceptEncoding :
  public Header,
  public has_multiple<encoding> {
private:
  AcceptEncoding(const AcceptEncoding &other)
    : Header(other), has_multiple(other) {}
  AcceptEncoding &operator=(const AcceptEncoding &);
  virtual AcceptEncoding *DoClone() const {
    return new AcceptEncoding(*this);
  }
public:
  AcceptEncoding() : Header(Header::HDR_ACCEPT_ENCODING) {}

  scoped_ptr<AcceptEncoding> Clone() const {
    return scoped_ptr<AcceptEncoding>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Accept-Encoding");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_ENCODING_H_
