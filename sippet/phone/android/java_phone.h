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
  jboolean Register(JNIEnv* env, jobject jcaller);
  jboolean Unregister(JNIEnv* env, jobject jcaller);
  jboolean UnregisterAll(JNIEnv* env, jobject jcaller);
  jlong MakeCall(JNIEnv* env, jobject jcaller,
                 jstring target);
  void HangUpAll(JNIEnv* env, jobject jcaller);
  void Finalize(JNIEnv* env, jobject jcaller);

  static bool RegisterBindings(JNIEnv* env);

  // Phone::Delegate implementation
  void OnNetworkError(int error_code) override;
  void OnRegisterCompleted(int status_code,
      const std::string& status_text) override;
  void OnRefreshError(int status_code,
      const std::string& status_text) override;
  void OnUnregisterCompleted(int status_code,
      const std::string& status_text) override;
  void OnIncomingCall(const scoped_refptr<Call>& call) override;

 private:
  scoped_refptr<Phone> phone_instance_;
  base::android::ScopedJavaGlobalRef<jobject> java_phone_;
};

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_PHONE_H_
