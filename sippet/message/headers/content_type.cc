// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/content_type.h"

namespace sippet {

MediaType::MediaType() {
}

MediaType::MediaType(const MediaType &other)
  : has_parameters(other), type_(other.type_), subtype_(other.subtype_) {
}

MediaType::MediaType(const std::string &type, const std::string &subtype)
  : type_(type), subtype_(subtype) {
  /* TODO: convert to lower case */
}

MediaType::~MediaType() {
}

MediaType &MediaType::operator=(const MediaType &other) {
  type_ = other.type_;
  subtype_ = other.subtype_;
  has_parameters::operator=(other);
  return *this;
}

void MediaType::print(raw_ostream &os) const {
  os << value();
  has_parameters::print(os);
}

ContentType::ContentType(const ContentType &other)
  : Header(other), MediaType(other) {
}

ContentType *ContentType::DoClone() const {
  return new ContentType(*this);
}

ContentType::ContentType()
  : Header(Header::HDR_CONTENT_TYPE) {
}

ContentType::ContentType(const std::string &type, const std::string &subtype)
  : Header(Header::HDR_CONTENT_TYPE), MediaType(type, subtype) {
}

ContentType::ContentType(const MediaType &mediaType)
  : Header(Header::HDR_CONTENT_TYPE), MediaType(mediaType) {
}

void ContentType::print(raw_ostream &os) const {
  Header::print(os);
  MediaType::print(os);
}

} // namespace sippet
