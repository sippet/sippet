// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_PHONE_H_
#define SIPPET_PHONE_ANDROID_PHONE_H_

#include <jni.h>

#include "sippet/phone/phone.h"

namespace sippet {
namespace phone {
namespace android {

// A Java Phone implementation.
class JavaPhone {
 public:
  JavaPhone(scoped_refptr<Phone> phone_instance);
  virtual ~JavaPhone();

  // Called from java.
  jboolean Init(JNIEnv* env, jobject jcaller,
                jobject settings);
  jlong GetState(JNIEnv* env, jobject jcaller);
  jboolean Register(JNIEnv* env, jobject jcaller);
  jboolean Unregister(JNIEnv* env, jobject jcaller);
  jboolean UnregisterAll(JNIEnv* env, jobject jcaller);
  jlong MakeCall(JNIEnv* env, jobject jcaller,
                 jstring target);
  jboolean HangUpAll(JNIEnv* env, jobject jcaller);
  void Finalize(JNIEnv* env, jobject jcaller);

  static bool RegisterBindings(JNIEnv* env);

 private:
  scoped_refptr<Phone> phone_instance_;
};

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_PHONE_H_
