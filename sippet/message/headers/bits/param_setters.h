// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_
#define SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_

#include <string>
#include "base/basictypes.h"
#include "base/strings/string_number_conversions.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

#define string_token_param_template(cls, Name, Capitalized)         \
template<class T>                                                   \
class cls {                                                         \
public:                                                             \
  bool Has##Capitalized() const {                                   \
    return static_cast<const T*>(this)->param_find(#Name) !=        \
           static_cast<const T*>(this)->param_end();                \
  }                                                                 \
  std::string Name() const {                                        \
    assert(Has##Capitalized() && "Cannot read " #Name);             \
    return static_cast<const T*>(this)->param_find(#Name)->second;  \
  }                                                                 \
  void set_##Name(const std::string &Name) {                        \
    static_cast<T*>(this)->param_set(#Name, Name);                  \
  }                                                                 \
};

string_token_param_template(has_tag,        tag,        Tag         )
string_token_param_template(has_maddr,      maddr,      Maddr       )
string_token_param_template(has_branch,     branch,     Branch      )

#undef string_token_param_template


#define integer_token_param_template(cls, Name, Capitalized, type)  \
template<class T>                                                   \
class cls {                                                         \
public:                                                             \
  bool Has##Capitalized() const {                                   \
    return static_cast<const T*>(this)->param_find(#Name) !=        \
           static_cast<const T*>(this)->param_end();                \
  }                                                                 \
  type Name() const {                                               \
    assert(Has##Capitalized() && "Cannot read " #Name);             \
    const std::string &value =                                      \
      static_cast<const T*>(this)->param_find(#Name)->second;       \
    int ret;                                                        \
    if (base::StringToInt(value, &ret))                             \
      return static_cast<type>(ret);                                \
    return 0;                                                       \
  }                                                                 \
  void set_##Name(type Name) {                                      \
    std::string buffer;                                             \
    raw_string_ostream os(buffer);                                  \
    os << Name;                                                     \
    static_cast<T*>(this)->param_set(#Name, os.str());              \
  }                                                                 \
};

integer_token_param_template(has_ttl,     ttl,     Ttl,     unsigned)
integer_token_param_template(has_expires, expires, Expires, unsigned)
integer_token_param_template(has_rport,   rport,   Rport,   uint16  )

#undef integer_token_param_template


template<class T>
class has_qvalue {
public:
  bool HasQvalue() const {
    return static_cast<const T*>(this)->param_find("q") !=
           static_cast<const T*>(this)->param_end();
  }

  double qvalue() const {
    double v;
    assert(HasQvalue() && "Cannot read qvalue");
    if (base::StringToDouble(static_cast<const T*>(this)->param_find("q")->second, &v))
      return v;
    return 0;
  }

  void set_qvalue(double q) {
    assert(q > 0 && "Invalid qvalue");
    std::string buffer;
    raw_string_ostream os(buffer);
    os << format("%.3f", q);
    os.flush();
    while (buffer.size() > 3 && *buffer.rbegin() == '0')
      buffer.resize(buffer.size()-1);
    static_cast<T*>(this)->param_set("q", buffer);
  }
};

template<class T>
class has_purpose {
public:
  enum PurposeType {
    icon = 0, info, card
  };

  bool HasPurpose() const {
    return static_cast<const T*>(this)->param_find("purpose") !=
           static_cast<const T*>(this)->param_end();
  }

  std::string purpose() const {
    assert(HasPurpose() && "Cannot read purpose");
    return static_cast<const T*>(this)->param_find("purpose")->second;
  }

  void set_purpose(PurposeType t) {
    static const char *rep[] = { "icon", "info", "card" };
    static_cast<T*>(this)->param_set("purpose", rep[static_cast<int>(t)]);
  }

  void set_purpose(const std::string &purpose) {
    static_cast<T*>(this)->param_set("purpose", purpose);
  }
};

template<class T>
class has_handling {
public:
  enum HandlingType {
    optional = 0, required
  };

  bool HasHandling() const {
    return static_cast<const T*>(this)->param_find("handling") !=
           static_cast<const T*>(this)->param_end();
  }

  std::string handling() const {
    assert(HasHandling() && "Cannot read handling");
    return static_cast<const T*>(this)->param_find("handling")->second;
  }

  void set_handling(HandlingType t) {
    static const char *rep[] = { "optional", "required" };
    static_cast<T*>(this)->param_set("handling", rep[static_cast<int>(t)]);
  }

  void set_handling(const std::string &handling) {
    static_cast<T*>(this)->param_set("handling", handling);
  }
};

template<class T>
class has_received {
 public:
  bool HasReceived() const {
    return static_cast<const T*>(this)->param_find("received") !=
           static_cast<const T*>(this)->param_end();
  }

  std::string received() const {
    assert(HasReceived() && "Cannot read received");
    return remove_sqb(
      static_cast<const T*>(this)->param_find("received")->second);
  }

  void set_received(const std::string& received) {
    static_cast<T*>(this)->param_set("received", remove_sqb(received));
  }

 private:
  static std::string remove_sqb(const std::string& input) {
    if (input.front() == '[' && input.back() == ']')
      return input.substr(1, input.size()-2);
    else
      return input;
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_
