// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_DIRECTION_H_
#define SIPPET_PHONE_CALL_DIRECTION_H_

namespace sippet {
namespace phone {

// Call direction: a call can be an incoming call or an outgoing call.
//
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.sippet.phone
enum CallDirection {
  CALL_DIRECTION_INCOMING = 0,
  CALL_DIRECTION_OUTGOING = 1
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_CALL_DIRECTION_H_
