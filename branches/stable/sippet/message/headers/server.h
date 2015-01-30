// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_SERVER_H_
#define SIPPET_MESSAGE_HEADERS_SERVER_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Server :
  public Header,
  public single_value<std::string> {
 private:
  DISALLOW_ASSIGN(Server);
  Server(const Server &other);
  virtual Server *DoClone() const OVERRIDE;

 public:
  Server();
  Server(const single_value::value_type &subject);
  virtual ~Server();

  scoped_ptr<Server> Clone() const {
    return scoped_ptr<Server>(DoClone());
  }

  virtual void print(raw_ostream &os) const OVERRIDE;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_SERVER_H_
