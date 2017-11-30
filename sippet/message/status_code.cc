// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/status_code.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"

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

std::string StatusCodeToShortString(int status_code) {
  switch (status_code) {
#define SIP_STATUS(label, code, reason) \
    case SIP_ ## label: \
      return "R_" #label;
#include "sippet/message/status_code_list.h"
#undef SIP_STATUS

    default:
      return base::StringPrintf("R_%d", status_code);
  }
}

}  // namespace sippet
