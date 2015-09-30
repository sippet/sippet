// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_RUN_COMPLETION_CALLBACK_H_
#define SIPPET_PHONE_ANDROID_RUN_COMPLETION_CALLBACK_H_

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

namespace sippet {
namespace phone {
namespace android {

void RunCompletionCallback(
    const base::android::ScopedJavaGlobalRef<jobject>& callback,
    int error);

} // namespace android
} // namespace phone
} // namespace sippet

#endif  // SIPPET_PHONE_ANDROID_RUN_COMPLETION_CALLBACK_H_
