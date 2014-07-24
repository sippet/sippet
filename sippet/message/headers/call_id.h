// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CALL_ID_H_
#define SIPPET_MESSAGE_HEADERS_CALL_ID_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class CallId :
  public Header,
  public single_value<std::string> {
 private:
  DISALLOW_ASSIGN(CallId);
  CallId(const CallId &other);
  virtual CallId *DoClone() const OVERRIDE;

 public:
  CallId();
  CallId(const single_value::value_type &callid);
  virtual ~CallId();

  scoped_ptr<CallId> Clone() const {
    return scoped_ptr<CallId>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CALL_ID_H_
