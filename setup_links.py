#!/usr/bin/env python
# Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

"""Setup links to a Chromium checkout for Sippet.

Sippet standalone shares a lot of dependencies and build tools with Chromium.
To do this, many of the paths of a Chromium checkout is emulated by creating
symlinks to files and directories. This script handles the setup of symlinks to
achieve this.

It also handles cleanup of the legacy Subversion-based approach that was used
before Chrome switched over their master repo from Subversion to Git.
"""


import ctypes
import errno
import logging
import optparse
import os
import shelve
import shutil
import subprocess
import sys
import textwrap


DIRECTORIES = [
  'build',
  'buildtools',
  'google_apis',  # Needed by build/common.gypi.
  'base',
  'crypto',
  'chrome',
  'dbus',
  'gin',
  'sql',
  'net',
  'sdch',
  'v8',
  'ipc',
  'url',
  'testing',
  'third_party/binutils',
  'third_party/boringssl',
  'third_party/colorama',
  'third_party/drmemory',
  'third_party/expat',
  'third_party/gflags',
  'third_party/icu',
  'third_party/instrumented_libraries',
  'third_party/jemalloc',
  'third_party/jsoncpp',
  'third_party/libjpeg',
  'third_party/libjpeg_turbo',
  'third_party/libsrtp',
  'third_party/libudev',
  'third_party/libvpx',
  'third_party/libxml',
  'third_party/libyuv',
  'third_party/llvm-build',
  'third_party/modp_b64',
  'third_party/nss',
  'third_party/ocmock',
  'third_party/openmax_dl',
  'third_party/opus',
  'third_party/protobuf',
  'third_party/re2',
  'third_party/speex',
  'third_party/sqlite',
  'third_party/syzygy',
  'third_party/tcmalloc',
  'third_party/usrsctp',
  'third_party/yasm',
  'third_party/zlib',
  'tools/clang',
  'tools/generate_library_loader',
  'tools/gn',
  'tools/grit',
  'tools/gyp',
  'tools/memory',
  'tools/protoc_wrapper',
  'tools/python',
  'tools/swarming_client',
  'tools/valgrind',
  'tools/win',

  # --- Compatibility mode:
  # We've changed some parts of libjingle
  'third_party/libjingle/overrides/allocator_shim',
  'third_party/libjingle/source',
  'jingle/glue',
  'jingle/notifier',

  # --- We've also changed some parts of webrtc
  'third_party/webrtc/base',
  'third_party/webrtc/build/android',
  'third_party/webrtc/common_audio',
  'third_party/webrtc/common_video',
  'third_party/webrtc/examples',
  'third_party/webrtc/libjingle',
  'third_party/webrtc/modules/audio_coding/codecs/cng',
  'third_party/webrtc/modules/audio_coding/codecs/g711',
  'third_party/webrtc/modules/audio_coding/codecs/g722',
  'third_party/webrtc/modules/audio_coding/codecs/ilbc',
  'third_party/webrtc/modules/audio_coding/codecs/isac',
  'third_party/webrtc/modules/audio_coding/codecs/mock',
  'third_party/webrtc/modules/audio_coding/codecs/opus',
  'third_party/webrtc/modules/audio_coding/codecs/pcm16b',
  'third_party/webrtc/modules/audio_coding/codecs/red',
  'third_party/webrtc/modules/audio_coding/codecs/tools',
  'third_party/webrtc/modules/audio_coding/neteq/interface',
  'third_party/webrtc/modules/audio_coding/neteq/mock',
  'third_party/webrtc/modules/audio_coding/neteq/test',
  'third_party/webrtc/modules/audio_coding/neteq/tools',
  'third_party/webrtc/modules/audio_coding/main/interface',
  'third_party/webrtc/modules/audio_coding/main/test',
  'third_party/webrtc/modules/audio_conference_mixer',
  'third_party/webrtc/modules/audio_device',
  'third_party/webrtc/modules/audio_processing',
  'third_party/webrtc/modules/bitrate_controller',
  'third_party/webrtc/modules/desktop_capture',
  'third_party/webrtc/modules/interface',
  'third_party/webrtc/modules/media_file',
  'third_party/webrtc/modules/pacing',
  'third_party/webrtc/modules/remote_bitrate_estimator',
  'third_party/webrtc/modules/rtp_rtcp',
  'third_party/webrtc/modules/utility',
  'third_party/webrtc/modules/video_capture',
  'third_party/webrtc/modules/video_coding',
  'third_party/webrtc/modules/video_processing',
  'third_party/webrtc/modules/video_render',
  'third_party/webrtc/overrides/webrtc/base',
  'third_party/webrtc/p2p/base',
  'third_party/webrtc/p2p/client',
  'third_party/webrtc/sound',
  'third_party/webrtc/system_wrappers',
  'third_party/webrtc/test',
  'third_party/webrtc/tools',
  'third_party/webrtc/video',
  'third_party/webrtc/video_engine',
  'third_party/webrtc/voice_engine',
]

