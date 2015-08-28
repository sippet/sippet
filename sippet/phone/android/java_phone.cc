// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_phone.h"
#include "sippet/phone/android/java_call.h"
#include "sippet/phone/android/java_settings.h"

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Phone_jni.h"

#include "sippet/phone/android/jni_onload.h"

namespace sippet {
namespace phone {
namespace android {

static void InitApplicationContext(JNIEnv* env,
    jobject jcaller, jobject context) {
  sippet::android::InitApplicationContext(env, context);
}

static jlong Create(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(reinterpret_cast<uintptr_t>(
      new JavaPhone(env, jcaller)));
}

JavaPhone::JavaPhone(JNIEnv *env, jobject obj) :
    phone_instance_(Phone::Create(this)) {
  java_phone_.Reset(env, obj);
}

JavaPhone::~JavaPhone() {
}

jboolean JavaPhone::Init(JNIEnv* env, jobject jcaller,
                         jobject settings) {
  Settings s(ConvertJavaSettingsToSettings(env, settings));
  DCHECK(s.is_valid());
  return phone_instance_->Init(s)
      ? JNI_TRUE : JNI_FALSE;
}

jint JavaPhone::GetState(JNIEnv* env, jobject jcaller) {
  return static_cast<jint>(phone_instance_->state());
}

jboolean JavaPhone::Register(JNIEnv* env, jobject jcaller) {
  return phone_instance_->Register()
    ? JNI_TRUE : JNI_FALSE;
}

jboolean JavaPhone::Unregister(JNIEnv* env, jobject jcaller) {
  return phone_instance_->Unregister()
    ? JNI_TRUE : JNI_FALSE;
}

jboolean JavaPhone::UnregisterAll(JNIEnv* env, jobject jcaller) {
  return phone_instance_->UnregisterAll()
    ? JNI_TRUE : JNI_FALSE;
}

jlong JavaPhone::MakeCall(JNIEnv* env, jobject jcaller,
                          jstring target) {
  scoped_refptr<Call> call_instance =
      phone_instance_->MakeCall(
          base::android::ConvertJavaStringToUTF8(env, target));
  return static_cast<jlong>(reinterpret_cast<uintptr_t>(
      new JavaCall(phone_instance_, call_instance)));
}

void JavaPhone::HangUpAll(JNIEnv* env, jobject jcaller) {
  phone_instance_->HangUpAll();
}

void JavaPhone::Finalize(JNIEnv* env, jobject jcaller) {
  delete this;
}

void JavaPhone::OnNetworkError(int error_code) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Phone_runOnNetworkError(env, java_phone_.obj(), error_code);
}

void JavaPhone::OnRegisterCompleted(int status_code,
    const std::string& status_text) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Phone_runOnRegisterCompleted(env, java_phone_.obj(), status_code,
      base::android::ConvertUTF8ToJavaString(env, status_text).Release());
}

void JavaPhone::OnRefreshError(int status_code,
    const std::string& status_text) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Phone_runOnRefreshError(env, java_phone_.obj(), status_code,
      base::android::ConvertUTF8ToJavaString(env, status_text).Release());
}

void JavaPhone::OnUnregisterCompleted(int status_code,
    const std::string& status_text) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_Phone_runOnUnregisterCompleted(env, java_phone_.obj(), status_code,
      base::android::ConvertUTF8ToJavaString(env, status_text).Release());
}

void JavaPhone::OnIncomingCall(const scoped_refptr<Call>& call) {
  JNIEnv* env = base::android::AttachCurrentThread();
  JavaCall *java_call = new JavaCall(phone_instance_, call);
  Java_Phone_runOnIncomingCall(env, java_phone_.obj(),
      static_cast<jlong>(reinterpret_cast<uintptr_t>(java_call)));
}

// static
bool JavaPhone::RegisterBindings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

} // namespace android
} // namespace phone
} // namespace sippet
