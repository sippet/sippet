// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/jni_helpers.h"

#include "base/logging.h"

namespace sippet {
namespace android {

namespace {
JavaVM* g_jvm = nullptr;
}  // namespace


jint InitGlobalJniVariables(JavaVM *jvm) {
  CHECK(!g_jvm) << "InitGlobalJniVariables!";
  g_jvm = jvm;
  CHECK(g_jvm) << "InitGlobalJniVariables handed NULL?";

  return JNI_VERSION_1_6;
}

JavaVM *GetJVM() {
  CHECK(g_jvm) << "JNI_OnLoad failed to run?";
  return g_jvm;
}

}  // namespace android
}  // namespace sippet
