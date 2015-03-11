/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

#ifndef __OPER_32B_H__
#define __OPER_32B_H__

/* Double precision operations */

void   WebRtcG729fix_L_Extract(int32_t L_32, int16_t *hi, int16_t *lo);
int32_t WebRtcG729fix_L_Comp(int16_t hi, int16_t lo);
int32_t WebRtcG729fix_Mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2);
int32_t WebRtcG729fix_Mpy_32_16(int16_t hi, int16_t lo, int16_t n);
int32_t WebRtcG729fix_Div_32(int32_t L_num, int16_t denom_hi, int16_t denom_lo);

#endif /* __OPER_32B_H__ */

