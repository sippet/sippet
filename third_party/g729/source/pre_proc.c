/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*------------------------------------------------------------------------*
 * Function Pre_Process()                                                 *
 *                                                                        *
 * Preprocessing of input speech.                                         *
 *   - 2nd order high pass filter with cut off frequency at 140 Hz.       *
 *   - Divide input by two.                                               *
 *-----------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 140 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2                    *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.92727435E+00, -0.18544941E+01, 0.92727435E+00};          *
 *     a[3] = {0.10000000E+01, 0.19059465E+01, -0.91140240E+00};          *
 *                                                                        *
 *  Input are divided by two in the filtering process.                    *
 *-----------------------------------------------------------------------*/


/* Initialization of static values */

void WebRtcG729fix_Init_Pre_Process(Pre_Process_state *st)
{
  st->y2_hi = 0;
  st->y2_lo = 0;
  st->y1_hi = 0;
  st->y1_lo = 0;
  st->x0   = 0;
  st->x1   = 0;
}


void WebRtcG729fix_Pre_Process(
  Pre_Process_state *st,
  const int16_t sigin[], /* Input signal        */
  int16_t sigout[],      /* Output signal       */
  int16_t lg)            /* Length of signal    */
{
  int16_t i, x2;
  int32_t L_tmp;
  int16_t xx0, xx1, yy1_hi, yy1_lo, yy2_hi, yy2_lo;

  xx1 = st->x1;
  xx0 = st->x0;
  yy1_hi = st->y1_hi;
  yy1_lo = st->y1_lo;
  yy2_hi = st->y2_hi;
  yy2_lo = st->y2_lo;

  for(i=0; i<lg; i++)
  {
     x2 = xx1;
     xx1 = xx0;
     xx0 = sigin[i];

     /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b140[2]*x[i-2]/2  */
     /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

     L_tmp     = WebRtcG729fix_Mpy_32_16(yy1_hi, yy1_lo, WebRtcG729fix_a140[1]);
     L_tmp     = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(yy2_hi, yy2_lo, WebRtcG729fix_a140[2]));
     L_tmp     = L_mac(L_tmp, xx0, WebRtcG729fix_b140[0]);
     L_tmp     = L_mac(L_tmp, xx1, WebRtcG729fix_b140[1]);
     L_tmp     = L_mac(L_tmp, x2, WebRtcG729fix_b140[2]);
     L_tmp     = L_shl(L_tmp, 3);      /* Q28 --> Q31 (Q12 --> Q15) */
     sigout[i] = L_round(L_tmp);

     yy2_hi = yy1_hi;
     yy2_lo = yy1_lo;
     WebRtcG729fix_L_Extract(L_tmp, &yy1_hi, &yy1_lo);
  }

  st->x1 = xx1;
  st->x0 = xx0;
  st->y1_hi = yy1_hi;
  st->y1_lo = yy1_lo;
  st->y2_hi = yy2_hi;
  st->y2_lo = yy2_lo;

  return;
}
