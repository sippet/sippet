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

#ifndef SIPPET_MESSAGE_ATOM_H_
#define SIPPET_MESSAGE_ATOM_H_

#include <string>
#include <memory>
#include "base/memory/scoped_ptr.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

template<typename T>
struct AtomTraits {};

template<typename T>
class Atom {
public:
  typedef AtomTraits<T> Traits;
  typedef typename Traits::type Type;

  Atom() { set_type(Traits::unknown_type); }
  Atom(const Atom &other)
    : atom_(other.atom_->clone()) {}
  Atom(Type t) { set_type(t); }
  explicit Atom(const char *str) { set_str(str); }
  explicit Atom(const std::string &str) { set_str(str); }
  ~Atom() {}

  Atom &operator=(const Atom &other) {
    atom_.reset(other.atom_->clone());
    return *this;
  }

  bool operator==(Type t) const {
    return type() == t;
  }

  Type type() const { return atom_->type(); }
  void set_type(Type t) {
    static NullAtom null;
    if (t != Traits::unknown_type)
      atom_.reset(new KnownAtom(t));
    else
      atom_.reset(&null);
  }

  const char *str() const { return atom_->str(); }
  void set_str(const std::string &str) {
    set_str(str.c_str());
  }
  void set_str(const char *str) {
    Type t = Traits::coerce(str);
    if (t != Traits::unknown_type)
      atom_.reset(new KnownAtom(t));
    else
      atom_.reset(new UnknownAtom(str));
  }

  void print(raw_ostream &os) const {
    os << str();
  }
private:
  struct AtomImp {
    virtual Type type() = 0;
    virtual const char *str() = 0;
    virtual AtomImp *clone() = 0;
    virtual void release() = 0;
  };

  struct NullAtom : public AtomImp {
    virtual Type type() { return Traits::unknown_type; }
    virtual const char *str() { return ""; }
    virtual NullAtom *clone() { return this; }
    virtual void release() {}
  };
  
  struct KnownAtom : public AtomImp {
    Type type_;
    KnownAtom(Type t) : type_(t) {}
    virtual Type type() { return type_; }
    virtual const char *str() { return Traits::string_of(type_); }
    virtual KnownAtom *clone() { return new KnownAtom(*this); }
    virtual void release() { delete this; }
  };
  
  struct UnknownAtom : public AtomImp {
    std::string atom_;
    UnknownAtom(const char *str) : atom_(str) {}
    virtual Type type() { return Traits::unknown_type; }
    virtual const char *str() { return atom_.c_str(); }
    virtual UnknownAtom *clone() { return new UnknownAtom(*this); }
    virtual void release() { delete this; }
  };

  struct AtomRelease {
    inline void operator()(AtomImp* ptr) const { ptr->release(); }
  };

  scoped_ptr<AtomImp, AtomRelease> atom_;
};

template<typename T>
inline
raw_ostream &operator << (raw_ostream &os, const Atom<T> &a) {
  a.print(os);
  return os;
}

template<typename T>
inline
bool operator==(typename Atom<T>::Type t, const Atom<T> &a) {
  return a == t;
}

} // End of sippet namespace

#endif // SIPPET_MESSAGE_ATOM_H_
