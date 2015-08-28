// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/android/java_settings.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/Settings_jni.h"

using base::android::ConvertJavaStringToUTF8;

namespace sippet {
namespace phone {
namespace android {

Settings ConvertJavaSettingsToSettings(JNIEnv* env, jobject settings) {
  Settings result;

  ScopedJavaLocalRef<jstring> j_uri =
      Java_Settings_getUri(env, settings);
  ScopedJavaLocalRef<jstring> j_user_agent =
      Java_Settings_getUserAgent(env, settings);
  ScopedJavaLocalRef<jstring> j_authorization_user =
      Java_Settings_getAuthorizationUser(env, settings);
  ScopedJavaLocalRef<jstring> j_display_name =
      Java_Settings_getDisplayName(env, settings);
  ScopedJavaLocalRef<jstring> j_password =
      Java_Settings_getPassword(env, settings);
  ScopedJavaLocalRef<jstring> j_registrar_server =
      Java_Settings_getRegistrarServer(env, settings);

  result.set_disable_encryption(
      Java_Settings_getDisableEncryption(env, settings));
  result.set_disable_sctp_data_channels(
      Java_Settings_getDisableSctpDataChannels(env, settings));
  result.set_register_expires(
      Java_Settings_getRegisterExpires(env, settings));

  if (!j_uri.is_null()) {
    result.set_uri(GURL(ConvertJavaStringToUTF8(j_uri)));
  }
  if (!j_user_agent.is_null()) {
    result.set_user_agent(ConvertJavaStringToUTF8(j_user_agent));
  }
  if (!j_authorization_user.is_null()) {
    result.set_authorization_user(
        ConvertJavaStringToUTF8(j_authorization_user));
  }
  if (!j_display_name.is_null()) {
    result.set_display_name(
        ConvertJavaStringToUTF8(j_display_name));
  }
  if (!j_password.is_null()) {
    result.set_password(
        ConvertJavaStringToUTF8(j_password));
  }
  if (!j_registrar_server.is_null()) {
    result.set_registrar_server(
        GURL(ConvertJavaStringToUTF8(j_registrar_server)));
  }
  DCHECK(result.is_valid());
  return result;
}

}  // namespace android
}  // namespace phone
}  // namespace sippet
