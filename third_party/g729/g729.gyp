# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'g729_amalgamation%': 0,
  },
  'includes': [
    '../../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'g729',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/third_party/webrtc/common_audio/common_audio.gyp:common_audio',
      ],
      'variables': {
        'clang_warning_flags': [
          # The code has constructs like:
          # if ((UWord32)(t0 - 0xfe000000L) < 0x01ffffffL -  0xfe000000L)
          '-Wno-tautological-constant-out-of-range-compare',
        ],
        'optimize': 'max',
      },
      'direct_dependent_settings': {
        'include_dirs': [
          './source',
          '<(DEPTH)/third_party',
          '<(DEPTH)/third_party/webrtc/common_audio/signal_processing/include',
        ],
      },
      'conditions': [
        ['g729_amalgamation==1', {
          'sources': [
            'source/g729a.c',
          ],
        }, {
          'include_dirs': [
            './source',
          ],
          'sources': [
            'source/acelp_ca.c',
            'source/basic_op.h',
            'source/bits.c',
            'source/calcexc.c',
            'source/cod_ld8a.c',
            'source/cor_func.c',
            'source/de_acelp.c',
            'source/dec_gain.c',
            'source/dec_lag3.c',
            'source/dec_ld8a.c',
            'source/dec_sid.c',
            'source/dspfunc.c',
            'source/dtx.c',
            'source/dtx.h',
            'source/filter.c',
            'source/gainpred.c',
            'source/ld8a.h',
            'source/lpc.c',
            'source/lpcfunc.c',
            'source/lspdec.c',
            'source/lspgetq.c',
            'source/octet.h',
            'source/oper_32b.c',
            'source/oper_32b.h',
            'source/pitch_a.c',
            'source/postfilt.c',
            'source/post_pro.c',
            'source/p_parity.c',
            'source/pred_lt3.c',
            'source/pre_proc.c',
            'source/qsidgain.c',
            'source/qsidlsf.c',
            'source/qua_gain.c',
            'source/qua_lsp.c',
            'source/sid.h',
            'source/tab_dtx.c',
            'source/tab_dtx.h',
            'source/tab_ld8a.c',
            'source/tab_ld8a.h',
            'source/taming.c',
            'source/vad.c',
            'source/vad.h',
          ],
        }],
        ['OS=="android" or OS=="ios"', {
          # Available assembly optimizations are valid only
          # for GCC and LLVM compilers
          'conditions': [
            ['target_arch=="ia32"', {
              'defines': [ 'ARCH_X86' ]
            }],
            ['target_arch=="arm"', {
              'defines': [ 'ARCH_ARM' ]
            }],
            ['OS=="android"', {
              'defines': [ 'OS_ANDROID' ],
            }],
            ['OS=="ios"', {
              'defines': [ 'OS_IOS' ],
            }],
          ],
        }], # OS=="android" or OS=="ios"
      ],
      'includes': [
        '../../build/android/increase_size_for_speed.gypi',
      ],
    },  # target g729
    {
      'target_name': 'g729_coder',
      'type': 'executable',
      'include_dirs': [
        '<(DEPTH)',
      ],
      'dependencies': [
        'g729',
      ],
      'sources': [
        'source/coder.c',
      ],
    },  # target g729_coder
    {
      'target_name': 'g729_decoder',
      'type': 'executable',
      'include_dirs': [
        '<(DEPTH)',
      ],
      'dependencies': [
        'g729',
      ],
      'sources': [
        'source/decoder.c',
      ],
    },  # target g729_decoder
  ],
}
