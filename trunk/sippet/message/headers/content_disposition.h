// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_
#define SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/param_setters.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class ContentDisposition :
  public Header,
  public single_value<std::string>,
  public has_parameters,
  public has_handling<ContentDisposition> {
 private:
  DISALLOW_ASSIGN(ContentDisposition);
  ContentDisposition(const ContentDisposition &other);
  virtual ContentDisposition *DoClone() const override;

 public:
  enum Type {
    render = 0, session, icon, alert
  };

  ContentDisposition();
  ContentDisposition(Type t);
  ContentDisposition(const single_value::value_type &value);
  virtual ~ContentDisposition();

  scoped_ptr<ContentDisposition> Clone() const {
    return scoped_ptr<ContentDisposition>(DoClone());
  }

  void set_value(Type t) {
    static const char *rep[] = { "render", "session", "icon", "alert" };
    single_value::set_value(rep[static_cast<int>(t)]);
  }

  virtual void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CONTENT_DISPOSITION_H_