from sync_chromium import get_target_os_list
if 'android' in get_target_os_list():
  DIRECTORIES += [
    'third_party/android_testrunner',
    'third_party/android_tools',
    'third_party/appurify-python',
    'third_party/ashmem',
    'third_party/jsr-305',
    'third_party/libevent',
    'third_party/requests',
    'tools/android',
    'tools/relocation_packer'
  ]

if 'mac' in get_target_os_list():
  DIRECTORIES += [
    'third_party/mach_override',
    'third_party/apple_apsl'
  ]

if 'linux' in get_target_os_list():
  DIRECTORIES += [
    'third_party/gold',
    'third_party/libevent',
    'tools/xdisplaycheck',
    'tools/generate_library_loader'
  ]

FILES = {
  '.gn': None,
  'tools/find_depot_tools.py': None,
  'third_party/BUILD.gn': None,

  # --- Compatibility mode:
  # We've changed some parts of libjingle
  'third_party/libjingle/overrides/init_webrtc.h': None,
  'third_party/libjingle/overrides/init_webrtc.cc': None,
  'third_party/libjingle/overrides/initialize_module.cc': None,
  'third_party/libjingle/overrides/talk/media/webrtc/webrtcexport.h': None,
  'third_party/libjingle/BUILD.gn': None,
  'third_party/libjingle/libjingle_common.gypi': None,
  'third_party/libjingle/libjingle_nacl.gyp': None,
  'third_party/libjingle/OWNERS': None,
  'third_party/libjingle/README.chromium': None,
  'jingle/BUILD.gn': None,
  'jingle/DEPS': None,
  'jingle/OWNERS': None,
  'jingle/jingle.gypi': None,
  'jingle/jingle_nacl.gyp': None,

  # --- We've also changed some parts of webrtc
  'third_party/webrtc/build/adb_shell.sh': None,
  'third_party/webrtc/build/apk_tests.gyp': None,
  'third_party/webrtc/build/apk_tests_noop.gyp': None,
  'third_party/webrtc/build/arm_neon.gypi': None,
  'third_party/webrtc/build/download_vs_toolchain.py': None,
  'third_party/webrtc/build/extra_gitignore.py': None,
  'third_party/webrtc/build/find_directx_sdk.py': None,
  'third_party/webrtc/build/gyp_webrtc': None,
  'third_party/webrtc/build/gyp_webrtc.py': None,
  'third_party/webrtc/build/isolate.gypi': None,
  'third_party/webrtc/build/merge_libs.gyp': None,
  'third_party/webrtc/build/merge_libs.py': None,
  'third_party/webrtc/build/merge_libs_voice.gyp': None,
  'third_party/webrtc/build/merge_voice_libs.gyp': None,
  'third_party/webrtc/build/no_op.cc': None,
  'third_party/webrtc/build/OWNERS': None,
  'third_party/webrtc/build/protoc.gypi': None,
  'third_party/webrtc/build/tsan_suppressions_webrtc.cc': None,
  'third_party/webrtc/build/version.py': None,
  'third_party/webrtc/build/webrtc.gni': None,
  'third_party/webrtc/modules/audio_coding/BUILD.gn': None,
  'third_party/webrtc/modules/audio_coding/OWNERS': None,
  'third_party/webrtc/modules/audio_coding/codecs/audio_decoder.cc': None,
  'third_party/webrtc/modules/audio_coding/codecs/audio_decoder.h': None,
  'third_party/webrtc/modules/audio_coding/codecs/audio_encoder.cc': None,
  'third_party/webrtc/modules/audio_coding/codecs/audio_encoder.h': None,
  'third_party/webrtc/modules/audio_coding/codecs/interfaces.gypi': None,
  'third_party/webrtc/modules/audio_coding/codecs/OWNERS': None,
  'third_party/webrtc/modules/audio_coding/main/OWNERS': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_amr.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_amr.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_amrwb.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_amrwb.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_cng.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_cng.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_codec_database.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_codec_database.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_common_defs.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_dtmf_playout.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_dtmf_playout.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7221.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7221c.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7221c.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7221.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g722.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g722.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7291.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_g7291.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_generic_codec.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_generic_codec.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_gsmfr.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_gsmfr.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_ilbc.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_ilbc.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_isac.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_isac.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_isac_macros.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_neteq_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_opus.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_opus.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_opus_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcm16b.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcm16b.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcma.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcma.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcmu.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_pcmu.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receiver.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receiver.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receiver_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receiver_unittest_oldapi.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receive_test.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receive_test.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receive_test_oldapi.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_receive_test_oldapi.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_red.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_red.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_resampler.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_resampler.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_send_test.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_send_test.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_send_test_oldapi.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_send_test_oldapi.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_speex.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/acm_speex.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/audio_coding_module.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/audio_coding_module_impl.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/audio_coding_module_impl.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/audio_coding_module_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/audio_coding_module_unittest_oldapi.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/call_statistics.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/call_statistics.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/call_statistics_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/initial_delay_manager.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/initial_delay_manager.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/initial_delay_manager_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/nack.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/nack.h': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/nack_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/main/acm2/OWNERS': None,
  'third_party/webrtc/modules/audio_coding/neteq/accelerate.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/accelerate.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_classifier.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_classifier.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_classifier_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_decoder_unittests.isolate': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_multi_vector.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_multi_vector.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_multi_vector_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_vector.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_vector.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/audio_vector_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/background_noise.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/background_noise.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/background_noise_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/buffer_level_filter.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/buffer_level_filter.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/buffer_level_filter_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/comfort_noise.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/comfort_noise.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/comfort_noise_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic_fax.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic_fax.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic_normal.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic_normal.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/decision_logic_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decoder_database.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/decoder_database.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/decoder_database_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/defines.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_manager.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_manager.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_manager_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_peak_detector.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_peak_detector.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/delay_peak_detector_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dsp_helper.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dsp_helper.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/dsp_helper_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_buffer.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_buffer.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_buffer_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_tone_generator.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_tone_generator.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/dtmf_tone_generator_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/expand.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/expand.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/expand_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/merge.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/merge.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/merge_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_external_decoder_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_impl.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_impl.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_impl_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_stereo_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_tests.gypi': None,
  'third_party/webrtc/modules/audio_coding/neteq/neteq_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/normal.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/normal.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/normal_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/OWNERS': None,
  'third_party/webrtc/modules/audio_coding/neteq/packet_buffer.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/packet_buffer.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/packet_buffer_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/packet.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/payload_splitter.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/payload_splitter.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/payload_splitter_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/post_decode_vad.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/post_decode_vad.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/post_decode_vad_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/preemptive_expand.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/preemptive_expand.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/random_vector.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/random_vector.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/random_vector_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/rtcp.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/rtcp.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/statistics_calculator.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/statistics_calculator.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/sync_buffer.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/sync_buffer.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/sync_buffer_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/timestamp_scaler.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/timestamp_scaler.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/timestamp_scaler_unittest.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/time_stretch.cc': None,
  'third_party/webrtc/modules/audio_coding/neteq/time_stretch.h': None,
  'third_party/webrtc/modules/audio_coding/neteq/time_stretch_unittest.cc': None,
  'third_party/webrtc/modules/module_common_types_unittest.cc': None,
  'third_party/webrtc/modules/modules_java_chromium.gyp': None,
  'third_party/webrtc/modules/modules_java.gyp': None,
  'third_party/webrtc/modules/modules_tests.isolate': None,
  'third_party/webrtc/modules/modules_unittests.isolate': None,
  'third_party/webrtc/modules/OWNERS': None,
  'third_party/webrtc/overrides/OWNERS': None,
  'third_party/webrtc/p2p/OWNERS': None,
  'third_party/webrtc/p2p/p2p_tests.gypi': None,
  'third_party/webrtc/BUILD.gn': None,
  'third_party/webrtc/LICENSE': None,
  'third_party/webrtc/LICENSE_THIRD_PARTY': None,
  'third_party/webrtc/OWNERS': None,
  'third_party/webrtc/PATENTS': None,
  'third_party/webrtc/PRESUBMIT.py': None,
  'third_party/webrtc/README.chromium': None,
  'third_party/webrtc/call.h': None,
  'third_party/webrtc/codereview.settings': None,
  'third_party/webrtc/common.gyp': None,
  'third_party/webrtc/common.h': None,
  'third_party/webrtc/common_types.h': None,
  'third_party/webrtc/config.cc': None,
  'third_party/webrtc/config.h': None,
  'third_party/webrtc/engine_configurations.h': None,
  'third_party/webrtc/experiments.h': None,
  'third_party/webrtc/frame_callback.h': None,
  'third_party/webrtc/rtc_unittests.isolate': None,
  'third_party/webrtc/supplement.gypi': None,
  'third_party/webrtc/transport.h': None,
  'third_party/webrtc/typedefs.h': None,
  'third_party/webrtc/video_decoder.h': None,
  'third_party/webrtc/video_encoder.h': None,
  'third_party/webrtc/video_engine_tests.isolate': None,
  'third_party/webrtc/video_frame.h': None,
  'third_party/webrtc/video_receive_stream.h': None,
  'third_party/webrtc/video_renderer.h': None,
  'third_party/webrtc/video_send_stream.h': None,
  'third_party/webrtc/webrtc_examples.gyp': None,
  'third_party/webrtc/webrtc_perf_tests.isolate': None,
  'third_party/webrtc/webrtc_tests.gypi': None,
}

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
CHROMIUM_CHECKOUT = os.path.join('chromium', 'src')
LINKS_DB = 'links'

