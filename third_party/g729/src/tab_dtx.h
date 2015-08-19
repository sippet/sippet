/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#ifndef __TAB_DTX_H__
#define __TAB_DTX_H__

/* VAD constants */
extern int16_t WebRtcG729fix_lbf_corr[NP+1];
extern int16_t WebRtcG729fix_shift_fx[33];
extern int16_t WebRtcG729fix_factor_fx[33];

/* SID LSF quantization */
extern int16_t WebRtcG729fix_noise_fg_sum[MODE][M];
extern int16_t WebRtcG729fix_noise_fg_sum_inv[MODE][M];
extern int16_t WebRtcG729fix_PtrTab_1[32];
extern int16_t WebRtcG729fix_PtrTab_2[2][16];
extern int16_t WebRtcG729fix_Mp[MODE];

/* SID gain quantization */
extern int16_t WebRtcG729fix_fact[NB_GAIN+1];
extern int16_t WebRtcG729fix_marg[NB_GAIN+1];
extern int16_t WebRtcG729fix_tab_Sidgain[32];

#endif /* __TAB_DTX_H__ */

