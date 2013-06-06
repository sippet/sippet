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

#ifndef SIPPET_MESSAGE_HEADERS_GENERIC_H_
#define SIPPET_MESSAGE_HEADERS_GENERIC_H_

#include "sippet/message/header.h"

namespace sippet {

class Generic :
  public Header {
private:
  DISALLOW_ASSIGN(Generic);
  Generic(const Generic &other)
    : Header(other), header_name_(other.header_name_),
      header_value_(other.header_value_) {}
  virtual Generic *DoClone() const {
    return new Generic(*this);
  }
public:
  Generic() : Header(Header::HDR_GENERIC) {}
  Generic(const std::string &header_name, const std::string &header_value)
    : Header(Header::HDR_GENERIC), header_name_(header_name),
      header_value_(header_value) {}

  scoped_ptr<Generic> Clone() const {
    return scoped_ptr<Generic>(DoClone());
  }

  std::string header_name() const {
    return header_name_;
  }
  void set_header_name(const std::string &header_name) {
    header_name_ = header_name;
  }

  std::string header_value() const {
    return header_value_;
  }
  void set_header_value(const std::string &header_value) {
    header_value_ = header_value;
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname(header_name_);
    os << header_value_;
  }
private:
  std::string header_name_;
  std::string header_value_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_GENERIC_H_
