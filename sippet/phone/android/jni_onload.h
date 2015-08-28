// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_JNI_ONLOAD_H_
#define SIPPET_PHONE_ANDROID_JNI_ONLOAD_H_

#include "base/android/base_jni_onload.h"

namespace sippet {
namespace android {

// Returns true if JNI registration succeeded.
bool OnJNIOnLoadRegisterJNI(JavaVM* vm,
    std::vector<base::android::RegisterCallback> callback);

// Returns true if initialization succeeded.
bool OnJNIOnLoadInit(std::vector<base::android::InitCallback> callback);

// Initialize the application context
void InitApplicationContext(JNIEnv* env, jobject context);

// Recover the JavaVM instance passed to JNI_OnLoad
JavaVM* GetJVM();

}  // namespace android
}  // namespace sippet

#endif  // SIPPET_PHONE_ANDROID_JNI_ONLOAD_H_
