// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/****************************************************************************************
Portions of this file are derived from the following ITU standard:
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
****************************************************************************************/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

#pragma pack(1)
struct ld8k_parms {
#ifdef WORDS_BIGENDIAN
  UWord8 LPC_cb1;              // 1st codebook           7 + 1 bit
  UWord8 LPC_cb21;             // 2nd codebook           5 + 5 bit
  UWord8 LPC_cb22 : 2;
                               // --- 1st subframe:
  UWord8 pitch_period1 : 6;    // pitch period                   8 bit
  UWord8 pitch_period2 : 2;
  UWord8 parity_check : 1;     // parity check on 1st period     1 bit
  UWord8 cb_index11 : 5;       // codebook index1(positions)    13 bit
  UWord8 cb_index12;
  UWord8 cb_index2 : 4;        // codebook index2(signs)         4 bit
  UWord8 cb_gains1 : 4;        // pitch and codebook gains   4 + 3 bit
  UWord8 cb_gains2 : 3;
                               // --- 2nd subframe:
  UWord8 rel_pitch_period : 5; // pitch period(relative)         5 bit
  UWord8 pos_cb_index11;       // codebook index1(positions)    13 bit
  UWord8 pos_cb_index12 : 5;
  UWord8 sign_cb_index21 : 3;  // codebook index2(signs)         4 bit
  UWord8 sign_cb_index22 : 1;
  UWord8 pitch_cb_gains : 7;   // pitch and codebook gains   4 + 3 bit
#else
  UWord8 LPC_cb1;
  UWord8 LPC_cb21;
  UWord8 pitch_period1 : 6;
  UWord8 LPC_cb22 : 2;
  UWord8 cb_index11 : 5;
  UWord8 parity_check : 1;
  UWord8 pitch_period2 : 2;
  UWord8 cb_index12;
  UWord8 cb_gains1 : 4;
  UWord8 cb_index2 : 4;
  UWord8 rel_pitch_period : 5;
  UWord8 cb_gains2 : 3;
  UWord8 pos_cb_index11;
  UWord8 sign_cb_index21 : 3;
  UWord8 pos_cb_index12 : 5;
  UWord8 pitch_cb_gains : 7;
  UWord8 sign_cb_index22 : 1;
#endif
};
#pragma pack()

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters are:
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *----------------------------------------------------------------------------
 */
void prm2bits_ld8k(
                    const Word16 prm[],     /* input : encoded parameters  (PRM_SIZE parameters)  */
                    UWord8 *bits            /* output: serial bits (SERIAL_SIZE )*/
)
{
  struct ld8k_parms *prms_map =
    (struct ld8k_parms *)bits;
  prms_map->LPC_cb1 = (UWord8)prm[0];
  prms_map->LPC_cb21 = (UWord8)(prm[1] >> 2);
  prms_map->LPC_cb22 = (UWord8)(prm[1] & ((1 << 2) - 1));
  prms_map->pitch_period1 = (UWord8)(prm[2] >> 2);
  prms_map->pitch_period2 = (UWord8)(prm[2] & ((1 << 2) - 1));
  prms_map->parity_check = (UWord8)prm[3];
  prms_map->cb_index11 = (UWord8)(prm[4] >> 8);
  prms_map->cb_index12 = (UWord8)(prm[4] & ((1 << 8) - 1));
  prms_map->cb_index2 = (UWord8)prm[5];
  prms_map->cb_gains1 = (UWord8)(prm[6] >> 3);
  prms_map->cb_gains2 = (UWord8)(prm[6] & ((1 << 3) - 1));
  prms_map->rel_pitch_period = (UWord8)prm[7];
  prms_map->pos_cb_index11 = (UWord8)(prm[8] >> 5);
  prms_map->pos_cb_index12 = (UWord8)(prm[8] & ((1 << 5) - 1));
  prms_map->sign_cb_index21 = (UWord8)(prm[9] >> 1);
  prms_map->sign_cb_index22 = (UWord8)(prm[9] & ((1 << 1) - 1));
  prms_map->pitch_cb_gains = (UWord8)(prm[10]);
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k(
                   const UWord8  *bits,      /* input : serial bits (80)                       */
                   Word16   prm[]            /* output: decoded parameters (11 parameters)     */
)
{
  const struct ld8k_parms *prms_map =
    (const struct ld8k_parms *)bits;
  prm[0] = prms_map->LPC_cb1;
  prm[1] = (prms_map->LPC_cb21 << 2) | prms_map->LPC_cb22;
  prm[2] = (prms_map->pitch_period1 << 2) | prms_map->pitch_period2;
  prm[3] = prms_map->parity_check;
  prm[4] = (prms_map->cb_index11 << 8) | prms_map->cb_index12;
  prm[5] = prms_map->cb_index2;
  prm[6] = (prms_map->cb_gains1 << 3) | prms_map->cb_gains2;
  prm[7] = prms_map->rel_pitch_period;
  prm[8] = (prms_map->pos_cb_index11 << 5) | prms_map->pos_cb_index12;
  prm[9] = (prms_map->sign_cb_index21 << 1) | prms_map->sign_cb_index22;
  prm[10] = prms_map->pitch_cb_gains;
}

