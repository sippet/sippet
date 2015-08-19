/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#ifndef __TAB_LD8A_H__
#define __TAB_LD8A_H__

extern int16_t WebRtcG729fix_hamwindow[L_WINDOW];
extern int16_t WebRtcG729fix_lag_h[M+2];
extern int16_t WebRtcG729fix_lag_l[M+2];
extern int16_t WebRtcG729fix_table[65];
extern int16_t WebRtcG729fix_slope[64];
extern int16_t WebRtcG729fix_table2[64];
extern int16_t WebRtcG729fix_slope_cos[64];
extern int16_t WebRtcG729fix_slope_acos[64];
extern int16_t WebRtcG729fix_lspcb1[NC0][M];
extern int16_t WebRtcG729fix_lspcb2[NC1][M];
extern int16_t WebRtcG729fix_fg[2][MA_NP][M];
extern int16_t WebRtcG729fix_fg_sum[2][M];
extern int16_t WebRtcG729fix_fg_sum_inv[2][M];
extern int16_t WebRtcG729fix_grid[GRID_POINTS+1];
extern int16_t WebRtcG729fix_freq_prev_reset[M];
extern int16_t WebRtcG729fix_inter_3l[FIR_SIZE_SYN];
extern int16_t WebRtcG729fix_pred[4];
extern int16_t WebRtcG729fix_gbk1[NCODE1][2];
extern int16_t WebRtcG729fix_gbk2[NCODE2][2];
extern int16_t WebRtcG729fix_map1[NCODE1];
extern int16_t WebRtcG729fix_map2[NCODE2];
extern int16_t WebRtcG729fix_coef[2][2];
extern int32_t WebRtcG729fix_L_coef[2][2];
extern int16_t WebRtcG729fix_thr1[NCODE1-NCAN1];
extern int16_t WebRtcG729fix_thr2[NCODE2-NCAN2];
extern int16_t WebRtcG729fix_imap1[NCODE1];
extern int16_t WebRtcG729fix_imap2[NCODE2];
extern int16_t WebRtcG729fix_b100[3];
extern int16_t WebRtcG729fix_a100[3];
extern int16_t WebRtcG729fix_b140[3];
extern int16_t WebRtcG729fix_a140[3];
extern int16_t WebRtcG729fix_bitsno[PRM_SIZE];
extern int16_t WebRtcG729fix_bitsno2[4]; 
extern int16_t WebRtcG729fix_tabpow[33];
extern int16_t WebRtcG729fix_tablog[33];
extern int16_t WebRtcG729fix_tabsqr[49];
extern int16_t WebRtcG729fix_tab_zone[PIT_MAX+L_INTERPOL-1];
extern int16_t WebRtcG729fix_lsp_old_reset[M];
extern int16_t WebRtcG729fix_lspSid_reset[M];
extern int16_t WebRtcG729fix_past_qua_en_reset[4];
extern int16_t WebRtcG729fix_old_A_reset[M+1];

#endif /* __TAB_LD8A_H__ */

