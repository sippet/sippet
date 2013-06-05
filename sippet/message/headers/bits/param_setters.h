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

#ifndef SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_
#define SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_

#include <cstdlib>
#include <string>
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
    assert(HasQvalue() && "Cannot read qvalue");
    return atof(static_cast<const T*>(this)->param_find("q")->second.c_str());
  }

  void set_qvalue(double q) {
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
    static_cast<T*>(this)->param_set("purpose", rep[static_int<int>(t)]);
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
    return static_cast<unsigned>(atoi(value.c_str()));
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

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_
