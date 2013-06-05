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

#ifndef SIPPET_MESSAGE_METHOD_H_
#define SIPPET_MESSAGE_METHOD_H_

#include <string>
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Method {
public:
  enum Type {
    INVITE = 0,
    ACK,
    CANCEL,
    PRACK,
    BYE,
    REFER,
    INFO,
    UPDATE,
    OPTIONS,
    REGISTER,
    MESSAGE,
    SUBSCRIBE,
    NOTIFY,
    PUBLISH,
    PULL,
    PUSH,
    STORE,
    Unknown
  };

  Method() : method_(Unknown) {}
  Method(const Method &other) : method_(other.method_) {}
  explicit Method(const Type &m) : method_(m) {}
  ~Method() {}

  Method &operator=(const Method &other) {
    method_ = other.method_;
    return *this;
  }

  Type type() const { return method_; }
  std::string name() const;

  void print(raw_ostream &os) const {
    os << name();
  }
private:
  Type method_;
};

inline
raw_ostream &operator << (raw_ostream &os, const Method &m) {
  m.print(os);
  return os;
}

} // End of sippet namespace

#endif // SIPPET_MESSAGE_METHOD_H_
