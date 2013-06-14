// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_ATOM_H_
#define SIPPET_MESSAGE_ATOM_H_

#include <string>
#include <memory>
#include "base/memory/scoped_ptr.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

template<typename T>
struct AtomTraits;

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
  bool operator!=(Type t) const {
    return !operator==(t);
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