# Version management to make future upgrades/downgrades easier to support.
SCHEMA_VERSION = 1


def query_yes_no(question, default=False):
  """Ask a yes/no question via raw_input() and return their answer.

  Modified from http://stackoverflow.com/a/3041990.
  """
  prompt = " [%s/%%s]: "
  prompt = prompt % ('Y' if default is True  else 'y')
  prompt = prompt % ('N' if default is False else 'n')

  if default is None:
    default = 'INVALID'

  while True:
    sys.stdout.write(question + prompt)
    choice = raw_input().lower()
    if choice == '' and default != 'INVALID':
      return default

    if 'yes'.startswith(choice):
      return True
    elif 'no'.startswith(choice):
      return False

    print "Please respond with 'yes' or 'no' (or 'y' or 'n')."


# Actions
class Action(object):
  def __init__(self, dangerous):
    self.dangerous = dangerous

  def announce(self, planning):
    """Log a description of this action.

    Args:
      planning - True iff we're in the planning stage, False if we're in the
                 doit stage.
    """
    pass

  def doit(self, links_db):
    """Execute the action, recording what we did to links_db, if necessary."""
    pass


class Remove(Action):
  def __init__(self, path, dangerous):
    super(Remove, self).__init__(dangerous)
    self._priority = 0
    self._path = path

  def announce(self, planning):
    log = logging.warn
    filesystem_type = 'file'
    if not self.dangerous:
      log = logging.info
      filesystem_type = 'link'
    if planning:
      log('Planning to remove %s: %s', filesystem_type, self._path)
    else:
      log('Removing %s: %s', filesystem_type, self._path)

  def doit(self, _links_db):
    os.remove(self._path)


