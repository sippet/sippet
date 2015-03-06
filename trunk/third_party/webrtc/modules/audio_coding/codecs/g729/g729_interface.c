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


size_t prm2bits_rtp(const int16_t prm[PRM_SIZE+1], uint8_t *bits) {
  if (prm[0] == 1) {

    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |L|      L1     |    L2   |    L3   |       P1      |P|    C1   |
    // |0|             |         |         |               |0|         |
    // | |0 1 2 3 4 5 6|0 1 2 3 4|0 1 2 3 4|0 1 2 3 4 5 6 7| |0 1 2 3 4|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |       C1      |  S1   | GA1 |  GB1  |    P2   |      C2       |
    // |          1 1 1|       |     |       |         |               |
    // |5 6 7 8 9 0 1 2|0 1 2 3|0 1 2|0 1 2 3|0 1 2 3 4|0 1 2 3 4 5 6 7|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   C2    |  S2   | GA2 |  GB2  |
    // |    1 1 1|       |     |       |
    // |8 9 0 1 2|0 1 2 3|0 1 2|0 1 2 3|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    // 1st codebook               1 + 7 bit (L0 + L1)
    bits[0] = (uint8_t)prm[1];

    // 2nd codebook               5 + 5 bit (L2 + L3)
    bits[1] = (uint8_t)(prm[2] >> 2);
    bits[2] = (uint8_t)(prm[2] << 6);

    // --- 1st subframe:
    // pitch period                   8 bit (P1)
    bits[2] |= (uint8_t)((prm[3] >> 2) & ((1 << 6) - 1));
    bits[3] = (uint8_t)(prm[3] << 6);

    // parity check on 1st period     1 bit (P0)
    bits[3] |= (uint8_t)((prm[4] & 1) << 5);

    // codebook index1(positions)    13 bit (C1)
    bits[3] |= (uint8_t)((prm[5] >> 8) & ((1 << 5) - 1));
    bits[4] = (uint8_t)prm[5];

    // codebook index2(signs)         4 bit (S1)
    bits[5] = (uint8_t)(prm[6] << 4);

    // pitch and codebook gains   3 + 4 bit (GA1 + GB1)
    bits[5] |= (uint8_t)((prm[7] >> 3) & ((1 << 4) - 1));
    bits[6] = (uint8_t)(prm[7] << 5);

    // --- 2nd subframe:
    // pitch period(relative)         5 bit (P2)
    bits[6] |= (uint8_t)(prm[8] & ((1 << 5) - 1));

    // codebook index1(positions)    13 bit (C2)
    bits[7] = (uint8_t)(prm[9] >> 5);
    bits[8] = (uint8_t)(prm[9] << 3);

    // codebook index2(signs)         4 bit (S2)
    bits[8] |= (uint8_t)((prm[10] >> 1) & ((1 << 3) - 1));
    bits[9] = (uint8_t)(prm[10] << 7);

    // pitch and codebook gains   3 + 4 bit (GA2 + GB2)
    bits[9] |= (uint8_t)(prm[11] & ((1 << 7) - 1));

    return 10;
  } else if (prm[0] == 2) {

    //  0                   1
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |L|  LSF1   |  LSF2 |   GAIN  |R|
    // |S|         |       |         |E|
    // |F|         |       |         |S|
    // |0|0 1 2 3 4|0 1 2 3|0 1 2 3 4|V|    RESV = Reserved (zero)
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    // SID Lsp : MA                1 bit (LSF0)
    bits[0] = (uint8_t)(prm[1] << 7);

    // SID Lsp : 1st stage         5 bit (LSF1)
    bits[0] |= (uint8_t)((prm[2] & ((5 << 1) - 1)) << 2);

    // SID Lsp : 2nd stage         4 bit (LSF2)
    bits[0] |= (uint8_t)((prm[3] >> 2) & ((2 << 1) - 1));
    bits[1] = (uint8_t)(prm[3] << 6);

    // SID gain                    5 bit (GAIN)
    bits[1] |= (uint8_t)((prm[4] & ((5 << 1) - 1)) << 1);

    return 2;
  }
  return (size_t)-1;
}

