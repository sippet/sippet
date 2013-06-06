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

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_TYPE_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_TYPE_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class MediaType :
  public has_parameters,
  public has_qvalue<MediaType> {
public:
  MediaType() {}
  MediaType(const MediaType &other)
    : has_parameters(other), type_(other.type_), subtype_(other.subtype_) {}
  MediaType(const std::string &type, const std::string &subtype)
    : type_(type), subtype_(subtype)
  { /* TODO: convert to lower case */ }
  ~MediaType() {}

  MediaType &operator=(const MediaType &other) {
    type_ = other.type_;
    subtype_ = other.subtype_;
    has_parameters::operator=(other);
    return *this;
  }

  std::string type() const { return type_; }
  void set_type(const std::string &type) { type_ = type; }

  std::string subtype() const { return subtype_; }
  void set_subtype(const std::string &subtype) { subtype_ = subtype; }

  std::string value() const { return type_ + "/" + subtype_; }

  void print(raw_ostream &os) const {
    os << value();
    has_parameters::print(os);
  }
private:
  std::string type_;
  std::string subtype_;
};

class ContentType :
  public Header,
  public MediaType {
private:
  ContentType(const ContentType &other) : Header(other), MediaType(other) {}
  ContentType &operator=(const ContentType &);
  virtual ContentType *DoClone() const {
    return new ContentType(*this);
  }
public:
  ContentType() : Header(Header::HDR_CONTENT_TYPE) {}
  explicit ContentType(const MediaType &mediaType)
    : Header(Header::HDR_CONTENT_LANGUAGE), MediaType(mediaType) {}

  scoped_ptr<ContentType> Clone() const {
    return scoped_ptr<ContentType>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Content-Type");
    MediaType::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_TYPE_H_
