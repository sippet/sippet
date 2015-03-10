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

extern int16_t hamwindow[L_WINDOW];
extern int16_t lag_h[M+2];
extern int16_t lag_l[M+2];
extern int16_t table[65];
extern int16_t slope[64];
extern int16_t table2[64];
extern int16_t slope_cos[64];
extern int16_t slope_acos[64];
extern int16_t lspcb1[NC0][M];
extern int16_t lspcb2[NC1][M];
extern int16_t fg[2][MA_NP][M];
extern int16_t fg_sum[2][M];
extern int16_t fg_sum_inv[2][M];
extern int16_t grid[GRID_POINTS+1];
extern int16_t freq_prev_reset[M];
extern int16_t inter_3l[FIR_SIZE_SYN];
extern int16_t pred[4];
extern int16_t gbk1[NCODE1][2];
extern int16_t gbk2[NCODE2][2];
extern int16_t map1[NCODE1];
extern int16_t map2[NCODE2];
extern int16_t coef[2][2];
extern int32_t L_coef[2][2];
extern int16_t thr1[NCODE1-NCAN1];
extern int16_t thr2[NCODE2-NCAN2];
extern int16_t imap1[NCODE1];
extern int16_t imap2[NCODE2];
extern int16_t b100[3];
extern int16_t a100[3];
extern int16_t b140[3];
extern int16_t a140[3];
extern int16_t bitsno[PRM_SIZE];
extern int16_t bitsno2[4]; 
extern int16_t tabpow[33];
extern int16_t tablog[33];
extern int16_t tabsqr[49];
extern int16_t tab_zone[PIT_MAX+L_INTERPOL-1];
extern int16_t lsp_old_reset[M];
extern int16_t lspSid_reset[M];
extern int16_t past_qua_en_reset[4];
extern int16_t old_A_reset[M+1];

#endif /* __TAB_LD8A_H__ */

