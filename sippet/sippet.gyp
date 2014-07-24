# Copyright (c) 2013 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'sippet',
      'type': 'static_library',
      'dependencies': [
        'sippet_version',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party',
      ],
      'sources': [
        'base/casting.h',
        'base/format.h',
        'base/ilist.h',
        'base/ilist_node.h',
        'base/raw_ostream.cc',
        'base/raw_ostream.h',
        'base/sequences.h',
        'base/stl_extras.h',
        'base/string_extras.h',
        'base/tags.h',
        'base/type_traits.h',
        'base/user_agent_utils.h',
        'base/user_agent_utils.cc',
        'base/user_agent_utils_ios.mm',
        'base/version.h',
        'base/version.cc',
        'message/atom.h',
        'message/header.h',
        'message/header.cc',
        'message/headers.h',
        'message/headers/accept_encoding.h',
        'message/headers/accept.h',
        'message/headers/accept_language.h',
        'message/headers/alert_info.h',
        'message/headers/allow.h',
        'message/headers/authentication_info.h',
        'message/headers/authorization.h',
        'message/headers/call_id.h',
        'message/headers/call_info.h',
        'message/headers/contact.h',
        'message/headers/content_disposition.h',
        'message/headers/content_encoding.h',
        'message/headers/content_language.h',
        'message/headers/content_length.h',
        'message/headers/content_type.h',
        'message/headers/content_type.cc',
        'message/headers/cseq.h',
        'message/headers/date.h',
        'message/headers/error_info.h',
        'message/headers/expires.h',
        'message/headers/from.h',
        'message/headers/generic.h',
        'message/headers/in_reply_to.h',
        'message/headers/max_forwards.h',
        'message/headers/mime_version.h',
        'message/headers/min_expires.h',
        'message/headers/organization.h',
        'message/headers/priority.h',
        'message/headers/proxy_authenticate.h',
        'message/headers/proxy_authorization.h',
        'message/headers/proxy_require.h',
        'message/headers/record_route.h',
        'message/headers/reply_to.h',
        'message/headers/require.h',
        'message/headers/retry_after.h',
        'message/headers/route.h',
        'message/headers/server.h',
        'message/headers/subject.h',
        'message/headers/supported.h',
        'message/headers/timestamp.h',
        'message/headers/to.h',
        'message/headers/unsupported.h',
        'message/headers/user_agent.h',
        'message/headers/via.h',
        'message/headers/warning.h',
        'message/headers/www_authenticate.h',
        'message/headers/bits/auth_setters.h',
        'message/headers/bits/has_auth_params.h',
        'message/headers/bits/has_multiple.h',
        'message/headers/bits/has_parameters.h',
        'message/headers/bits/has_parameters.cc',
        'message/headers/bits/param_setters.h',
        'message/headers/bits/single_value.h',
        'message/header_list.h',
        'message/method_list.h',
        'message/protocol_list.h',
        'message/status_code_list.h',
        'message/message.h',
        'message/message.cc',
        'message/method.h',
        'message/method.cc',
        'message/parser/parser.cc',
        'message/parser/tokenizer.h',
        'message/protocol.h',
        'message/protocol.cc',
        'message/request.h',
        'message/request.cc',
        'message/response.h',
        'message/response.cc',
        'message/version.h',
        'message/status_code.h',
        'message/status_code.cc',
        'uri/uri.h',
        'uri/uri.cc',
        'uri/uri_canon.h',
        'uri/uri_canon.cc',
        'uri/uri_canon_internal.h',
        'uri/uri_canon_internal.cc',
        'uri/uri_parse.h',
        'uri/uri_parse.cc',
        'uri/uri_util.h',
        'uri/uri_util.cc',
        'transport/end_point.h',
        'transport/end_point.cc',
        'transport/network_layer.h',
        'transport/network_layer.cc',
        'transport/network_settings.h',
        'transport/network_settings.cc',
        'transport/branch_factory.h',
        'transport/branch_factory.cc',
        'transport/channel.h',
        'transport/transaction_factory.h',
        'transport/transaction_factory.cc',
        'transport/client_transaction_impl.h',
        'transport/client_transaction_impl.cc',
        'transport/server_transaction_impl.h',
        'transport/server_transaction_impl.cc',
        'transport/time_delta_provider.h',
        'transport/time_delta_factory.h',
        'transport/time_delta_factory.cc',
        'transport/chrome/sequenced_write_stream_socket.h',
        'transport/chrome/sequenced_write_stream_socket.cc',
        'transport/chrome/framed_write_stream_socket.h',
        'transport/chrome/framed_write_stream_socket.cc',
        'transport/chrome/chrome_channel_factory.h',
        'transport/chrome/chrome_channel_factory.cc',
        'transport/chrome/chrome_stream_channel.h',
        'transport/chrome/chrome_stream_channel.cc',
        'ua/user_agent.h',
        'ua/user_agent.cc',
        'ua/dialog.h',
        'ua/dialog.cc',
        'ua/auth.h',
        'ua/auth.cc',
        'ua/auth_cache.h',
        'ua/auth_cache.cc',
        'ua/auth_handler.h',
        'ua/auth_handler.cc',
        'ua/auth_handler_digest.h',
        'ua/auth_handler_digest.cc',
        'ua/auth_handler_factory.h',
        'ua/auth_handler_factory.cc',
        'ua/auth_controller.h',
        'ua/auth_controller.cc',
      ],
      'conditions': [
        ['OS == "ios"', {
          # iOS has different user-agent construction utilities.
          'sources!': [
            'user_agent_util.cc',
          ],
        }],
      ],
    },  # target sippet
    {
      'target_name': 'sippet_version',
      'type': 'none',
      'actions': [
        {
          'action_name': 'sippet_version',
          'inputs': [
            '<(script)',
            '<(lastchange)',
            '<(template)',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/sippet_version.h',
          ],
          'action': ['python',
                     '<(script)',
                     '-f', '<(lastchange)',
                     '<(template)',
                     '<@(_outputs)',
                   ],
          'variables': {
            'script': '<(DEPTH)/chrome/tools/build/version.py',
            'lastchange': '<(DEPTH)/build/util/LASTCHANGE',
            'template': 'build/sippet_version.h.in',
          },
        },
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)',
        ],
      },
      # Dependents may rely on files generated by this target or one of its
      # own hard dependencies.
      'hard_dependency': 1,
    },
  ],
}
