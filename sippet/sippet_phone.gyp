# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'conditions': [
    ['OS == "android"', {
      'targets': [
        {
          'target_name': 'sippet_phone_jni_headers',
          'type': 'none',
          'sources': [
            'phone/android/java/src/io/sippet/phone/Phone.java',
            'phone/android/java/src/io/sippet/phone/Call.java',
            'phone/android/java/src/io/sippet/phone/Settings.java',
            'phone/android/java/src/io/sippet/phone/IceServer.java',
            'phone/android/java/src/io/sippet/phone/CompletionCallback.java',
          ],
          'variables': {
            'jni_gen_package': 'sippet_phone',
          },
          'includes': [ '../build/jni_generator.gypi' ],
        },
        {
          'target_name': 'sippet_phone_java_phone_state',
          'type': 'none',
          'variables': {
            'source_file': 'phone/phone_state.h',
          },
          'includes': [ '../build/android/java_cpp_enum.gypi' ],
        },
        {
          'target_name': 'sippet_phone_java_call_state',
          'type': 'none',
          'variables': {
            'source_file': 'phone/call_state.h',
          },
          'includes': [ '../build/android/java_cpp_enum.gypi' ],
        },
        {
          'target_name': 'sippet_phone_java_call_direction',
          'type': 'none',
          'variables': {
            'source_file': 'phone/call_direction.h',
          },
          'includes': [ '../build/android/java_cpp_enum.gypi' ],
        },
        {
          'target_name': 'sippet_phone_java_completion_status',
          'type': 'none',
          'variables': {
            'source_file': 'phone/completion_status_list.h',
          },
          'includes': [ '../sippet/build/android/java_cpp_enum.gypi' ],
        },
        {
          'target_name': 'sippet_phone_jni',
          'type': 'shared_library',
          'sources': [
            'phone/android/java_phone.h',
            'phone/android/java_phone.cc',
            'phone/android/java_call.h',
            'phone/android/java_call.cc',
            'phone/android/java_settings.h',
            'phone/android/java_settings.cc',
            'phone/android/jni_onload.h',
            'phone/android/jni_onload.cc',
            'phone/android/jni_registrar.h',
            'phone/android/jni_registrar.cc',
            'phone/android/entry_point.cc',
          ],
          'dependencies': [
            'sippet_phone',
            'sippet_phone_jni_headers',
            '../base/base.gyp:base',
            '../base/base.gyp:base_static',
            '../net/net.gyp:net',
            '../net/net.gyp:net_resources',
            '../third_party/webrtc/webrtc.gyp:webrtc',
          ],
          'include_dirs': [
            '<(DEPTH)',
            '<(SHARED_INTERMEDIATE_DIR)'
          ],
        },
        {
          'target_name': 'sippet_phone_aar',
          'type': 'none',
          'dependencies': [
            'sippet_phone_jni',
            '../base/base.gyp:base_java',
            '../third_party/webrtc/modules/modules_java_chromium.gyp:audio_device_module_java',
            'sippet_phone_java_phone_state',
            'sippet_phone_java_call_state',
            'sippet_phone_java_call_direction',
          ],
          'variables': {
            'apk_name': "SippetPhone",
            'java_in_dir': '../sippet/phone/android/java',
            'native_lib_target': 'libsippet_phone_jni',
          },
          'includes': [ '../sippet/build/java_apk.gypi' ],
        },
      ],
    }],
  ],
  'targets': [
    {
      'target_name': 'sippet_phone',
      'type': 'static_library',
      'dependencies': [
        'sippet.gyp:sippet',
        '<(DEPTH)/jingle/jingle.gyp:jingle_glue',
        '<(DEPTH)/third_party/libjingle/libjingle.gyp:libjingle_webrtc',
        '<(DEPTH)/third_party/libjingle/libjingle.gyp:libpeerconnection',
        '<(DEPTH)/third_party/re2/re2.gyp:re2'
      ],
      'include_dirs': [
        '<(DEPTH)/third_party/webrtc/overrides',
        '<(DEPTH)/third_party/libjingle/overrides',
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'defines': ['WEBRTC_CHROMIUM_BUILD'],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)',
        ],
      },
      'sources': [
        'phone/settings.h',
        'phone/settings.cc',
        'phone/ice_server.h',
        'phone/ice_server.cc',
        'phone/phone.h',
        'phone/phone_state.h'
        'phone/phone_impl.h',
        'phone/phone_impl.cc',
        'phone/call.h',
        'phone/call_direction.h',
        'phone/call_state.h',
        'phone/call_impl.h',
        'phone/call_impl.cc',
        'phone/completion_status.h',
        'phone/completion_status.cc',
        'phone/completion_status_list.h',
        'phone/q850.h',
      ],
    },  # target sippet_phone
    {
      'target_name': 'sippet_phone_v8',
      'type': 'static_library',
      'dependencies': [
        'sippet_phone',
        '../gin/gin.gyp:gin',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'phone/v8/js_callback.h',
        'phone/v8/phone_js_wrapper.h',
        'phone/v8/phone_js_wrapper.cc',
        'phone/v8/call_js_wrapper.h',
        'phone/v8/call_js_wrapper.cc',
      ],
    },  # target sippet_phone_v8
    {
      'target_name': 'sippet_phone_v8_shell',
      'type': 'executable',
      'dependencies': [
        'sippet_phone_v8',
        '../gin/gin.gyp:gin',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'phone/v8/phone_js_main.cc',
      ],
    },  # target sippet_phone_v8
  ],
}
