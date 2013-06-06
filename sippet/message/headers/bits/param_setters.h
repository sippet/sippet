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

#ifndef SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_
#define SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_

#include <string>
#include "base/string_number_conversions.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

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
    while (buffer.size() > 3 && buffer.back() == '0')
      buffer.pop_back();
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
class has_expires {
public:
  bool HasExpires() const {
    return static_cast<const T*>(this)->param_find("expires") !=
           static_cast<const T*>(this)->param_end();
  }

  unsigned expires() const {
    assert(HasExpires() && "Cannot read expires");
    const std::string &value =
      static_cast<const T*>(this)->param_find("expires")->second;
    int ret;
    if (StringToInt(value, &ret))
      return static_cast<unsigned>(ret);
    return 0;
  }

  void set_expires(unsigned seconds) {
    std::string buffer;
    raw_string_ostream os(buffer);
    os << seconds;
    static_cast<T*>(this)->param_set("expires", os.str());
  }
};

template<class T>
class has_tag {
public:
  bool HasTag() const {
    return static_cast<const T*>(this)->param_find("tag") !=
           static_cast<const T*>(this)->param_end();
  }

  std::string tag() const {
    assert(HasTag() && "Cannot read tag");
    return static_cast<const T*>(this)->param_find("tag")->second;
  }

  void set_tag(const std::string &tag) {
    static_cast<T*>(this)->param_set("tag", tag);
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
    assert(HasTag() && "Cannot read handling");
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

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_PARAM_SETTERS_H_
