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

#ifndef SIPPET_MESSAGE_HEADER_H_
#define SIPPET_MESSAGE_HEADER_H_

#include "sippet/base/ilist_node.h"
#include "sippet/base/casting.h"
#include "base/memory/scoped_ptr.h"
#include "sippet/message/atom.h"

namespace sippet {

class raw_ostream;
#define X(class_name, compact_form, header_name, enum_name, format) \
class class_name;
#include "sippet/message/known_headers.h"
#undef X
class Generic;

class Header : public ilist_node<Header> {
public:
  //! An enumeration to indicate the message header type.
  enum Type {
    #define X(class_name, compact_form, header_name, enum_name, format) \
    HDR_##enum_name,
    #include "sippet/message/known_headers.h"
    #undef X
    HDR_GENERIC
  };

private:
  Type type_;

  Header &operator=(const Header &);

protected:
  Header(const Header &other) : type_(other.type_) {}
  Header(Type type) : type_(type) {}

  virtual Header *DoClone() const = 0;

public:
  static scoped_ptr<Header> Parse(const std::string &raw_header);

  Type type() const { return type_; }
  const char *name() const;
  const char compact_form() const;

  scoped_ptr<Header> Clone() const { return scoped_ptr<Header>(DoClone()); }

  virtual void print(raw_ostream &os) const = 0;
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
#include "sippet/message/known_headers.h"
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
