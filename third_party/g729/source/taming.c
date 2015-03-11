/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/**************************************************************************
 * Taming functions.                                                      *
 **************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "tab_ld8a.h"

void WebRtcG729fix_Init_exc_err(int32_t L_exc_err[])
{
  int16_t i;
  for(i=0; i<4; i++) L_exc_err[i] = 0x00004000L;   /* Q14 */
}

/**************************************************************************
 * routine test_err - computes the accumulated potential error in the     *
 * adaptive codebook contribution                                         *
 **************************************************************************/

int16_t WebRtcG729fix_test_err(  /* (o) flag set to 1 if taming is necessary  */
 int32_t L_exc_err[],
 int16_t T0,       /* (i) integer part of pitch delay           */
 int16_t T0_frac   /* (i) fractional part of pitch delay        */
)
 {
    int16_t i, t1, zone1, zone2, flag;
    int32_t L_maxloc, L_acc;

    if(T0_frac > 0) {
        t1 = WebRtcSpl_AddSatW16(T0, 1);
    }
    else {
        t1 = T0;
    }

    i = WebRtcSpl_SubSatW16(t1, (L_SUBFR+L_INTER10));
    if(i < 0) {
        i = 0;
    }
    zone1 = WebRtcG729fix_tab_zone[i];

    i = WebRtcSpl_AddSatW16(t1, (L_INTER10 - 2));
    zone2 = WebRtcG729fix_tab_zone[i];

    L_maxloc = -1L;
    flag = 0 ;
    for(i=zone2; i>=zone1; i--) {
        L_acc = WebRtcSpl_SubSatW32(L_exc_err[i], L_maxloc);
        if(L_acc > 0L) {
                L_maxloc = L_exc_err[i];
        }
    }
    L_acc = WebRtcSpl_SubSatW32(L_maxloc, L_THRESH_ERR);
    if(L_acc > 0L) {
        flag = 1;
    }

    return(flag);
}

/**************************************************************************
 *routine update_exc_err - maintains the memory used to compute the error *
 * function due to an adaptive codebook mismatch between encoder and      *
 * decoder                                                                *
 **************************************************************************/

void WebRtcG729fix_update_exc_err(
 int32_t L_exc_err[],
 int16_t gain_pit,      /* (i) pitch gain */
 int16_t T0             /* (i) integer part of pitch delay */
)
 {

    int16_t i, zone1, zone2, n;
    int32_t L_worst, L_temp, L_acc;
    int16_t hi, lo;

    L_worst = -1L;
    n = WebRtcSpl_SubSatW16(T0, L_SUBFR);

    if(n < 0) {
        WebRtcG729fix_L_Extract(L_exc_err[0], &hi, &lo);
        L_temp = WebRtcG729fix_Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = WebRtcSpl_AddSatW32(0x00004000L, L_temp);
        L_acc = WebRtcSpl_SubSatW32(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
        WebRtcG729fix_L_Extract(L_temp, &hi, &lo);
        L_temp = WebRtcG729fix_Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = WebRtcSpl_AddSatW32(0x00004000L, L_temp);
        L_acc = WebRtcSpl_SubSatW32(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
    }

    else {

        zone1 = WebRtcG729fix_tab_zone[n];

        i = WebRtcSpl_SubSatW16(T0, 1);
        zone2 = WebRtcG729fix_tab_zone[i];

        for(i = zone1; i <= zone2; i++) {
                WebRtcG729fix_L_Extract(L_exc_err[i], &hi, &lo);
                L_temp = WebRtcG729fix_Mpy_32_16(hi, lo, gain_pit);
                L_temp = L_shl(L_temp, 1);
                L_temp = WebRtcSpl_AddSatW32(0x00004000L, L_temp);
                L_acc = WebRtcSpl_SubSatW32(L_temp, L_worst);
                if(L_acc > 0L) L_worst = L_temp;
        }
    }

    for(i=3; i>=1; i--) {
        L_exc_err[i] = L_exc_err[i-1];
    }
    L_exc_err[0] = L_worst;

    return;
}

