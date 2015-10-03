// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/jni_onload.h"

#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/android/base_jni_onload.h"
#include "base/android/base_jni_registrar.h"
#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/android/jni_utils.h"
#include "net/android/net_jni_registrar.h"
#include "url/android/url_jni_registrar.h"
#include "url/url_util.h"
#include "webrtc/modules/utility/interface/jvm_android.h"
#include "sippet/phone/android/jni_registrar.h"
#include "sippet/phone/phone.h"

namespace sippet {
namespace android {

namespace {

base::LazyInstance<scoped_ptr<base::MessageLoop>> g_java_message_loop =
    LAZY_INSTANCE_INITIALIZER;

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

void InitApplicationContext(JNIEnv* env, jobject context) {
  if (g_java_message_loop.Get() == nullptr) {
    base::android::ScopedJavaLocalRef<jobject> scoped_context(env, context);
    base::android::InitApplicationContext(env, scoped_context);

    base::i18n::InitializeICU();

    base::CommandLine::Init(0, nullptr);

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);

    // To view log output with IDs and timestamps use "adb logcat -v
    // threadtime".
    logging::SetLogItems(false,   // Process ID
                         false,   // Thread ID
                         false,   // Timestamp
                         false);  // Tick count
    logging::SetMinLogLevel(-10);

    g_java_message_loop.Get().reset(new base::MessageLoopForUI);
    base::MessageLoopForUI::current()->Start();

    url::Initialize();

    sippet::phone::Phone::Initialize();

    webrtc::JVM::Initialize(GetJVM(), context);
  }
}

}  // namespace android
}  // namespace sippet
