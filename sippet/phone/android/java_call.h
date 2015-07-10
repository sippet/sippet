// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_CALL_H_
#define SIPPET_PHONE_ANDROID_CALL_H_

#include <jni.h>

#include "sippet/phone/call.h"

namespace sippet {
namespace phone {
namespace android {

// A Java Call implementation.
class JavaCall {
 public:
  JavaCall(scoped_refptr<Call> call_instance);
  virtual ~JavaCall();

  // Called from java.
  jlong GetDirection(JNIEnv* env, jobject jcaller);
  jlong GetState(JNIEnv* env, jobject jcaller);
  jstring GetUri(JNIEnv* env, jobject jcaller);
  jstring GetName(JNIEnv* env, jobject jcaller);
  jlong GetCreationTime(JNIEnv* env, jobject jcaller);
  jlong GetStartTime(JNIEnv* env, jobject jcaller);
  jlong GetEndTime(JNIEnv* env, jobject jcaller);
  jboolean PickUp(JNIEnv* env, jobject jcaller);
  jboolean Reject(JNIEnv* env, jobject jcaller);
  jboolean HangUp(JNIEnv* env, jobject jcaller);
  void SendDtmf(JNIEnv* env, jobject jcaller, jstring digits);
  void Finalize(JNIEnv* env, jobject jcaller);

  static bool RegisterBindings(JNIEnv* env);

 private:
  scoped_refptr<Call> call_instance_;
};

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_CALL_H_
