// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/jni_onload.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/i18n/icu_util.h"
#include "base/android/base_jni_onload.h"
#include "base/android/base_jni_registrar.h"
#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/android/jni_utils.h"
#include "net/android/net_jni_registrar.h"
#include "url/android/url_jni_registrar.h"
#include "url/url_util.h"
#include "sippet/phone/android/jni_registrar.h"

namespace sippet {
namespace android {

namespace {

const base::android::RegistrationMethod kSippetRegisteredMethods[] = {
    {"BaseAndroid", base::android::RegisterJni},
    {"SippetPhone", RegisterPhoneJNI},
    {"NetAndroid", net::android::RegisterJni},
    {"UrlAndroid", url::android::RegisterJni},
};

bool RegisterJNI(JNIEnv* env) {
  return RegisterNativeMethods(
      env, kSippetRegisteredMethods, arraysize(kSippetRegisteredMethods));
}

bool Init() {
  base::i18n::InitializeICU();

  base::CommandLine::Init(0, nullptr);
  DCHECK(!base::MessageLoop::current());

  url::Initialize();
  return true;
}

}  // namespace


bool OnJNIOnLoadRegisterJNI(JavaVM* vm,
    std::vector<base::android::RegisterCallback> callbacks) {
  callbacks.push_back(base::Bind(&RegisterJNI));
  return base::android::OnJNIOnLoadRegisterJNI(vm, callbacks);
}

bool OnJNIOnLoadInit(std::vector<base::android::InitCallback> callbacks) {
  callbacks.push_back(base::Bind(&Init));
  return base::android::OnJNIOnLoadInit(callbacks);
}

}  // namespace android
}  // namespace sippet
