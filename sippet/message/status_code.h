// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_STATUS_CODE_H_
#define SIPPET_MESSAGE_STATUS_CODE_H_

#include <string>
#include "net/base/net_export.h"

namespace sippet {

enum StatusCode {

#define SIP_STATUS(label, code, reason) SIP_ ## label = code,
#include "sippet/message/status_code_list.h"
#undef SIP_STATUS

};

// Returns the corresponding SIP status description to use in the Reason-Phrase
// field in a SIP response for given |code|. It's based on the IANA Session
// Initiation Protocol (SIP) Parameters.
// http://www.iana.org/assignments/sip-parameters/sip-parameters.xhtml
//
// This function may not cover all codes defined in the IANA registry. It
// returns an empty string (or crash in debug build) for status codes which are
// not yet covered or just invalid. Please extend it when needed.
const char* GetReasonPhrase(StatusCode code);

// Returns a textual representation of the status code for logging purposes.
std::string StatusCodeToShortString(int status_code);

} // End of sippet namespace

#endif // SIPPET_MESSAGE_STATUS_CODE_H_
