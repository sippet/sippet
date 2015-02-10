# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../build/win_precompile.gypi',
  ],
  'variables': {
    'g729_source%': "source",
  },
  'targets': [
    {
      'target_name': 'g729',
      'type': 'static_library',
      'include_dirs': [
        '<(g729_source)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(g729_source)',
        ],
      },
      'variables': {
        'clang_warning_flags': [
          # g729 has code like:
          # *p0++ = mult(*p0, psign[i1]);
          #    ^          ~~
          '-Wno-unsequenced',
          # It also has "if (a) if (b) s; else s2;" ambiguities
          '-Wno-dangling-else',
        ],
      },
      'cflags': [
        '-fno-builtin',
      ],
      'sources': [
        '<(g729_source)/acelp_ca.c',
        '<(g729_source)/basic_op.c',
        '<(g729_source)/basic_op.h',
        '<(g729_source)/bits.c',
        '<(g729_source)/calcexc.c',
        '<(g729_source)/cod_ld8a.c',
        '<(g729_source)/cor_func.c',
        '<(g729_source)/de_acelp.c',
        '<(g729_source)/dec_gain.c',
        '<(g729_source)/dec_lag3.c',
        '<(g729_source)/dec_ld8a.c',
        '<(g729_source)/dec_sid.c',
        '<(g729_source)/dspfunc.c',
        '<(g729_source)/dtx.c',
        '<(g729_source)/dtx.h',
        '<(g729_source)/filter.c',
        '<(g729_source)/gainpred.c',
        '<(g729_source)/ld8a.h',
        '<(g729_source)/lpc.c',
        '<(g729_source)/lpcfunc.c',
        '<(g729_source)/lspdec.c',
        '<(g729_source)/lspgetq.c',
        '<(g729_source)/octet.h',
        '<(g729_source)/oper_32b.c',
        '<(g729_source)/oper_32b.h',
        '<(g729_source)/pitch_a.c',
        '<(g729_source)/postfilt.c',
        '<(g729_source)/post_pro.c',
        '<(g729_source)/p_parity.c',
        '<(g729_source)/pred_lt3.c',
        '<(g729_source)/pre_proc.c',
        '<(g729_source)/qsidgain.c',
        '<(g729_source)/qsidlsf.c',
        '<(g729_source)/qua_gain.c',
        '<(g729_source)/qua_lsp.c',
        '<(g729_source)/sid.h',
        '<(g729_source)/tab_dtx.c',
        '<(g729_source)/tab_dtx.h',
        '<(g729_source)/tab_ld8a.c',
        '<(g729_source)/tab_ld8a.h',
        '<(g729_source)/taming.c',
        '<(g729_source)/typedef.h',
        '<(g729_source)/util.c',
        '<(g729_source)/vad.c',
        '<(g729_source)/vad.h',
      ],
    },  # target g729
  ],
}
