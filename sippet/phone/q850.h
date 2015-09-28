// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file intentionally does not have header guards, it's included inside
// a macro to generate enum.
//
// This file contains the list of Q.850 cause values.
// https://www.itu.int/rec/T-REC-Q.850-199805-I/en

#ifndef Q850_CAUSE
#error "Q850_CAUSE should be defined before including this file"
#endif

// Cause not defined.
Q850_CAUSE(NOT_DEFINED, 0)

// Unallocated (unassigned) number. Same as 404, 485, 604.
Q850_CAUSE(UNALLOCATED, 1)

// No route to specified transmit network.
Q850_CAUSE(NO_ROUTE_TRANSIT_NET, 2)

// No route to destination. Same as 420.
Q850_CAUSE(NO_ROUTE_DESTINATION, 3)

// Misdialled trunk prefix (national use).
Q850_CAUSE(MISDIALLED_TRUNK_PREFIX, 5)

// Channel unacceptable.
Q850_CAUSE(CHANNEL_UNACCEPTABLE, 6)

// Call awarded and being delivered in an established channel.
Q850_CAUSE(CALL_AWARDED_DELIVERED, 7)

// Preemption.
Q850_CAUSE(PRE_EMPTED, 8)

// QoR: ported number.
Q850_CAUSE(NUMBER_PORTED_NOT_HERE, 14)

// Normal Clearing.
Q850_CAUSE(NORMAL_CLEARING, 16)

// User busy. Same as 486, 600.
Q850_CAUSE(USER_BUSY, 17)

// No user responding. Same as 408.
Q850_CAUSE(NO_USER_RESPONSE, 18)

// No answer from user (user alerted). Same as 480, 483.
Q850_CAUSE(NO_ANSWER, 19)

// Subscriber absent.
Q850_CAUSE(SUBSCRIBER_ABSENT, 20)

// Call Rejected. Same as 401, 403, 407, 603.
Q850_CAUSE(CALL_REJECTED, 21)

// Number changed. Same as 410.
Q850_CAUSE(NUMBER_CHANGED, 22)

// Redirected to new destination.
Q850_CAUSE(REDIRECTED_TO_NEW_DESTINATION, 23)

// Non-selected user clearing.
Q850_CAUSE(ANSWERED_ELSEWHERE, 26)

// Destination out of order. Same as 502.
Q850_CAUSE(DESTINATION_OUT_OF_ORDER, 27)

// Invalid number format. Same as 484.
Q850_CAUSE(INVALID_NUMBER_FORMAT, 28)

// Facility rejected. Same as 501.
Q850_CAUSE(FACILITY_REJECTED, 29)

// Response to STATUS ENQUIRY.
Q850_CAUSE(RESPONSE_TO_STATUS_ENQUIRY, 30)

// Normal, unspecified.
Q850_CAUSE(NORMAL_UNSPECIFIED, 31)

// No circuit/channel available
Q850_CAUSE(NORMAL_CIRCUIT_CONGESTION, 34)

// Network out of order. Same as 500.
Q850_CAUSE(NETWORK_OUT_OF_ORDER, 38)

// Temporary failure. Same sa 409.
Q850_CAUSE(NORMAL_TEMPORARY_FAILURE, 41)

// Switching equipment congestion. Same as 5xx.
Q850_CAUSE(SWITCH_CONGESTION, 42)

// Access information discarded.
Q850_CAUSE(ACCESS_INFO_DISCARDED, 43)

// Requested circuit/channel not available.
Q850_CAUSE(REQUESTED_CHAN_UNAVAIL, 44)

// Requested facility not subscribed.
Q850_CAUSE(FACILITY_NOT_SUBSCRIBED, 50)

// Outgoing call barred.
Q850_CAUSE(OUTGOING_CALL_BARRED, 52)

// Incoming call barred.
Q850_CAUSE(INCOMING_CALL_BARRED, 54)

// Bearer capability not authorized.
Q850_CAUSE(BEARER_CAP_NOTAUTH, 57)

// Bearer capability not presently available. Same as 488, 606.
Q850_CAUSE(BEARER_CAP_NOTAVAIL, 58)

// Bearer capability not implemented.
Q850_CAUSE(BEARER_CAP_NOTIMPL, 65)

// Channel type not implemented.
Q850_CAUSE(CHAN_NOT_IMPLEMENTED, 66)

// Requested facility not implemented.
Q850_CAUSE(FACILITY_NOT_IMPLEMENTED, 69)

// Invalid call reference value.
Q850_CAUSE(INVALID_CALL_REFERENCE, 81)

// Incompatible destination.
Q850_CAUSE(INCOMPATIBLE_DESTINATION, 88)

// Invalid message unspecified.
Q850_CAUSE(INVALID_MSG_UNSPECIFIED, 95)

// Mandatory information element is missing.
Q850_CAUSE(MANDATORY_IE_MISSING, 96)

// Message type non-existent or not implemented.
Q850_CAUSE(MESSAGE_TYPE_NONEXIST, 97)

// Message not compatible with call state or message type non-existent or not
// implemented.
Q850_CAUSE(WRONG_MESSAGE, 98)

// Information element nonexistent or not implemented.
Q850_CAUSE(IE_NONEXIST, 99)

// Invalid information element contents.
Q850_CAUSE(INVALID_IE_CONTENTS, 100)

// Message not compatible with call state.
Q850_CAUSE(WRONG_CALL_STATE, 101)

// Recover on timer expiry. Same as 504.
Q850_CAUSE(RECOVERY_ON_TIMER_EXPIRE, 102)

// Parameter non-existent or not implemented, passed on.
Q850_CAUSE(PARAMETER_NONEXIST, 103)

// Message with unrecognized parameter, discarded.
Q850_CAUSE(INVALID_PARAMETER, 110)

// Protocol error, unspecified.
Q850_CAUSE(PROTOCOL_ERROR, 111)

//  Interworking, unspecified. Same as 4xx, 505, 6xx.
Q850_CAUSE(INTERWORKING, 127)
