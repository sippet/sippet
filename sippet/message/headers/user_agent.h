// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_USER_AGENT_H_
#define SIPPET_MESSAGE_HEADERS_USER_AGENT_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class UserAgent :
  public Header,
  public single_value<std::string> {
 private:
  DISALLOW_ASSIGN(UserAgent);
  UserAgent(const UserAgent &other);
  UserAgent *DoClone() const override;

 public:
  UserAgent();
  UserAgent(const single_value::value_type &subject);
  ~UserAgent() override;

  scoped_ptr<UserAgent> Clone() const {
    return scoped_ptr<UserAgent>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_USER_AGENT_H_
