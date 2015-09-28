// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_COMPLETION_STATUS_H_
#define SIPPET_PHONE_COMPLETION_STATUS_H_

#include <string>

namespace sippet {

// Completion status: the several possible status codes returned on
// asynchronous completion callbacks.
//
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.sippet.phone
enum CompletionStatus {
  // No error.
  OK = 0,

  // Network-specific errors
  #define NET_ERROR(label, value) ERR_ ## label = value,
  #include "net/base/net_error_list.h"
  #undef NET_ERROR

  // SIP-specific errors
  #define NO_SUCCESS // do not include successful cases
  #define SIP_STATUS(label, code, reason) ERR_SIP_ ## label = -(1000 + code),
  #include "sippet/message/status_code_list.h"
  #undef SIP_STATUS
  #undef NO_SUCCESS

  // Q.850-specific errors
  #define Q850_CAUSE(label, code) ERR_HANGUP_ ## label = -(1700 + code),
  #include "sippet/phone/q850.h"
  #undef Q850_CAUSE

  // The value of the first SIP-specific error code.
  ERR_SIP_BEGIN = ERR_SIP_TRYING,

  // The value of the first Q.850-specific error code.
  ERR_HANGUP_BEGIN = ERR_HANGUP_NOT_DEFINED,
};

// Returns a textual representation of the error code for logging purposes.
std::string ErrorToString(int error);

// Same as above, but leaves off the leading scope.
std::string ErrorToShortString(int error);

// A convenient function to translate SIP status codes to completion status.
int StatusCodeToCompletionStatus(int status_code);

// Returns true if |error| is a network error code.
bool IsNetError(int error);

// Returns true if |error| is a SIP-specific error code.
bool IsStatusCodeError(int error);

// Returns true if |error| is a Q.850-specific error code.
bool IsHangupCause(int error);

} // namespace phone

#endif // SIPPET_PHONE_COMPLETION_STATUS_H_
