/*
 *  Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/******************************************************************

 G729 Speech Coder ANSI-C Source Code

 g729_interface.c

******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "g729_interface.h"
#include "g729_inst.h"


#pragma pack(1)

struct speech_parms {
#ifdef WORDS_BIGENDIAN
  uint8_t LPC_cb1;              // 1st codebook           7 + 1 bit
  uint8_t LPC_cb21;             // 2nd codebook           5 + 5 bit
  uint8_t LPC_cb22 : 2;
                                // --- 1st subframe:
  uint8_t pitch_period1 : 6;    // pitch period                   8 bit
  uint8_t pitch_period2 : 2;
  uint8_t parity_check : 1;     // parity check on 1st period     1 bit
  uint8_t cb_index11 : 5;       // codebook index1(positions)    13 bit
  uint8_t cb_index12;
  uint8_t cb_index2 : 4;        // codebook index2(signs)         4 bit
  uint8_t cb_gains1 : 4;        // pitch and codebook gains   4 + 3 bit
  uint8_t cb_gains2 : 3;
                                // --- 2nd subframe:
  uint8_t rel_pitch_period : 5; // pitch period(relative)         5 bit
  uint8_t pos_cb_index11;       // codebook index1(positions)    13 bit
  uint8_t pos_cb_index12 : 5;
  uint8_t sign_cb_index21 : 3;  // codebook index2(signs)         4 bit
  uint8_t sign_cb_index22 : 1;
  uint8_t pitch_cb_gains : 7;   // pitch and codebook gains   4 + 3 bit
#else
  uint8_t LPC_cb1;
  uint8_t LPC_cb21;
  uint8_t pitch_period1 : 6;
  uint8_t LPC_cb22 : 2;
  uint8_t cb_index11 : 5;
  uint8_t parity_check : 1;
  uint8_t pitch_period2 : 2;
  uint8_t cb_index12;
  uint8_t cb_gains1 : 4;
  uint8_t cb_index2 : 4;
  uint8_t rel_pitch_period : 5;
  uint8_t cb_gains2 : 3;
  uint8_t pos_cb_index11;
  uint8_t sign_cb_index21 : 3;
  uint8_t pos_cb_index12 : 5;
  uint8_t pitch_cb_gains : 7;
  uint8_t sign_cb_index22 : 1;
#endif
};

struct sid_parms {
#ifdef WORDS_BIGENDIAN
  uint8_t MA : 1;      // SID Lsp : MA                1 bit
  uint8_t stage1 : 5;  // SID Lsp : 1st stage         5 bit
  uint8_t stage21 : 2;  // SID Lsp : 2nd stage        4 bit
  uint8_t stage22 : 2;
  uint8_t gain : 5;    // SID gain                    5 bit
  uint8_t pad : 1;
#else
  uint8_t stage21 : 2;
  uint8_t stage1 : 5;
  uint8_t MA : 1;
  uint8_t pad : 1;
  uint8_t gain : 5;
  uint8_t stage22 : 2;
#endif
};

#pragma pack()

static size_t prm2packet(const int16_t prm[PRM_SIZE+1], uint8_t *pkt) {
  if (prm[0] == 1) {
    struct speech_parms *prms_map = (struct speech_parms*)pkt;
    prms_map->LPC_cb1 = (uint8_t)prm[1];
    prms_map->LPC_cb21 = (uint8_t)(prm[2] >> 2);
    prms_map->LPC_cb22 = (uint8_t)(prm[2] & ((1 << 2) - 1));
    prms_map->pitch_period1 = (uint8_t)(prm[3] >> 2);
    prms_map->pitch_period2 = (uint8_t)(prm[3] & ((1 << 2) - 1));
    prms_map->parity_check = (uint8_t)prm[4];
    prms_map->cb_index11 = (uint8_t)(prm[5] >> 8);
    prms_map->cb_index12 = (uint8_t)(prm[5] & ((1 << 8) - 1));
    prms_map->cb_index2 = (uint8_t)prm[6];
    prms_map->cb_gains1 = (uint8_t)(prm[7] >> 3);
    prms_map->cb_gains2 = (uint8_t)(prm[7] & ((1 << 3) - 1));
    prms_map->rel_pitch_period = (uint8_t)prm[8];
    prms_map->pos_cb_index11 = (uint8_t)(prm[9] >> 5);
    prms_map->pos_cb_index12 = (uint8_t)(prm[9] & ((1 << 5) - 1));
    prms_map->sign_cb_index21 = (uint8_t)(prm[10] >> 1);
    prms_map->sign_cb_index22 = (uint8_t)(prm[10] & ((1 << 1) - 1));
    prms_map->pitch_cb_gains = (uint8_t)(prm[11]);
    return 10;
  } else if (prm[0] == 2) {
    struct sid_parms *prms_map = (struct sid_parms*)pkt;
    prms_map->MA = (uint8_t)prm[1];
    prms_map->stage1 = (uint8_t)prm[2];
    prms_map->stage21 = (uint8_t)(prm[3] >> 2);
    prms_map->stage22 = (uint8_t)(prm[3] & ((1 << 2) - 1));
    prms_map->gain = (uint8_t)prm[4];
    prms_map->pad = 0;
    return 2;
  }
  return -1;
}

static void packet2prm(const uint8_t *bits, size_t len, int16_t prm[PRM_SIZE+2]) {
  prm[0] = 0; // no frame erasure
  if (len == 10) {
    const struct speech_parms *prms_map = (const struct speech_parms *)bits;
    prm[1] = 1;
    prm[2] = prms_map->LPC_cb1;
    prm[3] = ((int16_t)prms_map->LPC_cb21 << 2) | prms_map->LPC_cb22;
    prm[4] = ((int16_t)prms_map->pitch_period1 << 2) | prms_map->pitch_period2;
    prm[5] = prms_map->parity_check;
    prm[6] = ((int16_t)prms_map->cb_index11 << 8) | prms_map->cb_index12;
    prm[7] = prms_map->cb_index2;
    prm[8] = ((int16_t)prms_map->cb_gains1 << 3) | prms_map->cb_gains2;
    prm[9] = prms_map->rel_pitch_period;
    prm[10] = ((int16_t)prms_map->pos_cb_index11 << 5) | prms_map->pos_cb_index12;
    prm[11] = ((int16_t)prms_map->sign_cb_index21 << 1) | prms_map->sign_cb_index22;
    prm[12] = prms_map->pitch_cb_gains;

    // Check parity and put 1 in parm[5] if parity error
    prm[5] = Check_Parity_Pitch(prm[4], prm[5]);

  } else if (len == 2) {
    const struct sid_parms *prms_map = (const struct sid_parms *)bits;
    prm[1] = 2;
    prm[2] = prms_map->MA;
    prm[3] = prms_map->stage1;
    prm[4] = (prms_map->stage21 << 2) | prms_map->stage22;
    prm[5] = prms_map->gain;
  }
}

int16_t WebRtcG729_EncoderCreate(G729EncInst** inst) {
  if (inst == NULL)
    return -1;
  *inst = (G729EncInst*) calloc(sizeof(G729EncInst), 1);
  if (!*inst)
    return -1;
  return 0;
}

int16_t WebRtcG729_EncoderInit(G729EncInst* encInst) {
  if (encInst == NULL)
    return -1;
  Init_Pre_Process(&encInst->pre_process_state);
  Init_Coder_ld8a(&encInst->state);
  Init_Cod_cng(&encInst->state); // for G.729B
  encInst->frame = 0;
  return 0;
}

int16_t WebRtcG729_EncoderFree(G729EncInst* encInst) {
  if (encInst == NULL)
    return -1;
  free(encInst);
  return 0;
}

int16_t WebRtcG729_DecoderCreate(G729DecInst** inst) {
  if (inst == NULL)
    return -1;
  *inst = (G729DecInst *) calloc(sizeof(G729DecInst), 1);
  if (!*inst)
    return -1;
  return 0;
}

int16_t WebRtcG729_DecoderInit(G729DecInst* decInst) {
  if (decInst == NULL)
    return -1;

  for (size_t i = 0; i < M; i++)
    decInst->synth_buf[i] = 0;
  decInst->synth = decInst->synth_buf + M;

  Init_Decod_ld8a(&decInst->state);
  Init_Post_Filter(&decInst->post_filter_state);
  Init_Post_Process(&decInst->post_proc_state);
  Init_Dec_cng(&decInst->state); // for G.729B

  return 0;
}

int16_t WebRtcG729_DecoderFree(G729DecInst* decInst) {
  if (decInst == NULL)
    return -1;
  free(decInst);
  return 0;
}

int16_t WebRtcG729_Encode(G729EncInst* encInst, const int16_t* input,
                          int16_t len, uint8_t* output) {
  size_t size = 0;
  size_t i, frms = len / L_FRAME;
  int16_t prm[PRM_SIZE+1];

  for (i = 0; i < frms; ++i) {

    if (encInst->frame == 32767) encInst->frame = 256;
    else encInst->frame++;

    memcpy(encInst->state.new_speech, &input[i*L_FRAME], L_FRAME*sizeof(int16_t));
    Pre_Process(&encInst->pre_process_state, encInst->state.new_speech, L_FRAME);
    Coder_ld8a(&encInst->state, prm, encInst->frame, 0); // VAD off

    size += prm2packet(prm, &output[size]);
  }

  return (int16_t)size;
}

int16_t WebRtcG729_Decode(G729DecInst* decInst, const uint8_t* encoded,
                          int16_t len, int16_t* decoded, int16_t* speechType) {
  size_t i, j;
  size_t speech_frms = len / 10;
  size_t sid_frms = (len % 10) / 2;
  int16_t prm[PRM_SIZE+2], Vad;

  // One or more G.729A frames
  for (i = 0; i < speech_frms; i++) {
    packet2prm(&encoded[i*10], 10, prm);

    Decod_ld8a(&decInst->state, prm, decInst->synth, decInst->Az_dec,
               decInst->T2, &Vad, 0);

    Post_Filter(&decInst->post_filter_state, decInst->synth,
                decInst->Az_dec, decInst->T2, Vad);

    Post_Process(&decInst->post_proc_state, decInst->synth, L_FRAME);

    memcpy(&decoded[i*L_FRAME], decInst->synth, L_FRAME*sizeof(int16_t));
  }

  // Followed by one or more G.729B frames
  for (j = 0; j < sid_frms; j++) {
    packet2prm(&encoded[i*10+j*2], 2, prm);

    Decod_ld8a(&decInst->state, prm, decInst->synth, decInst->Az_dec,
               decInst->T2, &Vad, 0);

    Post_Filter(&decInst->post_filter_state, decInst->synth,
                decInst->Az_dec, decInst->T2, Vad);

    Post_Process(&decInst->post_proc_state, decInst->synth, L_FRAME);

    memcpy(&decoded[(i+j)*L_FRAME], decInst->synth, L_FRAME*sizeof(int16_t));
  }

  *speechType = Vad;
  return (i + j) * L_FRAME;
}

int16_t WebRtcG729_DecodePlc(G729DecInst* decInst, int16_t* decoded,
                             int16_t noOfLostFrames) {
  int16_t i;
  int16_t prm[PRM_SIZE+2], Vad;

  Set_zero(prm, PRM_SIZE+2);
  prm[0] = 1; // frame erasure

  for (i = 0; i < noOfLostFrames; ++i) {

    Decod_ld8a(&decInst->state, prm, decInst->synth, decInst->Az_dec,
               decInst->T2, &Vad, 0);

    Post_Filter(&decInst->post_filter_state, decInst->synth,
                decInst->Az_dec, decInst->T2, Vad);

    Post_Process(&decInst->post_proc_state, decInst->synth, L_FRAME);
    memcpy(&decoded[i*L_FRAME], decInst->synth, L_FRAME*sizeof(int16_t));
  }

  return noOfLostFrames * L_FRAME;
}

