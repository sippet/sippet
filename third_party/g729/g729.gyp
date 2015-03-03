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
          './include',
        ],
      },
      'conditions': [
        ['g729_amalgamation==1', {
          'sources': [
            'source/g729a.c',
          ],
        }, {
          'include_dirs': [
            './include',
          ],
          'sources': [
            'include/g729a_decoder.h',
            'include/g729a_encoder.h',
            'include/g729a.h',
            'include/basic_op.h',
            'include/ld8a.h',
            'include/oper_32b.h',
            'include/tab_ld8a.h',
            'include/typedef.h',
            'source/acelp_ca.c',
            'source/bits.c',
            'source/cod_ld8a.c',
            'source/cor_func.c',
            'source/de_acelp.c',
            'source/dec_gain.c',
            'source/dec_lag3.c',
            'source/dec_ld8a.c',
            'source/dspfunc.c',
            'source/filter.c',
            'source/g729a_decoder.c',
            'source/g729a_encoder.c',
            'source/gainpred.c',
            'source/lpc.c',
            'source/lpcfunc.c',
            'source/lspdec.c',
            'source/lspgetq.c',
            'source/oper_32b.c',
            'source/pitch_a.c',
            'source/postfilt.c',
            'source/post_pro.c',
            'source/p_parity.c',
            'source/pred_lt3.c',
            'source/pre_proc.c',
            'source/qua_gain.c',
            'source/qua_lsp.c',
            'source/tab_ld8a.c',
            'source/taming.c',
            'source/util.c',
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
      'target_name': 'g729_unittest',
      'type': 'executable',
      'include_dirs': [
        '<(DEPTH)',
      ],
      'dependencies': [
        'g729',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ],
      'sources': [
        'source/g729a_unittest.cc',
      ],
    },  # target g729_unittest
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
        'source/g729a_coder.cc',
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
        'source/g729a_decoder.cc',
      ],
    },  # target g729_decoder
  ],
}
