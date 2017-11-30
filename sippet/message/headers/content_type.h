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
  MediaType();
  MediaType(const MediaType &other);
  MediaType(const std::string &type, const std::string &subtype);
  ~MediaType();

  MediaType &operator=(const MediaType &other);

  std::string type() const { return type_; }
  void set_type(const std::string &type) { type_ = type; }

  std::string subtype() const { return subtype_; }
  void set_subtype(const std::string &subtype) { subtype_ = subtype; }

  std::string value() const { return type_ + "/" + subtype_; }

  void print(raw_ostream &os) const;

 private:
  std::string type_;
  std::string subtype_;
};

class ContentType :
  public Header,
  public MediaType {
 private:
  DISALLOW_ASSIGN(ContentType);
  ContentType(const ContentType &other);
  ContentType *DoClone() const override;
 public:
  ContentType();
  ContentType(const std::string &type, const std::string &subtype);
  explicit ContentType(const MediaType &mediaType);
  ~ContentType() override;

  std::unique_ptr<ContentType> Clone() const {
    return std::unique_ptr<ContentType>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_TYPE_H_
