/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*-------------------------------------------------------------------*
 * Function  Pred_lt_3()                                             *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractional        *
 * interpolation of resolution 1/3.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"

void WebRtcG729fix_Pred_lt_3(
  int16_t   exc[],       /* in/out: excitation buffer */
  int16_t   T0,          /* input : integer pitch lag */
  int16_t   frac,        /* input : fraction of lag   */
  int16_t   L_subfr      /* input : subframe size     */
)
{
  int16_t   i, j, k;
  int16_t   *x0, *x1, *x2, *c1, *c2;
  int32_t  s;

  x0 = &exc[-T0];

  frac = negate(frac);
  if (frac < 0)
  {
    frac = WebRtcSpl_AddSatW16(frac, UP_SAMP);
    x0--;
  }

  for (j=0; j<L_subfr; j++)
  {
    x1 = x0++;
    x2 = x0;
    c1 = &WebRtcG729fix_inter_3l[frac];
    c2 = &WebRtcG729fix_inter_3l[WebRtcSpl_SubSatW16(UP_SAMP,frac)];

    s = 0;
    for(i=0, k=0; i< L_INTER10; i++, k+=UP_SAMP)
    {
      s = L_mac(s, x1[-i], c1[k]);
      s = L_mac(s, x2[i],  c2[k]);
    }

    exc[j] = L_round(s);
  }

  return;
}
