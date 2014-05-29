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
        'sippet_test_support',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        '../net/test/run_all_unittests.cc',
        'message/message_unittest.cc',
        'message/headers_unittest.cc',
        'message/parser_unittest.cc',
        'uri/uri_unittest.cc',
        'transport/end_point_unittest.cc',
        'transport/network_layer_unittest.cc',
        'transport/glue/framed_write_stream_socket_unittest.cc',
        'transport/glue/sequenced_write_stream_socket_unittest.cc',
      ],
    },  # target sippet_unittest
    {
      'target_name': 'sippet_test_support',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/net/net.gyp:net_test_support',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
        'sippet.gyp:sippet',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/net/net.gyp:net_test_support',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gmock.gyp:gmock',
      ],
      'sources': [
        'transport/glue/transport_test_util.h',
        'transport/glue/transport_test_util.cc',
      ],
    },  # target sippet_test_support
  ],
}
