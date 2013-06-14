// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

namespace sippet {

void Message::print(raw_ostream &os) const
{
  const_iterator i = headers_.begin(), ie = headers_.end();
  for (; i != ie; ++i) {
    i->print(os);
  }
}

}
