// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/run_completion_callback.h"

#include "jni/CompletionCallback_jni.h"

namespace sippet {
namespace phone {
namespace android {

void RunCompletionCallback(
    const base::android::ScopedJavaGlobalRef<jobject>& callback,
    int error) {
  JNIEnv* env = base::android::AttachCurrentThread();
  if (!callback.is_null()) {
    Java_CompletionCallback_runOnCompleted(env, callback.obj(), error);
  }
}

}  // namespace android
}  // namespace phone
}  // namespace sippet
