# Copyright (c) 2013 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'sippet_login_example',
      'type': 'executable',
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
      'sources': [
        'examples/login/login_main.cc',
        'examples/login/url_request_context_getter.h',
        'examples/login/url_request_context_getter.cc',
      ],
    },  # target sippet_login_example
  ],
}
