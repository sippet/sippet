// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_GENERIC_H_
#define SIPPET_MESSAGE_HEADERS_GENERIC_H_

#include "sippet/message/header.h"

namespace sippet {

class Generic :
  public Header {
 private:
  DISALLOW_ASSIGN(Generic);
  Generic(const Generic &other);
  Generic *DoClone() const override;

 public:
  Generic();
  Generic(const std::string &header_name, const std::string &header_value);
  ~Generic() override;

  scoped_ptr<Generic> Clone() const {
    return scoped_ptr<Generic>(DoClone());
  }

  std::string header_name() const {
    return header_name_;
  }
  void set_header_name(const std::string &header_name) {
    header_name_ = header_name;
  }

  std::string header_value() const {
    return header_value_;
  }
  void set_header_value(const std::string &header_value) {
    header_value_ = header_value;
  }

  void print(raw_ostream &os) const override;

 private:
  friend class Header;
  std::string header_name_;
  std::string header_value_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_GENERIC_H_
