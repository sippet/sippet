// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/run_completion_callback.h"

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

using base::android::MethodID;
using base::android::CheckException;
using base::android::AttachCurrentThread;
using base::android::GetClass;
using base::android::ScopedJavaGlobalRef;
using base::android::ScopedJavaLocalRef;

namespace sippet {
namespace phone {
namespace android {

void RunCompletionCallback(
    const ScopedJavaGlobalRef<jobject>& callback,
    int error) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jclass> completion_callback_clazz =
      GetClass(env, "io/sippet/phone/CompletionCallback");
  jmethodID on_completed = MethodID::Get<MethodID::TYPE_INSTANCE>(env,
      completion_callback_clazz.obj(), "onCompleted", "(I)V");
  env->CallVoidMethod(callback.obj(), on_completed, error);
  CheckException(env);
}

}  // namespace android
}  // namespace phone
}  // namespace sippet
