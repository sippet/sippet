// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_PHONE_H_
#define SIPPET_PHONE_ANDROID_PHONE_H_

#include <jni.h>

#include "sippet/phone/phone.h"
#include "base/android/scoped_java_ref.h"

namespace sippet {
namespace phone {
namespace android {

// A Java Phone implementation.
class JavaPhone :
  public Phone::Delegate {
 public:
  JavaPhone(JNIEnv *env, jobject obj);
  virtual ~JavaPhone();

  // Called from java.
  jboolean Init(JNIEnv* env, jobject jcaller,
                jobject settings);
  jint GetState(JNIEnv* env, jobject jcaller);
  void Register(JNIEnv* env, jobject jcaller, jobject jcallback);
  void StartRefreshRegister(JNIEnv* env, jobject jcaller, jobject jcallback);
  void StopRefreshRegister(JNIEnv* env, jobject jcaller);
  void Unregister(JNIEnv* env, jobject jcaller, jobject jcallback);
  void UnregisterAll(JNIEnv* env, jobject jcaller, jobject jcallback);
  jlong MakeCall(JNIEnv* env, jobject jcaller,
                 jstring target, jobject jcallback);
  void Finalize(JNIEnv* env, jobject jcaller);

  static bool RegisterBindings(JNIEnv* env);

  // Phone::Delegate implementation
  void OnIncomingCall(const scoped_refptr<Call>& call) override;

 private:
  scoped_refptr<Phone> phone_instance_;
  base::android::ScopedJavaGlobalRef<jobject> java_phone_;
};

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_PHONE_H_
