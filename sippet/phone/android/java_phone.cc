// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_phone.h"

#include <jni.h>

#include "sippet/phone/android/java_call.h"
#include "sippet/phone/android/java_settings.h"
#include "sippet/phone/android/run_completion_callback.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Phone_jni.h"

#include "sippet/phone/android/jni_onload.h"

namespace sippet {
namespace phone {
namespace android {

static void InitApplicationContext(JNIEnv* env, const JavaParamRef<jclass>&
    jcaller, const JavaParamRef<jobject>& context) {
  sippet::android::InitApplicationContext(env, context);
}

static jlong Create(JNIEnv* env, const JavaParamRef<jobject>& jcaller) {
  return reinterpret_cast<jlong>(new JavaPhone(env, jcaller));
}

JavaPhone::JavaPhone(JNIEnv *env, jobject obj) :
    phone_instance_(Phone::Create(this)),
    java_phone_(env, obj) {
  DCHECK(!java_phone_.is_null());
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

void JavaPhone::Register(JNIEnv* env, jobject jcaller,
                         jobject jcallback) {
  phone_instance_->Register(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

void JavaPhone::StartRefreshRegister(JNIEnv* env, jobject jcaller,
                                     jobject jcallback) {
  phone_instance_->StartRefreshRegister(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

void JavaPhone::StopRefreshRegister(JNIEnv* env, jobject jcaller) {
  phone_instance_->StopRefreshRegister();
}

void JavaPhone::Unregister(JNIEnv* env, jobject jcaller,
                           jobject jcallback) {
  phone_instance_->Unregister(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

void JavaPhone::UnregisterAll(JNIEnv* env, jobject jcaller,
                              jobject jcallback) {
  return phone_instance_->UnregisterAll(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

jlong JavaPhone::MakeCall(JNIEnv* env, jobject jcaller,
                          jstring target, jobject jcallback) {
  scoped_refptr<Call> call =
      phone_instance_->MakeCall(
          base::android::ConvertJavaStringToUTF8(env, target),
          base::Bind(&RunCompletionCallback,
              base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
  return reinterpret_cast<jlong>(new JavaCall(phone_instance_, call));
}

void JavaPhone::Finalize(JNIEnv* env, jobject jcaller) {
  delete this;
}

void JavaPhone::OnIncomingCall(const scoped_refptr<Call>& call) {
  JNIEnv* env = base::android::AttachCurrentThread();
  JavaCall *java_call = new JavaCall(phone_instance_, call);
  DCHECK(!java_phone_.is_null());
  Java_Phone_runOnIncomingCall(env,
      java_phone_.obj(),
      reinterpret_cast<jlong>(java_call));
}

// static
bool JavaPhone::RegisterBindings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace android
}  // namespace phone
}  // namespace sippet
