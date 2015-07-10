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
            'jni_gen_package': 'sippet',
          },
          'includes': [ '../build/jni_generator.gypi' ],
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
        'phone/phone.h',
        'phone/phone.cc',
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
        'phone/phone_js_wrapper.h',
        'phone/phone_js_wrapper.cc',
        'phone/call_js_wrapper.h',
        'phone/call_js_wrapper.cc',
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
        'phone/phone_js_main.cc',
      ],
    },  # target sippet_phone_v8
  ],
}
