/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*-----------------------------------------------------------*
 *  Function  Decod_ACELP()                                  *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

void WebRtcG729fix_Decod_ACELP(
  int16_t sign,      /* (i)     : signs of 4 pulses.                       */
  int16_t index,     /* (i)     : Positions of the 4 pulses.               */
  int16_t cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
)
{
  int16_t i, j;
  int16_t pos[4];


  /* Decode the positions */

  i      = index & (int16_t)7;
  pos[0] = WebRtcSpl_AddSatW16(i, shl(i, 2));           /* pos0 =i*5 */

  index  = shr(index, 3);
  i      = index & (int16_t)7;
  i      = WebRtcSpl_AddSatW16(i, shl(i, 2));           /* pos1 =i*5+1 */
  pos[1] = WebRtcSpl_AddSatW16(i, 1);

  index  = shr(index, 3);
  i      = index & (int16_t)7;
  i      = WebRtcSpl_AddSatW16(i, shl(i, 2));           /* pos2 =i*5+1 */
  pos[2] = WebRtcSpl_AddSatW16(i, 2);

  index  = shr(index, 3);
  j      = index & (int16_t)1;
  index  = shr(index, 1);
  i      = index & (int16_t)7;
  i      = WebRtcSpl_AddSatW16(i, shl(i, 2));           /* pos3 =i*5+3+j */
  i      = WebRtcSpl_AddSatW16(i, 3);
  pos[3] = WebRtcSpl_AddSatW16(i, j);

  /* decode the signs  and build the codeword */

  WebRtcSpl_ZerosArrayW16(cod, L_SUBFR);

  for (j = 0; j < 4; j++)
  {
    i = sign & (int16_t)1;
    sign = shr(sign, 1);
    if (i != 0) {
      cod[pos[j]] = 8191;      /* Q13 +1.0 */
    }
    else {
      cod[pos[j]] = -8192;     /* Q13 -1.0 */
    }
  }

  return;
}
