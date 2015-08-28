// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_ANDROID_JAVA_SETTINGS_H_
#define SIPPET_PHONE_ANDROID_JAVA_SETTINGS_H_

#include <jni.h>

#include "sippet/phone/settings.h"

namespace sippet {
namespace phone {
namespace android {

Settings ConvertJavaSettingsToSettings(JNIEnv* env, jobject settings);

}  // namespace android
}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_ANDROID_JAVA_SETTINGS_H_
