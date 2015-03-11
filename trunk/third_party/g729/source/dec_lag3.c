/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*------------------------------------------------------------------------*
 *    Function Dec_lag3                                                   *
 *             ~~~~~~~~                                                   *
 *   Decoding of fractional pitch lag with 1/3 resolution.                *
 * See "Enc_lag3.c" for more details about the encoding procedure.        *
 *------------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

void WebRtcG729fix_Dec_lag3(
  int16_t index,       /* input : received pitch index           */
  int16_t pit_min,     /* input : minimum pitch lag              */
  int16_t pit_max,     /* input : maximum pitch lag              */
  int16_t i_subfr,     /* input : subframe flag                  */
  int16_t *T0,         /* output: integer part of pitch lag      */
  int16_t *T0_frac     /* output: fractional part of pitch lag   */
)
{
  int16_t i;
  int16_t T0_min, T0_max;

  if (i_subfr == 0)                  /* if 1st subframe */
  {
    if (index < 197)
    {
      /* *T0 = (index+2)/3 + 19 */

      *T0 = WebRtcSpl_AddSatW16(mult(WebRtcSpl_AddSatW16(index, 2), 10923), 19);

      /* *T0_frac = index - *T0*3 + 58 */

      i = WebRtcSpl_AddSatW16(WebRtcSpl_AddSatW16(*T0, *T0), *T0);
      *T0_frac = WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(index, i), 58);
    }
    else
    {
      *T0 = WebRtcSpl_SubSatW16(index, 112);
      *T0_frac = 0;
    }

  }

  else  /* second subframe */
  {
    /* find T0_min and T0_max for 2nd subframe */

    T0_min = WebRtcSpl_SubSatW16(*T0, 5);
    if (T0_min < pit_min)
    {
      T0_min = pit_min;
    }

    T0_max = WebRtcSpl_AddSatW16(T0_min, 9);
    if (T0_max > pit_max)
    {
      T0_max = pit_max;
      T0_min = WebRtcSpl_SubSatW16(T0_max, 9);
    }

    /* i = (index+2)/3 - 1 */
    /* *T0 = i + t0_min;    */

    i = WebRtcSpl_SubSatW16(mult(WebRtcSpl_AddSatW16(index, 2), 10923), 1);
    *T0 = WebRtcSpl_AddSatW16(i, T0_min);

    /* t0_frac = index - 2 - i*3; */

    i = WebRtcSpl_AddSatW16(WebRtcSpl_AddSatW16(i, i), i);
    *T0_frac = WebRtcSpl_SubSatW16(WebRtcSpl_SubSatW16(index, 2), i);
  }

  return;
}

