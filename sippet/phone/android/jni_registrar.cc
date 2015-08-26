// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "sippet/phone/android/java_phone.h"
#include "sippet/phone/android/java_call.h"

namespace sippet {
namespace android {

static const base::android::RegistrationMethod kPhoneRegisteredMethods[] = {
    {"JavaPhone", sippet::phone::android::JavaPhone::RegisterBindings},
    {"JavaCall", sippet::phone::android::JavaCall::RegisterBindings},
};

bool RegisterPhoneJNI(JNIEnv* env) {
  return RegisterNativeMethods(
      env, kPhoneRegisteredMethods, arraysize(kPhoneRegisteredMethods));
}

}  // namespace android
}  // namespace sippet
