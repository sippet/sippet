// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_COMPLETION_STATUS_H_
#define SIPPET_PHONE_COMPLETION_STATUS_H_

#include <string>

#include "sippet/phone/completion_status_list.h"

namespace sippet {

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
