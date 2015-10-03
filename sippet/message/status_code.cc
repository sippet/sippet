// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/status_code.h"
#include "base/logging.h"

namespace sippet {

const char* GetReasonPhrase(StatusCode code) {
  switch (code) {
#define SIP_STATUS(label, code, reason) case SIP_ ## label: return reason;
#include "sippet/message/status_code_list.h"
#undef SIP_STATUS

    default:
      NOTREACHED() << "unknown SIP status code " << code;
  }

  return "";
}

}  // namespace sippet
