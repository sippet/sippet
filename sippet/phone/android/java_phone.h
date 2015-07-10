// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_PHONE_H_
#define SIPPET_PHONE_ANDROID_PHONE_H_

#include <jni.h>

#include "sippet/phone/phone_impl.h"

namespace sippet {
namespace phone {
namespace android {

// A Java Phone implementation.
class JavaPhone {
 public:
  JavaPhone();
  virtual ~JavaPhone();

  // Called from java.
  static void Initialize(JNIEnv* env, jclass jcaller);
  static jlong Create(JNIEnv* env, jobject jcaller);
  jboolean Init(JNIEnv* env, jobject jcaller,
                jlong instance,
                jobject settings);
  jlong GetState(JNIEnv* env, jobject jcaller,
                 jlong instance);
  void Register(JNIEnv* env, jobject jcaller,
                jlong instance);
  void Unregister(JNIEnv* env, jobject jcaller,
                  jlong instance);
  jlong MakeCall(JNIEnv* env, jobject jcaller,
                 jlong instance,
                 jstring target);
  void HangUpAll(JNIEnv* env, jobject jcaller,
                 jlong instance);
  void Finalize(JNIEnv* env, jobject jcaller,
                jlong instance);

  static bool RegisterBindings(JNIEnv* env);

 private:
  scoped_ref_ptr<sippet::phone::Phone> phone_instance_;
};

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_PHONE_H_
