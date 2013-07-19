// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  DISALLOW_ASSIGN(ContentType);
  ContentType(const ContentType &other) : Header(other), MediaType(other) {}
  virtual ContentType *DoClone() const OVERRIDE {
    return new ContentType(*this);
  }
public:
  ContentType() : Header(Header::HDR_CONTENT_TYPE) {}
  ContentType(const std::string &type, const std::string &subtype)
    : Header(Header::HDR_CONTENT_TYPE), MediaType(type, subtype) {}
  explicit ContentType(const MediaType &mediaType)
    : Header(Header::HDR_CONTENT_TYPE), MediaType(mediaType) {}

  scoped_ptr<ContentType> Clone() const {
    return scoped_ptr<ContentType>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    MediaType::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_TYPE_H_
