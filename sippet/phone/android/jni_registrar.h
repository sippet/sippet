// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_JNI_REGISTRAR_H_
#define SIPPET_PHONE_ANDROID_JNI_REGISTRAR_H_

#include <jni.h>

namespace sippet {
namespace android {

// Register all JNI bindings necessary for sippet phone.
bool RegisterPhoneJNI(JNIEnv* env);

} // namespace android
} // namespace sippet

#endif  // SIPPET_PHONE_ANDROID_JNI_REGISTRAR_H_