class Rmtree(Action):
  def __init__(self, path):
    super(Rmtree, self).__init__(dangerous=True)
    self._priority = 0
    self._path = path

  def announce(self, planning):
    if planning:
      logging.warn('Planning to remove directory: %s', self._path)
    else:
      logging.warn('Removing directory: %s', self._path)

  def doit(self, _links_db):
    if sys.platform.startswith('win'):
      # shutil.rmtree() doesn't work on Windows if any of the directories are
      # read-only, which svn repositories are.
      subprocess.check_call(['rd', '/q', '/s', self._path], shell=True)
    else:
      shutil.rmtree(self._path)


class Makedirs(Action):
  def __init__(self, path):
    super(Makedirs, self).__init__(dangerous=False)
    self._priority = 1
    self._path = path

  def doit(self, _links_db):
    try:
      os.makedirs(self._path)
    except OSError as e:
      if e.errno != errno.EEXIST:
        raise


class Symlink(Action):
  def __init__(self, source_path, link_path):
    super(Symlink, self).__init__(dangerous=False)
    self._priority = 2
    self._source_path = source_path
    self._link_path = link_path

  def announce(self, planning):
    if planning:
      logging.info(
          'Planning to create link from %s to %s', self._link_path,
          self._source_path)
    else:
      logging.debug(
          'Linking from %s to %s', self._link_path, self._source_path)

  def doit(self, links_db):
    # Files not in the root directory need relative path calculation.
    # On Windows, use absolute paths instead since NTFS doesn't seem to support
    # relative paths for symlinks.
    if sys.platform.startswith('win'):
      source_path = os.path.abspath(self._source_path)
    else:
      if os.path.dirname(self._link_path) != self._link_path:
        source_path = os.path.relpath(self._source_path,
                                      os.path.dirname(self._link_path))

    os.symlink(source_path, os.path.abspath(self._link_path))
    links_db[self._source_path] = self._link_path


