// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "typedef.h"
#include "g729a_decoder.h"

Word32 g729a_dec_mem_size ()
{
  return sizeof(g729a_decode_frame_state);
}

Flag   g729a_dec_init (void *decState)
{
  g729a_decode_frame_state *state;
  if (decState == NULL)
    return 0;

  state = (g729a_decode_frame_state *)decState;

  Set_zero(state->synth_buf, M);
  state->synth = state->synth_buf + M;

  Init_Decod_ld8a(&state->decoderState);
  Init_Post_Filter(&state->postFilterState);
  Init_Post_Process(&state->postProcessState);

  return 1;
}

void   g729a_dec_process  (void *decState, UWord8 *bitstream, Word16 *pcm,
                           Flag badFrame)
{
  g729a_decode_frame_state *state = (g729a_decode_frame_state *)decState;
  static Word16 bad_lsf = 0;          /* Initialize bad LSF indicator */
  Word16  parm[PRM_SIZE+1];             /* Synthesis parameters        */
  Word16  Az_dec[MP1*2];                /* Decoded Az for post-filter  */
  Word16  T2[2];                        /* Pitch lag for 2 subframes   */

  bits2prm_ld8k( bitstream, &parm[1]);

  parm[0] = badFrame ? 1 : 0;           /* No frame erasure */

  /* check pitch parity and put 1 in parm[4] if parity error */
  parm[4] = Check_Parity_Pitch(parm[3], parm[4]);

  Decod_ld8a(&state->decoderState, parm, state->synth, Az_dec, T2, bad_lsf);

  Post_Filter(&state->postFilterState, state->synth, Az_dec, T2);        /* Post-filter */

  Post_Process(&state->postProcessState, state->synth, pcm, L_FRAME);
}

void   g729a_dec_deinit   (void *decState)
{
  if (decState == NULL)
    return;

  memset(decState,0,sizeof(g729a_decode_frame_state));
}
