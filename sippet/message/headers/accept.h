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

#ifndef SIPPET_MESSAGE_HEADERS_ACCEPT_H_
#define SIPPET_MESSAGE_HEADERS_ACCEPT_H_

#include "sippet/message/headers/content_type.h"
#include "sippet/message/headers/has_multiple.h"

namespace sippet {

class media_range :
  public media_type {
public:
  media_range() {}
  media_range(const media_range &other)
    : media_type(other) {}
  media_range(const std::string &type, const std::string &subtype)
    : media_type(type, subtype) {}
  ~media_range() {}

  media_range &operator=(const media_range &other) {
    media_type::operator=(other);
    return *this;
  }

  bool allowsAll() { return type() == "*" && allowsAllSubtypes(); }
  bool allowsAllSubtypes() { return subtype() == "*"; }
};

inline
raw_ostream &operator << (raw_ostream &os, const media_range &m) {
  m.print(os);
  return os;
}

class Accept :
  public Header,
  public has_multiple<media_range> {
private:
  Accept(const Accept &other) : Header(other), has_multiple(other) {}
  Accept &operator=(const Accept &);
  virtual Accept *DoClone() const {
    return new Accept(*this);
  }
public:
  Accept() : Header(Header::HDR_ACCEPT) {}

  scoped_ptr<Accept> Clone() const {
    return scoped_ptr<Accept>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Accept");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ACCEPT_H_