class LinkError(IOError):
  """Failed to create a link."""
  pass


# Handles symlink creation on the different platforms.
if sys.platform.startswith('win'):
  def symlink(source_path, link_path):
    flag = 1 if os.path.isdir(source_path) else 0
    if not ctypes.windll.kernel32.CreateSymbolicLinkW(
        unicode(link_path), unicode(source_path), flag):
      raise OSError('Failed to create symlink to %s. Notice that only NTFS '
                    'version 5.0 and up has all the needed APIs for '
                    'creating symlinks.' % source_path)
  os.symlink = symlink


class SippetLinkSetup():
  def __init__(self, links_db, force=False, dry_run=False, prompt=False):
    self._force = force
    self._dry_run = dry_run
    self._prompt = prompt
    self._links_db = links_db

  def CreateLinks(self, on_bot):
    logging.debug('CreateLinks')
    # First, make a plan of action
    actions = []

    for source_path, link_path in FILES.iteritems():
      actions += self._ActionForPath(
          source_path, link_path, check_fn=os.path.isfile, check_msg='files')
    for source_dir in DIRECTORIES:
      actions += self._ActionForPath(
          source_dir, None, check_fn=os.path.isdir,
          check_msg='directories')

    if not on_bot and self._force:
      # When making the manual switch from legacy SVN checkouts to the new
      # Git-based Chromium DEPS, the .gclient_entries file that contains cached
      # URLs for all DEPS entries must be removed to avoid future sync problems.
      entries_file = os.path.join(os.path.dirname(ROOT_DIR), '.gclient_entries')
      if os.path.exists(entries_file):
        actions.append(Remove(entries_file, dangerous=True))

    actions.sort()

    if self._dry_run:
      for action in actions:
        action.announce(planning=True)
      logging.info('Not doing anything because dry-run was specified.')
      sys.exit(0)

    if any(a.dangerous for a in actions):
      logging.warn('Dangerous actions:')
      for action in (a for a in actions if a.dangerous):
        action.announce(planning=True)
      print

      if not self._force:
        logging.error(textwrap.dedent("""\
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                             A C T I O N     R E Q U I R E D
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        Because chromium/src is transitioning to Git (from SVN), we needed to
        change the way that the Sippet standalone checkout works. Instead of
        individually syncing subdirectories of Chromium in SVN, we're now
        syncing Chromium (and all of its DEPS, as defined by its own DEPS file),
        into the `chromium/src` directory.

        As such, all Chromium directories which are currently pulled by DEPS are
        now replaced with a symlink into the full Chromium checkout.

        To avoid disrupting developers, we've chosen to not delete your
        directories forcibly, in case you have some work in progress in one of
        them :).

        ACTION REQUIRED:
        Before running `gclient sync|runhooks` again, you must run:
        %s%s --force

        Which will replace all directories which now must be symlinks, after
        prompting with a summary of the work-to-be-done.
        """), 'python ' if sys.platform.startswith('win') else '', sys.argv[0])
        sys.exit(1)
      elif self._prompt:
        if not query_yes_no('Would you like to perform the above plan?'):
          sys.exit(1)

    for action in actions:
      action.announce(planning=False)
      action.doit(self._links_db)

    if not on_bot and self._force:
      logging.info('Completed!\n\nNow run `gclient sync|runhooks` again to '
                   'let the remaining hooks (that probably were interrupted) '
                   'execute.')

  def CleanupLinks(self):
    logging.debug('CleanupLinks')
    for source, link_path  in self._links_db.iteritems():
      if source == 'SCHEMA_VERSION':
        continue
      if os.path.islink(link_path) or sys.platform.startswith('win'):
        # os.path.islink() always returns false on Windows
        # See http://bugs.python.org/issue13143.
        logging.debug('Removing link to %s at %s', source, link_path)
        if not self._dry_run:
          if os.path.exists(link_path):
            if sys.platform.startswith('win') and os.path.isdir(link_path):
              subprocess.check_call(['rmdir', '/q', link_path], shell=True)
            else:
              os.remove(link_path)
          del self._links_db[source]

  @staticmethod
  def _ActionForPath(source_path, link_path=None, check_fn=None,
                     check_msg=None):
    """Create zero or more Actions to link to a file or directory.

    This will be a symlink on POSIX platforms. On Windows this requires
    that NTFS is version 5.0 or higher (Vista or newer).

    Args:
      source_path: Path relative to the Chromium checkout root.
        For readability, the path may contain slashes, which will
        automatically be converted to the right path delimiter on Windows.
      link_path: The location for the link to create. If omitted it will be the
        same path as source_path.
      check_fn: A function returning true if the type of filesystem object is
        correct for the attempted call. Otherwise an error message with
        check_msg will be printed.
      check_msg: String used to inform the user of an invalid attempt to create
        a file.
    Returns:
      A list of Action objects.
    """
    def fix_separators(path):
      if sys.platform.startswith('win'):
        return path.replace(os.altsep, os.sep)
      else:
        return path

    assert check_fn
    assert check_msg
    link_path = link_path or source_path
    link_path = fix_separators(link_path)

    source_path = fix_separators(source_path)
    source_path = os.path.join(CHROMIUM_CHECKOUT, source_path)
    if os.path.exists(source_path) and not check_fn:
      raise LinkError('_LinkChromiumPath can only be used to link to %s: '
                      'Tried to link to: %s' % (check_msg, source_path))

    if not os.path.exists(source_path):
      logging.debug('Silently ignoring missing source: %s. This is to avoid '
                    'errors on platform-specific dependencies.', source_path)
      return []

    actions = []

    if os.path.exists(link_path) or os.path.islink(link_path):
      if os.path.islink(link_path):
        actions.append(Remove(link_path, dangerous=False))
      elif os.path.isfile(link_path):
        actions.append(Remove(link_path, dangerous=True))
      elif os.path.isdir(link_path):
        actions.append(Rmtree(link_path))
      else:
        raise LinkError('Don\'t know how to plan: %s' % link_path)

    # Create parent directories to the target link if needed.
    target_parent_dirs = os.path.dirname(link_path)
    if (target_parent_dirs and
        target_parent_dirs != link_path and
        not os.path.exists(target_parent_dirs)):
      actions.append(Makedirs(target_parent_dirs))

    actions.append(Symlink(source_path, link_path))

    return actions

