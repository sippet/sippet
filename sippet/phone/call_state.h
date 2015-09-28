// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_STATE_H_
#define SIPPET_PHONE_CALL_STATE_H_

namespace sippet {
namespace phone {

// Call state: the life cycle of the call.
//
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.sippet.phone
enum CallState {
  CALL_STATE_CALLING = 0,
  CALL_STATE_RINGING = 1,
  CALL_STATE_ESTABLISHED = 2,
  CALL_STATE_TERMINATED = 3
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_CALL_STATE_H_
