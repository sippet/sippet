// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/completion_status.h"

#include "net/base/net_errors.h"

namespace sippet {

std::string ErrorToString(int error) {
  if (IsNetError(error)) {
    return net::ErrorToString(error);
  } else {
    return "sippet::" + ErrorToShortString(error);
  }
}

std::string ErrorToShortString(int error) {
  if (IsNetError(error)) {
    return net::ErrorToShortString(error);
  } else if (OK == error) {
    return "OK";
  }

  const char* error_string;
  switch (error) {
#define NO_SUCCESS  // do not include successful cases
#define SIP_STATUS(label, code, reason) \
  case ERR_SIP_ ## label: \
    error_string = "SIP_" # label; \
    break;
#include "sippet/message/status_code_list.h"
#undef SIP_STATUS
#undef NO_SUCCESS

#define Q850_CAUSE(label, code) \
  case ERR_HANGUP_ ## label: \
    error_string = "HANGUP_" # label; \
    break;
#include "sippet/phone/q850.h"
#undef Q850_CAUSE

  default:
    NOTREACHED();
    error_string = "<unknown>";
  }
  return std::string("ERR_") + error_string;
}

int StatusCodeToCompletionStatus(int status_code) {
  if (status_code >= 200 && status_code <= 200)
    return OK;
  return -(1000 + status_code);
}

bool IsNetError(int error) {
  return error < 0 && error > -1000;
}

bool IsStatusCodeError(int error) {
  return error <= -1000 && error > -1700;
}

bool IsHangupCause(int error) {
  return error <= -1700;
}

}  // namespace sippet
