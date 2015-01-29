# Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'audio_device',
      'type': 'static_library',
      'dependencies': [
        'webrtc_utility',
        '<(webrtc_root)/common_audio/common_audio.gyp:common_audio',
        '<(webrtc_root)/system_wrappers/source/system_wrappers.gyp:system_wrappers',
      ],
      'include_dirs': [
        '.',
        '../interface',
        'include',
        'dummy', # dummy audio device
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../interface',
          'include',
        ],
      },
      'sources': [
        'include/audio_device.h',
        'include/audio_device_defines.h',
        'audio_device_buffer.cc',
        'audio_device_buffer.h',
        'audio_device_generic.cc',
        'audio_device_generic.h',
        'audio_device_utility.cc',
        'audio_device_utility.h',
        'audio_device_impl.cc',
        'audio_device_impl.h',
        'audio_device_config.h',
        'dummy/audio_device_dummy.cc',
        'dummy/audio_device_dummy.h',
        'dummy/audio_device_utility_dummy.cc',
        'dummy/audio_device_utility_dummy.h',
        'linux/alsasymboltable_linux.cc',
        'linux/alsasymboltable_linux.h',
        'linux/audio_device_alsa_linux.cc',
        'linux/audio_device_alsa_linux.h',
        'linux/audio_device_utility_linux.cc',
        'linux/audio_device_utility_linux.h',
        'linux/audio_mixer_manager_alsa_linux.cc',
        'linux/audio_mixer_manager_alsa_linux.h',
        'linux/latebindingsymboltable_linux.cc',
        'linux/latebindingsymboltable_linux.h',
        'ios/audio_device_ios.cc',
        'ios/audio_device_ios.h',
        'ios/audio_device_utility_ios.cc',
        'ios/audio_device_utility_ios.h',
        'mac/audio_device_mac.cc',
        'mac/audio_device_mac.h',
        'mac/audio_device_utility_mac.cc',
        'mac/audio_device_utility_mac.h',
        'mac/audio_mixer_manager_mac.cc',
        'mac/audio_mixer_manager_mac.h',
        'mac/portaudio/pa_memorybarrier.h',
        'mac/portaudio/pa_ringbuffer.c',
        'mac/portaudio/pa_ringbuffer.h',
        'win/audio_device_core_win.cc',
        'win/audio_device_core_win.h',
        'win/audio_device_wave_win.cc',
        'win/audio_device_wave_win.h',
        'win/audio_device_utility_win.cc',
        'win/audio_device_utility_win.h',
        'win/audio_mixer_manager_win.cc',
        'win/audio_mixer_manager_win.h',
        'android/audio_device_utility_android.cc',
        'android/audio_device_utility_android.h',
      ],
      'conditions': [
        ['OS=="linux"', {
          'include_dirs': [
            'linux',
          ],
          'defines': [
            'LINUX_ALSA',
          ],
          'link_settings': {
            'libraries': [
              '-ldl','-lX11',
            ],
          },
          'conditions': [
            ['include_pulse_audio==1', {
              'defines': [
                'LINUX_PULSE',
              ],
              'sources': [
                'linux/audio_device_pulse_linux.cc',
                'linux/audio_device_pulse_linux.h',
                'linux/audio_mixer_manager_pulse_linux.cc',
                'linux/audio_mixer_manager_pulse_linux.h',
                'linux/pulseaudiosymboltable_linux.cc',
                'linux/pulseaudiosymboltable_linux.h',
              ],
            }],
          ],
        }], # OS==linux
        ['OS=="ios"', {
          'include_dirs': [
            'ios',
          ],
        }], # OS==ios
        ['OS=="mac"', {
          'include_dirs': [
            'mac',
          ],
        }], # OS==mac
        ['OS=="win"', {
          'include_dirs': [
            'win',
          ],
          'link_settings': {
            'libraries': [
              # Required for the built-in WASAPI AEC.
              '-ldmoguids.lib',
              '-lwmcodecdspuuid.lib',
              '-lamstrmid.lib',
              '-lmsdmo.lib',
            ],
          },
        }],
        ['OS=="android"', {
          'include_dirs': [
            'android',
          ],
          'link_settings': {
            'libraries': [
              '-llog',
              '-lOpenSLES',
            ],
          },
          'conditions': [
            ['enable_android_opensl==1', {
              'sources': [
                'android/audio_device_opensles_android.cc',
                'android/audio_device_opensles_android.h',
                'android/audio_manager_jni.cc',
                'android/audio_manager_jni.h',
                'android/fine_audio_buffer.cc',
                'android/fine_audio_buffer.h',
                'android/low_latency_event_posix.cc',
                'android/low_latency_event.h',
                'android/opensles_common.cc',
                'android/opensles_common.h',
                'android/opensles_input.cc',
                'android/opensles_input.h',
                'android/opensles_output.cc',
                'android/opensles_output.h',
                'android/single_rw_fifo.cc',
                'android/single_rw_fifo.h',
              ],
            }, {
              'sources': [
                'android/audio_device_jni_android.cc',
                'android/audio_device_jni_android.h',
              ],
            }],
          ],
        }],
        ['OS=="mac" or OS=="ios"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
            ],
          },
        }],
      ], # conditions
    },
  ],
}

