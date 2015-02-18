# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
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
        'phone/phone_impl.h',
        'phone/phone_impl.cc',
        'phone/call.h',
        'phone/call_impl.h',
        'phone/call_impl.cc',
      ],
    },  # target sippet_phone
  ],
}
