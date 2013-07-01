# Copyright (c) 2013 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'sippet_unittest',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/net/net.gyp:net_test_support',
        'sippet.gyp:sippet',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        '../net/base/run_all_unittests.cc',
        'message/message_unittest.cc',
        'message/headers_unittest.cc',
        'message/parser_unittest.cc',
        'transport/end_point_unittest.cc',
        'transport/framed_write_stream_socket_unittest.cc',
        'transport/sequenced_write_stream_socket_unittest.cc',
      ],
    },  # target sippet_unittest
  ],
}
