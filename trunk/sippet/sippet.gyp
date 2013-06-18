# Copyright (c) 2013, Guilherme Balena Versiani
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies, 
# either expressed or implied, of the FreeBSD Project.

{
  'includes': [
    '../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'sippet',
      'type': 'static_library',
      'dependencies': [
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
        'base/stl_extras.h',
        'base/string_extras.h',
        'base/type_traits.h',
        'message/header.h',
        'message/header.cc',
        'message/headers/accept.h',
        'message/headers/accept_encoding.h',
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
        'message/headers/cseq.h',
        'message/headers/date.h',
        'message/headers/error_info.h',
        'message/headers/expires.h',
        'message/headers/from.h',
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
        'message/headers/bits/has_multiple.h',
        'message/headers/bits/has_parameters.h',
        'message/headers/bits/single_value.h',
        'message/method.h',
        'message/method.cc',
        'message/message.h',
        'message/message.cc',
        'message/protocol.h',
        'message/protocol.cc',
        'message/parser/parser.cc',
        'transport/end_point.h',
        'transport/end_point.cc',
        'transport/network_layer.h',
        'transport/network_layer.cc',
        'transport/network_layer_delegate.h',
      ],
    },  # target sippet
  ],
}
