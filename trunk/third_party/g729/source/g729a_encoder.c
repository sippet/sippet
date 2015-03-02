// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "typedef.h"
#include "g729a_encoder.h"

Word32 g729a_enc_mem_size ()
{
  return sizeof(g729a_encode_frame_state);
}

Flag   g729a_enc_init     (void *encState)
{
  g729a_encode_frame_state *state;
  if (encState == NULL)
    return 0;

  state = (g729a_encode_frame_state *)encState;

  Init_Pre_Process(&state->preProcessState);
  Init_Coder_ld8a(&state->encoderState);

  return 1;
}

void   g729a_enc_process  (void *encState, Word16 *pcm, UWord8 *bitstream)
{
  g729a_encode_frame_state *state = (g729a_encode_frame_state *)encState;
  Word16 prm[PRM_SIZE];          /* Analysis parameters.                  */

  Pre_Process(&state->preProcessState, pcm, state->encoderState.new_speech, L_FRAME);

  Coder_ld8a(&state->encoderState, prm);

  prm2bits_ld8k( prm, bitstream);
}

void   g729a_enc_deinit   (void *encState)
{
  if (encState == NULL)
    return;

  memset(encState,0,sizeof(g729a_encode_frame_state));
}
