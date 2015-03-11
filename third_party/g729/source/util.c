/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

/* Random generator  */

int16_t WebRtcG729fix_Random(int16_t *seed)
{
  /* seed = seed*31821 + 13849; */
  *seed = extract_l(WebRtcSpl_AddSatW32(L_shr(L_mult(*seed, 31821), 1), 13849L));

  return(*seed);
}

