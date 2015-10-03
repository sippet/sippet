// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_call.h"

#include <jni.h>

#include "sippet/phone/android/java_phone.h"
#include "sippet/phone/android/run_completion_callback.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Call_jni.h"

namespace sippet {
namespace phone {
namespace android {

JavaCall::JavaCall(const scoped_refptr<Phone>& phone_instance,
                   const scoped_refptr<Call>& call_instance) :
  phone_instance_(phone_instance),
  call_instance_(call_instance) {
}

JavaCall::~JavaCall() {
}

jlong JavaCall::GetDirection(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->direction());
}

jlong JavaCall::GetState(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->state());
}

ScopedJavaLocalRef<jstring> JavaCall::GetUri(JNIEnv* env, jobject jcaller) {
  return base::android::ConvertUTF8ToJavaString(env,
      call_instance_->uri().spec());
}

ScopedJavaLocalRef<jstring> JavaCall::GetName(JNIEnv* env, jobject jcaller) {
  return base::android::ConvertUTF8ToJavaString(env,
      call_instance_->name());
}

jlong JavaCall::GetCreationTime(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->creation_time().ToJavaTime());
}

jlong JavaCall::GetStartTime(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->start_time().ToJavaTime());
}

jlong JavaCall::GetEndTime(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->end_time().ToJavaTime());
}

void JavaCall::PickUp(JNIEnv* env, jobject jcaller, jobject jcallback) {
  call_instance_->PickUp(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

void JavaCall::Reject(JNIEnv* env, jobject jcaller) {
  call_instance_->Reject();
}

void JavaCall::HangUp(JNIEnv* env, jobject jcaller, jobject jcallback) {
  call_instance_->HangUp(base::Bind(&RunCompletionCallback,
      base::android::ScopedJavaGlobalRef<jobject>(env, jcallback)));
}

void JavaCall::SendDtmf(JNIEnv* env, jobject jcaller, jstring digits) {
  call_instance_->SendDtmf(
      base::android::ConvertJavaStringToUTF8(env, digits));
}

void JavaCall::Finalize(JNIEnv* env, jobject jcaller) {
  delete this;
}

// static
bool JavaCall::RegisterBindings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace android
}  // namespace phone
}  // namespace sippet
