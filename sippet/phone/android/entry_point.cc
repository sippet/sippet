// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/bind.h"
#include "sippet/phone/android/jni_onload.h"

namespace {

JavaVM* g_jvm = nullptr;

bool RegisterJNI(JNIEnv* env) {
  return true;
}

bool Init() {
  return true;
}

}  // namespace

namespace sippet {
namespace android {

JavaVM* GetJVM() {
  DCHECK(g_jvm);
  return g_jvm;
}

}  // namespace android
}  // namespace sippet


// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  g_jvm = vm;

  std::vector<base::android::RegisterCallback> register_callbacks;
  register_callbacks.push_back(base::Bind(&RegisterJNI));

  std::vector<base::android::InitCallback> init_callbacks;
  init_callbacks.push_back(base::Bind(&Init));

  if (!sippet::android::OnJNIOnLoadRegisterJNI(vm, register_callbacks) ||
      !sippet::android::OnJNIOnLoadInit(init_callbacks)) {
    return -1;
  }

  return JNI_VERSION_1_6;
}
