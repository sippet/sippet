// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_SUBJECT_H_
#define SIPPET_MESSAGE_HEADERS_SUBJECT_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Subject :
  public Header,
  public single_value<std::string> {
 private:
  DISALLOW_ASSIGN(Subject);
  Subject(const Subject &other);
  Subject *DoClone() const override;

 public:
  Subject();
  Subject(const single_value::value_type &subject);
  ~Subject() override;

  scoped_ptr<Subject> Clone() const {
    return scoped_ptr<Subject>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_SUBJECT_H_
