// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADER_H_
#define SIPPET_MESSAGE_HEADER_H_

#include "sippet/base/ilist.h"
#include "sippet/base/ilist_node.h"
#include "sippet/base/casting.h"
#include "base/memory/scoped_ptr.h"
#include "sippet/message/atom.h"

namespace sippet {

class raw_ostream;
#define X(class_name, compact_form, header_name, enum_name, format) \
class class_name;
#include "sippet/message/header_list.h"
#undef X
class Generic;

class Header : public ilist_node<Header> {
 public:
  //! An enumeration to indicate the message header type.
  enum Type {
    #define X(class_name, compact_form, header_name, enum_name, format) \
    HDR_##enum_name,
    #include "sippet/message/header_list.h"
    #undef X
    HDR_GENERIC
  };

 private:
  Type type_;

  Header &operator=(const Header &);

 protected:
  friend struct base::DefaultDeleter<Header>;
  friend struct ilist_node_traits<Header>;

  Header(const Header &other);
  Header(Type type);
  virtual ~Header();

  virtual Header *DoClone() const = 0;

 public:
  static scoped_ptr<Header> Parse(const std::string &raw_header);

  Type type() const { return type_; }
  const char *name() const;
  const char compact_form() const;

  scoped_ptr<Header> Clone() const { return scoped_ptr<Header>(DoClone()); }

  virtual void print(raw_ostream &os) const;

  std::string ToString() {
    std::string result;
    raw_string_ostream os(result);
    print(os);
    return os.str();
  }
};

// isa - Provide some specializations of isa so that we don't have to include
// the subtype header files to test to see if the value is a subclass...
//
#define X(class_name, compact_form, header_name, enum_name, format) \
template <> struct isa_impl<class_name, Header> {                   \
  static inline bool doit(const Header &h) {                        \
    return h.type() == Header::HDR_##enum_name;                     \
  }                                                                 \
};
#include "sippet/message/header_list.h"
#undef X

template <> struct isa_impl<Generic, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_GENERIC;
  }
};

template<>
struct AtomTraits<Header::Type> {
  typedef Header::Type type;
  static const type unknown_type = Header::HDR_GENERIC;
  static const char *string_of(type t);
  static type coerce(const char *str);
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADER_H_
