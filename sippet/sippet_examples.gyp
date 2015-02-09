# Copyright (c) 2013 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'sippet_examples_common',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
        'sippet.gyp:sippet',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)',
          '<(DEPTH)/third_party',
        ],
      },
      'sources': [
        'examples/common/url_request_context_getter.h',
        'examples/common/url_request_context_getter.cc',
        'examples/common/static_password_handler.h',
        'examples/common/static_password_handler.cc',
        'examples/common/dump_ssl_cert_error.h',
        'examples/common/dump_ssl_cert_error.cc',
      ],
    },  # target sippet_examples_common
    {
      'target_name': 'sippet_examples_program_main',
      'type': 'static_library',
      'dependencies': [
        'sippet_examples_common',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)',
          '<(DEPTH)/third_party',
        ],
      },
      'sources': [
        'examples/program_main/program_main.h',
        'examples/program_main/program_main.cc',
      ],
    },  # target sippet_examples_program_main
    {
      'target_name': 'sippet_examples_login',
      'type': 'executable',
      'dependencies': [
        'sippet_examples_program_main',
      ],
      'sources': [
        'examples/login/login_main.cc',
      ],
    },  # target sippet_examples_login
    {
      'target_name': 'sippet_examples_call',
      'type': 'executable',
      'dependencies': [
        'sippet_examples_program_main',
        '<(DEPTH)/jingle/jingle.gyp:*',
        '<(DEPTH)/third_party/webrtc/webrtc.gyp:*',
        '<(DEPTH)/third_party/libjingle/libjingle.gyp:*',
        '<(DEPTH)/third_party/re2/re2.gyp:*',
      ],
      'sources': [
        'examples/call/call_main.cc',
      ],
    },  # target sippet_examples_call
  ],
}
