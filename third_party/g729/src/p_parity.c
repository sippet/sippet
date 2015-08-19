/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*------------------------------------------------------*
 * Parity_pitch - compute parity bit for first 6 MSBs   *
 *------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

int16_t WebRtcG729fix_Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
   int16_t pitch_index   /* input : index for which parity to compute */
)
{
  int16_t temp, sum, i, bit;

  temp = shr(pitch_index, 1);

  sum = 1;
  for (i = 0; i <= 5; i++) {
    temp = shr(temp, 1);
    bit = temp & (int16_t)1;
    sum = WebRtcSpl_AddSatW16(sum, bit);
  }
  sum = sum & (int16_t)1;


  return sum;
}

/*--------------------------------------------------------------------*
 * check_parity_pitch - check parity of index with transmitted parity *
 *--------------------------------------------------------------------*/

int16_t  WebRtcG729fix_Check_Parity_Pitch( /* output: 0 = no error, 1= error */
  int16_t pitch_index,       /* input : index of parameter     */
  int16_t parity             /* input : parity bit             */
)
{
  int16_t temp, sum, i, bit;

  temp = shr(pitch_index, 1);

  sum = 1;
  for (i = 0; i <= 5; i++) {
    temp = shr(temp, 1);
    bit = temp & (int16_t)1;
    sum = WebRtcSpl_AddSatW16(sum, bit);
  }
  sum = WebRtcSpl_AddSatW16(sum, parity);
  sum = sum & (int16_t)1;

  return sum;
}
