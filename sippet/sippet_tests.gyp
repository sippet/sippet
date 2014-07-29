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
        'sippet.gyp:sippet',
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
        'transport/chrome/framed_write_stream_socket_unittest.cc',
        'transport/chrome/sequenced_write_stream_socket_unittest.cc',
        'ua/auth_controller_unittest.cc',
        'ua/auth_handler_digest_unittest.cc',
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
        'transport/chrome/transport_test_util.h',
        'transport/chrome/transport_test_util.cc',
        'ua/auth_handler_mock.h',
        'ua/auth_handler_mock.cc',
      ],
    },  # target sippet_test_support
    {
      'target_name': 'sippet_standalone_test_server',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/third_party/pjsip/pjsip.gyp:*',
        'sippet.gyp:sippet',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'test/standalone_test_server/standalone_test_server.h',
        'test/standalone_test_server/standalone_test_server.cc',
      ],
    },  # target sippet_test_support
    {
      'target_name': 'sippet_standalone_test_server_unittest',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        'sippet_standalone_test_server',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'test/standalone_test_server/standalone_test_server_unittest.cc',
      ],
    },  # target sippet_standalone_test_server_unittest
    {
      'target_name': 'sippet_standalone_test_server_main',
      'type': 'executable',
      'dependencies': [
        'sippet_standalone_test_server',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'test/standalone_test_server/main.cc',
      ],
    },  # target sippet_standalone_test_server_main
  ],
}
