/*
 *  Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ld8a.h"
#include "dtx.h"

struct WebRtcG729EncInst {
  Pre_Process_state pre_process_state;
  Coder_ld8a_state state;
  int16_t frame; // frame counter
  int16_t parm[PRM_SIZE+1];
  int dtx;
};

struct WebRtcG729DecInst {
  Decod_ld8a_state state;
  Post_Filter_state post_filter_state;
  Post_Process_state post_proc_state;

  int16_t  synth_buf[L_FRAME+M], *synth; // Synthesis
  int16_t  parm[PRM_SIZE+2];             // Synthesis parameters
  int16_t  Az_dec[MP1*2];                // Decoded Az for post-filter
  int16_t  T2[2];                        // Pitch lag for 2 subframes
};

size_t prm2bits_rtp(const int16_t prm[PRM_SIZE+1], uint8_t *pkt);
void bits2prm_rtp(const uint8_t *bits, size_t len, int16_t prm[PRM_SIZE+2]);

#ifdef __cplusplus
}
#endif

#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_
