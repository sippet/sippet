# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../build/win_precompile.gypi',
  ],
  'targets': [
    {
      'target_name': 'g729',
      'type': 'static_library',
      'include_dirs': [
        './overrides',
        './include',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          './overrides',
          './include',
        ],
      },
      'variables': {
        'clang_warning_flags': [
          # g729 has code like:
          # *p0++ = mult(*p0, psign[i1]);
          #    ^          ~~
          #'-Wno-unsequenced',
          # It also has "if (a) if (b) s; else s2;" ambiguities
          #'-Wno-dangling-else',
        ],
      },
      'cflags': [
        # g729 source uses 'round' builtin
        #'-fno-builtin',
      ],
      'sources': [
        'overrides/include/basic_op.h',
        'include/g729a_decoder.h',
        'include/g729a_encoder.h',
        'include/g729a.h',
        'include/ld8a.h',
        'include/libavcodec_get_bits.h',
        'overrides/libavcodec_put_bits.h',
        'include/oper_32b.h',
        'include/tab_ld8a.h',
        'include/typedef.h',
        'source/acelp_ca.c',
        'source/basic_op.c',
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
    },  # target g729
  ],
}
