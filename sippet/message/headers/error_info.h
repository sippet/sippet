// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
#define SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"
#include "url/gurl.h"

namespace sippet {

class ErrorUri :
  public single_value<GURL>,
  public has_parameters {
 public:
  ErrorUri();
  ErrorUri(const ErrorUri &other);
  explicit ErrorUri(const single_value::value_type &type);
  ~ErrorUri();

  ErrorUri &operator=(const ErrorUri &other);

  void print(raw_ostream &os) const;
};

inline
raw_ostream &operator<<(raw_ostream &os, const ErrorUri &u) {
  u.print(os);
  return os;
}

class ErrorInfo :
  public Header,
  public has_multiple<ErrorUri> {
 private:
  DISALLOW_ASSIGN(ErrorInfo);
  ErrorInfo(const ErrorInfo &other);
  ErrorInfo *DoClone() const override;

 public:
  ErrorInfo();
  ~ErrorInfo() override;

  scoped_ptr<ErrorInfo> Clone() const {
    return scoped_ptr<ErrorInfo>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ERROR_INFO_H_