def _initialize_database(filename):
  links_database = shelve.open(filename)

  # Wipe the database if this version of the script ends up looking at a
  # newer (future) version of the links db, just to be sure.
  version = links_database.get('SCHEMA_VERSION')
  if version and version != SCHEMA_VERSION:
    logging.info('Found database with schema version %s while this script only '
                 'supports %s. Wiping previous database contents.', version,
                 SCHEMA_VERSION)
    links_database.clear()
  links_database['SCHEMA_VERSION'] = SCHEMA_VERSION
  return links_database


def main():
  on_bot = os.environ.get('CHROME_HEADLESS') == '1'

  parser = optparse.OptionParser()
  parser.add_option('-d', '--dry-run', action='store_true', default=False,
                    help='Print what would be done, but don\'t perform any '
                         'operations. This will automatically set logging to '
                         'verbose.')
  parser.add_option('-c', '--clean-only', action='store_true', default=False,
                    help='Only clean previously created links, don\'t create '
                         'new ones. This will automatically set logging to '
                         'verbose.')
  parser.add_option('-f', '--force', action='store_true', default=on_bot,
                    help='Force link creation. CAUTION: This deletes existing '
                         'folders and files in the locations where links are '
                         'about to be created.')
  parser.add_option('-n', '--no-prompt', action='store_false', dest='prompt',
                    default=(not on_bot),
                    help='Prompt if we\'re planning to do a dangerous action')
  parser.add_option('-v', '--verbose', action='store_const',
                    const=logging.DEBUG, default=logging.INFO,
                    help='Print verbose output for debugging.')
  options, _ = parser.parse_args()

  if options.dry_run or options.force or options.clean_only:
    options.verbose = logging.DEBUG
  logging.basicConfig(format='%(message)s', level=options.verbose)

  # Work from the root directory of the checkout.
  script_dir = os.path.dirname(os.path.abspath(__file__))
  os.chdir(script_dir)

  if sys.platform.startswith('win'):
    def is_admin():
      try:
        return os.getuid() == 0
      except AttributeError:
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    if not is_admin():
      logging.error('On Windows, you now need to have administrator '
                    'privileges for the shell running %s (or '
                    '`gclient sync|runhooks`).\nPlease start another command '
                    'prompt as Administrator and try again.' % sys.argv[0])
      return 1

  if not os.path.exists(CHROMIUM_CHECKOUT):
    logging.error('Cannot find a Chromium checkout at %s. Did you run "gclient '
                  'sync" before running this script?', CHROMIUM_CHECKOUT)
    return 2

  links_database = _initialize_database(LINKS_DB)
  try:
    symlink_creator = SippetLinkSetup(links_database, options.force,
                                      options.dry_run, options.prompt)
    symlink_creator.CleanupLinks()
    if not options.clean_only:
      symlink_creator.CreateLinks(on_bot)
  except LinkError as e:
    print >> sys.stderr, e.message
    return 3
  finally:
    links_database.close()
  return 0


if __name__ == '__main__':
  sys.exit(main())