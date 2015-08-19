# Copyright (c) 2015 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'g729',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/third_party/webrtc/common_audio/common_audio.gyp:common_audio',
      ],
      'include_dirs': [
        './include',
        './source',
        '<(DEPTH)/third_party/webrtc',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          './source',
          '<(DEPTH)/third_party/webrtc',
        ],
      },
      'sources': [
        'src/acelp_ca.c',
        'src/basic_op.h',
        'src/bits.c',
        'src/calcexc.c',
        'src/cod_ld8a.c',
        'src/cor_func.c',
        'src/de_acelp.c',
        'src/dec_gain.c',
        'src/dec_lag3.c',
        'src/dec_ld8a.c',
        'src/dec_sid.c',
        'src/dspfunc.c',
        'src/dtx.c',
        'src/dtx.h',
        'src/filter.c',
        'src/gainpred.c',
        'src/ld8a.h',
        'src/lpc.c',
        'src/lpcfunc.c',
        'src/lspdec.c',
        'src/lspgetq.c',
        'src/octet.h',
        'src/oper_32b.c',
        'src/oper_32b.h',
        'src/pitch_a.c',
        'src/postfilt.c',
        'src/post_pro.c',
        'src/p_parity.c',
        'src/pred_lt3.c',
        'src/pre_proc.c',
        'src/qsidgain.c',
        'src/qsidlsf.c',
        'src/qua_gain.c',
        'src/qua_lsp.c',
        'src/sid.h',
        'src/tab_dtx.c',
        'src/tab_dtx.h',
        'src/tab_ld8a.c',
        'src/tab_ld8a.h',
        'src/taming.c',
        'src/vad.c',
        'src/vad.h',
        'src/util.c',
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
        'src/coder.c',
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
        'src/decoder.c',
      ],
    },  # target g729_decoder
    {
      'target_name': 'autocorr',
      'type': 'executable',
      'include_dirs': [
        '<(DEPTH)',
      ],
      'dependencies': [
        'g729',
      ],
      'sources': [
        'src/autocorr.c',
      ],
    },  # target autocorr
  ],
}
