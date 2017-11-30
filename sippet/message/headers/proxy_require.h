// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_PROXY_REQUIRE_H_
#define SIPPET_MESSAGE_HEADERS_PROXY_REQUIRE_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class ProxyRequire :
  public Header,
  public has_multiple<std::string> {
 private:
  DISALLOW_ASSIGN(ProxyRequire);
  ProxyRequire(const ProxyRequire &other);
  ProxyRequire *DoClone() const override;

 public:
  ProxyRequire();
  ProxyRequire(const std::string &value);
  ~ProxyRequire() override;

  std::unique_ptr<ProxyRequire> Clone() const {
    return std::unique_ptr<ProxyRequire>(DoClone());
  }

  void print(raw_ostream &os) const override;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PROXY_REQUIRE_H_
