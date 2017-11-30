// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file intentionally does not have header guards, it's included inside
// a macro to generate enum.
//
// This file contains the list of SIP status codes. Taken from IANA Session
// Initiation Protocol (SIP) Parameters.
// http://www.iana.org/assignments/sip-parameters/sip-parameters.xhtml

#ifndef SIP_STATUS
#error "SIP_STATUS should be defined before including this file"
#endif

// Provisional 1xx
SIP_STATUS(TRYING, 100, "Trying")
SIP_STATUS(RINGING, 180, "Ringing")
SIP_STATUS(CALL_IS_BEING_FORWARDED, 181, "Call Is Being Forwarded")
SIP_STATUS(QUEUED, 182, "Queued")
SIP_STATUS(SESSION_PROGRESS, 183, "Session Progress")
SIP_STATUS(EARLY_DIALOG_TERMINATED, 199, "Early Dialog Terminated")

#ifndef NO_SUCCESS
// Successful 2xx
SIP_STATUS(OK, 200, "OK")
SIP_STATUS(ACCEPTED, 202, "Accepted")
SIP_STATUS(NO_NOTIFICATION, 204, "No Notification")
#endif

// Redirection 3xx
SIP_STATUS(MULTIPLE_CHOICES, 300, "Multiple Choices")
SIP_STATUS(MOVED_PERMANENTLY, 301, "Moved Permanently")
SIP_STATUS(MOVED_TEMPORARILY, 302, "Moved Temporarily")
SIP_STATUS(USE_PROXY, 305, "Use Proxy")
SIP_STATUS(ALTERNATIVE_SERVICE, 380, "Alternative Service")

// Request failure 4xx
SIP_STATUS(BAD_REQUEST, 400, "Bad Request")
SIP_STATUS(UNAUTHORIZED, 401, "Unauthorized")
SIP_STATUS(PAYMENT_REQUIRED, 402, "Payment Required")
SIP_STATUS(FORBIDDEN, 403, "Forbidden")
SIP_STATUS(NOT_FOUND, 404, "Not Found")
SIP_STATUS(METHOD_NOT_ALLOWED, 405, "Method Not Allowed")
SIP_STATUS(NOT_ACCEPTABLE, 406, "Not Acceptable")
SIP_STATUS(PROXY_AUTHENTICATION_REQUIRED, 407, "Proxy Authentication Required")
SIP_STATUS(REQUEST_TIMEOUT, 408, "Request Timeout")
SIP_STATUS(CONFLICT, 409, "Conflict")
SIP_STATUS(GONE, 410, "Gone")
SIP_STATUS(CONDITIONAL_REQUEST_FAILED, 412, "Conditional Request Failed")
SIP_STATUS(REQUEST_ENTITY_TOO_LARGE, 413, "Request Entity Too Large")
SIP_STATUS(REQUEST_URI_TOO_LONG, 414, "Request-URI Too Long")
SIP_STATUS(UNSUPPORTED_MEDIA_TYPE, 415, "Unsupported Media Type")
SIP_STATUS(UNSUPPORTED_URI_SCHEME, 416, "Unsupported URI Scheme")
SIP_STATUS(UNKNOWN_RESOURCE_PRIORITY, 417, "Unknown Resource-Priority")
SIP_STATUS(BAD_EXTENSION, 420, "Bad Extension")
SIP_STATUS(EXTENSION_REQUIRED, 421, "Extension Required")
SIP_STATUS(SESSION_INTERVAL_TOO_SMALL, 422, "Session Interval Too Small")
SIP_STATUS(INTERVAL_TOO_BRIEF, 423, "Interval Too Brief")
SIP_STATUS(BAD_LOCATION_INFORMATION, 424, "Bad Location Information")
SIP_STATUS(USE_IDENTITY_HEADER, 428, "Use Identity Header")
SIP_STATUS(PROVIDE_REFERRER_IDENTITY, 429, "Provide Referrer Identity")
SIP_STATUS(FLOW_FAILED, 430, "Flow Failed")
SIP_STATUS(ANONYMITY_DISALLOWED, 433, "Anonymity Disallowed")
SIP_STATUS(BAD_IDENTITY_INFO, 436, "Bad Identity-Info")
SIP_STATUS(UNSUPPORTED_CERTIFICATE, 437, "Unsupported Certificate")
SIP_STATUS(INVALID_IDENTITY_HEADER, 438, "Invalid Identity Header")
SIP_STATUS(FIRST_HOP_LACKS_OUTBOUND_SUPPORT, 439, "First Hop Lacks Outbound Support")
SIP_STATUS(MAX_BREADTH_EXCEEDED, 440, "Max-Breadth Exceeded")
SIP_STATUS(BAD_INFO_PACKAGE, 469, "Bad Info Package")
SIP_STATUS(CONSENT_NEEDED, 470, "Consent Needed")
SIP_STATUS(TEMPORARILY_UNAVAILABLE, 480, "Temporarily Unavailable")
SIP_STATUS(CALL_TRANSACTION_DOES_NOT_EXIST, 481, "Call/Transaction Does Not Exist")
SIP_STATUS(LOOP_DETECTED, 482, "Loop Detected")
SIP_STATUS(TOO_MANY_HOPS, 483, "Too Many Hops")
SIP_STATUS(ADDRESS_INCOMPLETE, 484, "Address Incomplete")
SIP_STATUS(AMBIGUOUS, 485, "Ambiguous")
SIP_STATUS(BUSY_HERE, 486, "Busy Here")
SIP_STATUS(REQUEST_TERMINATED, 487, "Request Terminated")
SIP_STATUS(NOT_ACCEPTABLE_HERE, 488, "Not Acceptable Here")
SIP_STATUS(BAD_EVENT, 489, "Bad Event")
SIP_STATUS(REQUEST_PENDING, 491, "Request Pending")
SIP_STATUS(UNDECIPHERABLE, 493, "Undecipherable")
SIP_STATUS(SECURITY_AGREEMENT_REQUIRED, 494, "Security Agreement Required")

// Server failure 5xx 
SIP_STATUS(SERVER_INTERNAL_ERROR, 500, "Server Internal Error")
SIP_STATUS(NOT_IMPLEMENTED, 501, "Not Implemented")
SIP_STATUS(BAD_GATEWAY, 502, "Bad Gateway")
SIP_STATUS(SERVICE_UNAVAILABLE, 503, "Service Unavailable")
SIP_STATUS(SERVER_TIMEOUT, 504, "Server Time-out")
SIP_STATUS(VERSION_NOT_SUPPORTED, 505, "Version Not Supported")
SIP_STATUS(MESSAGE_TOO_LARGE, 513, "Message Too Large")
SIP_STATUS(PRECONDITION_FAILURE, 580, "Precondition Failure")

// Global failure 6xx
SIP_STATUS(BUSY_EVERYWHERE, 600, "Busy Everywhere")
SIP_STATUS(DECLINE, 603, "Decline")
SIP_STATUS(DOES_NOT_EXIST_ANYWHERE, 604, "Does Not Exist Anywhere")
SIP_STATUS(NOT_ACCEPTABLE_EVERYWHERE, 606, "Not Acceptable")
