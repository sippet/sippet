// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_call.h"

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Call_jni.h"

namespace sippet {
namespace phone {
namespace android {

JavaCall::JavaCall(scoped_refptr<Call> call_instance) :
  call_instance_(call_instance) {
}

JavaCall::~JavaCall() {
}

jlong JavaCall::GetDirection(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->GetDirection());
}

jlong JavaCall::GetState(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(call_instance_->GetState());
}

jstring JavaCall::GetUri(JNIEnv* env, jobject jcaller) {
  return ConvertUTF8ToJavaString(env,
      call_instance_->uri().spec()).Release();
}

jstring JavaCall::GetName(JNIEnv* env, jobject jcaller) {
  return ConvertUTF8ToJavaString(env,
      call_instance_->name()).Release();
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

jboolean JavaCall::PickUp(JNIEnv* env, jobject jcaller) {
  return call_instance_->PickUp() ? JNI_TRUE : JNI_FALSE;
}

jboolean JavaCall::Reject(JNIEnv* env, jobject jcaller) {
  return call_instance_->Reject() ? JNI_TRUE : JNI_FALSE;
}

jboolean JavaCall::HangUp(JNIEnv* env, jobject jcaller) {
  return call_instance_->HangUp() ? JNI_TRUE : JNI_FALSE;
}

void JavaCall::SendDtmf(JNIEnv* env, jobject jcaller, jstring digits) {
  call_instance_->SendDtmf(ConvertJavaStringToUTF8(env, digits));
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
