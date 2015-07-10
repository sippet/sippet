// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_phone.h"

#include <jni.h>

#include <cstdint>
#include <cstddef>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Phone_jni.h"

namespace sippet {
namespace phone {
namespace android {

static void Initialize(JNIEnv* env, jclass jcaller) {
  // TODO: maybe initialize the WebRTC (passing JNIEnv) from here as well
  Phone::Initialize();
}

static jlong Create(JNIEnv* env, jobject jcaller) {
  scoped_refptr<Phone>& phone_instance = 
      Phone::Create(PhoneObserver *phone_observer);
}

JavaPhone::JavaPhone(const scoped_refptr<Phone>& phone_instance) :
  phone_instance_(phone_instance) {
}

JavaPhone::~JavaPhone() {
}

jboolean JavaPhone::Init(JNIEnv* env, jobject jcaller,
                         jobject settings) {
  return phone_instance_->Init(JavaSettingsToSettings(settings))
    ? JNI_TRUE : JNI_FALSE;
}

jlong JavaPhone::GetState(JNIEnv* env, jobject jcaller) {
  return static_cast<jlong>(phone_instance_->GetState());
}

void JavaPhone::Register(JNIEnv* env, jobject jcaller) {
  phone_instance_->Register();
}

void JavaPhone::Unregister(JNIEnv* env, jobject jcaller) {
  phone_instance_->Unregister();
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

// static
bool JavaPhone::RegisterBindings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

} // namespace android
} // namespace phone
} // namespace sippet
