# Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'webrtc_g729',
      'type': 'static_library',
      'dependencies': [
        'audio_encoder_interface',
        '<(DEPTH)/third_party/g729/g729.gyp:g729',
      ],
      'include_dirs': [
        '<(webrtc_root)',
        './include',
      ],
      'sources': [
        'audio_encoder_g729.cc',
        'include/audio_encoder_g729.h',
        'include/g729_interface.h',
        'g729_inst.h',
        'g729_interface.c',
      ],
    },
  ],
}
