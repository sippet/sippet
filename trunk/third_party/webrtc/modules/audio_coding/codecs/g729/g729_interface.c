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
#include "g729a_encoder.h"
#include "g729a_decoder.h"

int16_t WebRtcG729_EncoderCreate(G729EncInst** inst) {
  if (inst == NULL)
    return -1;
  *inst = (G729EncInst*) calloc(sizeof(G729EncInst), 1);
  if (!*inst)
    return -1;
  (*inst)->enc = calloc(g729a_enc_mem_size(), 1);
  if (!(*inst)->enc) {
    free(*inst);
    return -1;
  }
  return 0;
}

int16_t WebRtcG729_EncoderInit(G729EncInst* encInst) {
  if (!g729a_enc_init(encInst->enc))
    return -1;
  return 0;
}

int16_t WebRtcG729_EncoderFree(G729EncInst* encInst) {
  if (encInst == NULL)
    return -1;
  g729a_enc_deinit(encInst->enc);
  free(encInst->enc);
  free(encInst);
  return 0;
}

int16_t WebRtcG729_DecoderCreate(G729DecInst** inst) {
  if (inst == NULL)
    return -1;
  *inst = (G729DecInst *) calloc(sizeof(G729DecInst), 1);
  if (!*inst)
    return -1;
  (*inst)->dec = calloc(g729a_dec_mem_size(), 1);
  if (!(*inst)->dec) {
    free(*inst);
    return -1;
  }
  return 0;
}

int16_t WebRtcG729_DecoderInit(G729DecInst* decInst) {
  if (!g729a_dec_init(decInst->dec))
    return -1;
  return 0;
}

int16_t WebRtcG729_DecoderFree(G729DecInst* decInst) {
  if (decInst == NULL)
    return -1;
  g729a_dec_deinit(decInst->dec);
  free(decInst->dec);
  free(decInst);
  return 0;
}

int16_t WebRtcG729_Encode(G729EncInst* encInst, const int16_t* input,
                          int16_t len, uint8_t* output) {
  int16_t i, frms = len / L_FRAME;
  for (i = 0; i < frms; ++i) {
    g729a_enc_process(encInst->enc, (Word16*)&input[i*L_FRAME],
                      (UWord8*)&output[i*L_PACKED_G729A]);
  }
  return frms * L_PACKED_G729A;
}

int16_t WebRtcG729_Decode(G729DecInst* decInst, const uint8_t* encoded,
                          int16_t len, int16_t* decoded, int16_t* speechType) {
  int i;
  UWord8* frames = (UWord8*)encoded;

  if (len <= 0 || len % L_PACKED_G729A != 0) {
    /* Unsupported frame length */
    return -1;
  }

  for (i = 0; len - i*L_PACKED_G729A > 0; ++i) {
    g729a_dec_process(decInst->dec, &frames[i*L_PACKED_G729A],
                      &decoded[i*L_FRAME], 0);
  }

  /* Save last frame for PLC */
  if (i > 0) {
    memcpy(decInst->last_frm, &frames[(i-1)*L_PACKED_G729A],
           L_PACKED_G729A);
  }

  *speechType = 1;
  return i*L_FRAME;
}

int16_t WebRtcG729_DecodePlc(G729DecInst* decInst, int16_t* decoded,
                             int16_t noOfLostFrames) {
  int16_t i;

  for (i = 0; i < noOfLostFrames; ++i) {
    g729a_dec_process(decInst->dec, decInst->last_frm,
                      &decoded[i*L_FRAME], 1);
  }

  return noOfLostFrames*L_FRAME;
}
