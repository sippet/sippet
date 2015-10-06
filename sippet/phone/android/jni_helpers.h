// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_JNI_HELPERS_H_
#define SIPPET_PHONE_ANDROID_JNI_HELPERS_H_

#include <jni.h>

namespace sippet {
namespace android {

jint InitGlobalJniVariables(JavaVM *jvm);

JavaVM *GetJVM();

}  // namespace android
}  // namespace sippet

#endif  // SIPPET_PHONE_ANDROID_JNI_HELPERS_H_
