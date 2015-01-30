// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CALL_INFO_H_
#define SIPPET_MESSAGE_HEADERS_CALL_INFO_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/base/raw_ostream.h"
#include "url/gurl.h"

namespace sippet {

class Info :
  public single_value<GURL>,
  public has_parameters,
  public has_purpose<Info> {
 public:
  Info();
  Info(const Info &other);
  explicit Info(const single_value::value_type &type);
  ~Info();

  Info &operator=(const Info &other);

  void print(raw_ostream &os) const;
};

inline
raw_ostream &operator<<(raw_ostream &os, const Info &i) {
  i.print(os);
  return os;
}

class CallInfo :
  public Header,
  public has_multiple<Info> {
 private:
  DISALLOW_ASSIGN(CallInfo);
  CallInfo(const CallInfo &other);
  virtual CallInfo *DoClone() const OVERRIDE;

 public:
  CallInfo();
  virtual ~CallInfo();

  scoped_ptr<CallInfo> Clone() const {
    return scoped_ptr<CallInfo>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CALL_INFO_H_