void bits2prm_rtp(const uint8_t *bits, size_t len, int16_t prm[PRM_SIZE+2]) {
  prm[0] = 0; // no frame erasure
  if (len == 10) {
    prm[1] = 1;

    // Reverting previous encoding
    prm[2] = bits[0];
    prm[3] = ((int16_t)bits[1] << 2) | (bits[2] >> 6);
    prm[4] = (((int16_t)bits[2] << 2) | (bits[3] >> 6)) & ((1 << 8) - 1);
    prm[5] = (bits[3] >> 5) & 1;
    prm[6] = (((int16_t)bits[3] & ((1 << 5) - 1)) << 8) | bits[4];
    prm[7] = bits[5] >> 4;
    prm[8] = ((bits[5] & ((1 << 4) - 1)) << 3) | (bits[6] >> 5);
    prm[9] = bits[6] & ((1 << 5) - 1);
    prm[10] = ((int16_t)bits[7] << 5) | (bits[8] >> 3);
    prm[11] = ((bits[8] & ((1 << 3) - 1)) << 1) | (bits[9] >> 7);
    prm[12] = bits[9] & ((1 << 7) - 1);

    // Check parity and put 1 in parm[5] if parity error
    prm[5] = Check_Parity_Pitch(prm[4], prm[5]);

  } else if (len == 2) {
    prm[1] = 2;

    // Reverting previous encoding
    prm[2] = bits[0] >> 7;
    prm[3] = (bits[0] >> 2) & ((1 << 5) - 1);
    prm[4] = ((bits[0] & ((1 << 2) - 1)) << 2) | (bits[1] >> 6);
    prm[5] = (bits[1] >> 1) & ((1 << 5) - 1);
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

  for (i = 0; i < frms; ++i) {

    if (encInst->frame == 32767) encInst->frame = 256;
    else encInst->frame++;

    Pre_Process(&encInst->pre_process_state, &input[i*L_FRAME],
        encInst->state.new_speech, L_FRAME);
    Coder_ld8a(&encInst->state, encInst->parm, encInst->frame, 0); // VAD off

    if (encInst->parm[0] != 1 && encInst->parm[0] != 2)
      break; // Unrecognized payload generated by the encoder!!!

    size += prm2bits_rtp(encInst->parm, &output[size]);
  }

  return (int16_t)size;
}

int16_t WebRtcG729_Decode(G729DecInst* decInst, const uint8_t* encoded,
                          int16_t len, int16_t* decoded, int16_t* speechType) {
  size_t i, j;
  size_t speech_frms = len / 10;
  size_t sid_frms = (len % 10) / 2;
  int16_t Vad = 0;

  // One or more G.729A frames
  for (i = 0; i < speech_frms; i++) {
    bits2prm_rtp(&encoded[i*10], 10, decInst->parm);

    Decod_ld8a(&decInst->state, decInst->parm, decInst->synth,
               decInst->Az_dec, decInst->T2, &Vad, 0);

    Post_Filter(&decInst->post_filter_state, decInst->synth,
                decInst->Az_dec, decInst->T2, Vad);

    Post_Process(&decInst->post_proc_state, decInst->synth, L_FRAME);

    memcpy(&decoded[i*L_FRAME], decInst->synth, L_FRAME*sizeof(int16_t));
  }

  // Followed by one or more G.729B frames
  for (j = 0; j < sid_frms; j++) {
    bits2prm_rtp(&encoded[i*10+j*2], 2, decInst->parm);

    Decod_ld8a(&decInst->state, decInst->parm, decInst->synth,
               decInst->Az_dec, decInst->T2, &Vad, 0);

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

