// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_STATE_H_
#define SIPPET_PHONE_PHONE_STATE_H_

namespace sippet {
namespace phone {

// Phone state: the life cycle of the phone.
//
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.sippet.phone
enum PhoneState {
  PHONE_STATE_OFFLINE = 0,
  PHONE_STATE_READY = 1,
  PHONE_STATE_REGISTERING = 2,
  PHONE_STATE_REGISTERED = 3,
  PHONE_STATE_UNREGISTERING = 4
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_PHONE_STATE_H_
