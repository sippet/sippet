/*
 *  Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef G729_INTERFACE_H
#define G729_INTERFACE_H

#include <typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_PACKED_G729A 10

typedef struct WebRtcG729EncInst G729EncInst;
typedef struct WebRtcG729DecInst G729DecInst;

int16_t WebRtcG729_EncoderCreate(G729EncInst** inst);
int16_t WebRtcG729_DecoderCreate(G729DecInst** inst);
int16_t WebRtcG729_EncoderFree(G729EncInst* inst);
int16_t WebRtcG729_DecoderFree(G729DecInst* inst);
int16_t WebRtcG729_Encode(G729EncInst* encInst, const int16_t* input,
                          int16_t len, uint8_t* output);
int16_t WebRtcG729_EncoderInit(G729EncInst* encInst, int enableDtx);
int16_t WebRtcG729_Decode(G729DecInst* decInst, const uint8_t* encoded,
                          int16_t len, int16_t* decoded, int16_t* speechType);
int16_t WebRtcG729_DecodePlc(G729DecInst* decInst, int16_t* decodec,
                             int16_t frames);
int16_t WebRtcG729_DecoderInit(G729DecInst* decInst);

#ifdef __cplusplus
}
#endif

#endif // G729_INTERFACE_H

/* Modeline for vim: set tw=79 et ts=4: */

