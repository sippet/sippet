// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_REASON_H_
#define SIPPET_MESSAGE_HEADERS_REASON_H_

#include "sippet/message/header.h"
#include "sippet/message/status_code.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/bits/param_setters.h"

namespace sippet {

class Reason :
  public Header,
  public single_value<std::string>,
  public has_parameters,
  public has_cause<Reason>,
  public has_text<Reason> {
private:
  DISALLOW_ASSIGN(Reason);
  Reason(const Reason &other);
  Reason *DoClone() const override;

public:
  static const char* kProtoSIP;
  static const char* kQ850;

  Reason();
  Reason(const single_value::value_type &value);
  Reason(StatusCode status_code, const std::string& text="");
  Reason(const std::string& protocol, int cause, const std::string& text);
  ~Reason() override;

  std::unique_ptr<Reason> Clone() const {
    return std::unique_ptr<Reason>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_REASON_H_
