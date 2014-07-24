// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_
#define SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_

#include <string>

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"
#include "url/gurl.h"

namespace sippet {

class AlertParam :
  public single_value<GURL>,
  public has_parameters {
 public:
  AlertParam();
  AlertParam(const AlertParam &other);
  explicit AlertParam(const single_value::value_type &type);
  ~AlertParam();

  AlertParam &operator=(const AlertParam &other);

  void print(raw_ostream &os) const;
};

inline
raw_ostream &operator<<(raw_ostream &os, const AlertParam &p) {
  p.print(os);
  return os;
}

class AlertInfo :
  public Header,
  public has_multiple<AlertParam> {
 private:
  DISALLOW_ASSIGN(AlertInfo);
  AlertInfo(const AlertInfo &other);
  virtual AlertInfo *DoClone() const OVERRIDE;

 public:
  AlertInfo();

  scoped_ptr<AlertInfo> Clone() const {
    return scoped_ptr<AlertInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ALERT_INFO_H_
