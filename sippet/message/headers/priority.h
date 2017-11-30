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
  Priority(const Priority &other);
  Priority *DoClone() const override;

 public:
  enum Level {
    emergency = 0, urgent, normal, non_urgent
  };

  Priority();
  Priority(Level l);
  Priority(const single_value::value_type &priority);
  ~Priority() override;

  std::unique_ptr<Priority> Clone() const {
    return std::unique_ptr<Priority>(DoClone());
  }

  void set_value(Level l) {
    const char *rep[] = { "emergency", "urgent", "normal", "non-urgent" };
    single_value::set_value(rep[static_cast<int>(l)]);
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PRIORITY_H_
