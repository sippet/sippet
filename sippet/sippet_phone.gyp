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
            'phone/android/java/src/org/sippet/phone/Phone.java',
            'phone/android/java/src/org/sippet/phone/Call.java',
          ],
          'variables': {
            'jni_gen_package': 'sippet_phone',
          },
          'includes': [ '../build/jni_generator.gypi' ],
        },
        {
          'target_name': 'sippet_phone_java',
          'type': 'none',
          'dependencies': [
            '../base/base.gyp:base_java',
            'sippet_phone_jni',
            'sippet_phone_java_phone_state',
          ],
          'variables': {
            'java_in_dir': '../sippet/phone/android/java',
          },
          'includes': [ '../build/java.gypi' ],
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
          'target_name': 'sippet_phone_jni',
          'type': 'static_library',
          'sources': [
            'phone/android/java_phone.h',
            'phone/android/java_phone.cc',
            'phone/android/java_call.h',
            'phone/android/java_call.cc',
          ],
          'dependencies': [
            'sippet_phone_jni_headers',
          ],
          'include_dirs': [
            '<(DEPTH)',
            '<(SHARED_INTERMEDIATE_DIR)'
          ],
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
        '<(DEPTH)/third_party/libjingle/libjingle.gyp:*',
        '<(DEPTH)/third_party/webrtc/webrtc.gyp:*',
        '<(DEPTH)/third_party/re2/re2.gyp:*',
      ],
      'include_dirs': [
        '<(DEPTH)/third_party/webrtc/overrides',
        '<(DEPTH)/third_party/libjingle/overrides',
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
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
        'phone/phone_impl.h',
        'phone/phone_impl.cc',
        'phone/call.h',
        'phone/call_impl.h',
        'phone/call_impl.cc',
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
        'phone/v8/js_function_call.h',
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
