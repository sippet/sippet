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
#include <memory>
#include "base/memory/scoped_ptr.h"
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

  Method() : method_(&NullMethod::instance) {}
  Method(const Method &other)
    : method_(other.method_->clone()) {}
  explicit Method(const Type &m) : method_(new KnownMethod(m)) {}
  explicit Method(const char *m) : method_(coerce(m)) {}
  explicit Method(const std::string &m) : method_(coerce(m.c_str())) {}
  ~Method() {}

  Method &operator=(const Method &other) {
    method_.reset(other.method_->clone());
    return *this;
  }

  Type type() const { return method_->type(); }
  void set_type(Type t) { method_.reset(new KnownMethod(t)); }

  const char *str() const { return method_->str(); }
  void set_str(const char *str) { method_.reset(coerce(str)); }
  void set_str(const std::string &str) { method_.reset(coerce(str.c_str())); }

  void print(raw_ostream &os) const {
    os << str();
  }
private:
  struct MethodImp {
    virtual Type type() = 0;
    virtual const char *str() = 0;
    virtual MethodImp *clone() = 0;
    virtual void release() = 0;
  };

  struct NullMethod : public MethodImp {
    virtual Type type() { return Unknown; }
    virtual const char *str() { return ""; }
    virtual NullMethod *clone() { return this; }
    virtual void release() {}
    static NullMethod instance;
  };
  
  struct KnownMethod : public MethodImp {
    Type method_;
    KnownMethod(Type method) : method_(method) {}
    virtual Type type() { return method_; }
    virtual const char *str();
    virtual KnownMethod *clone() { return new KnownMethod(*this); }
    virtual void release() { delete this; }
  };
  
  struct UnknownMethod : public MethodImp {
    std::string method_;
    UnknownMethod(const char *m) : method_(m) {}
    virtual Type type() { return Unknown; }
    virtual const char *str() { return method_.c_str(); }
    virtual UnknownMethod *clone() { return new UnknownMethod(*this); }
    virtual void release() { delete this; }
  };

  struct MethodRelease {
    inline void operator()(MethodImp* ptr) const { ptr->release(); }
  };

  scoped_ptr<MethodImp, MethodRelease> method_;

  MethodImp *coerce(const char *str);
};

inline
raw_ostream &operator << (raw_ostream &os, const Method &m) {
  m.print(os);
  return os;
}

} // End of sippet namespace

#endif // SIPPET_MESSAGE_METHOD_H_
