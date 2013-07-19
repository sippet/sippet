// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_PRIORITY_H_
#define SIPPET_MESSAGE_HEADERS_PRIORITY_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Priority :
  public Header,
  public single_value<std::string> {
private:
  DISALLOW_ASSIGN(Priority);
  Priority(const Priority &other) : Header(other), single_value(other) {}
  virtual Priority *DoClone() const OVERRIDE {
    return new Priority(*this);
  }
public:
  enum Level {
    emergency = 0, urgent, normal, non_urgent
  };

  Priority() : Header(Header::HDR_PRIORITY) {}
  Priority(Level l)
    : Header(Header::HDR_PRIORITY) { set_value(l); }
  Priority(const single_value::value_type &priority)
    : Header(Header::HDR_PRIORITY), single_value(priority) {}

  scoped_ptr<Priority> Clone() const {
    return scoped_ptr<Priority>(DoClone());
  }

  void set_value(Level l) {
    const char *rep[] = { "emergency", "urgent", "normal", "non-urgent" };
    single_value::set_value(rep[static_cast<int>(l)]);
  }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    single_value::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PRIORITY_H_
