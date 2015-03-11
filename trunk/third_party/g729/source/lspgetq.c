/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

#include <stdio.h>
#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"


void WebRtcG729fix_Lsp_get_quant(
  int16_t lspcb1[][M],      /* (i) Q13 : first stage LSP codebook      */
  int16_t lspcb2[][M],      /* (i) Q13 : Second stage LSP codebook     */
  int16_t code0,            /* (i)     : selected code of first stage  */
  int16_t code1,            /* (i)     : selected code of second stage */
  int16_t code2,            /* (i)     : selected code of second stage */
  int16_t fg[][M],          /* (i) Q15 : MA prediction coef.           */
  int16_t freq_prev[][M],   /* (i) Q13 : previous LSP vector           */
  int16_t lspq[],           /* (o) Q13 : quantized LSP parameters      */
  int16_t fg_sum[]          /* (i) Q15 : present MA prediction coef.   */
)
{
  int16_t j;
  int16_t buf[M];           /* Q13 */


  for ( j = 0 ; j < NC ; j++ )
    buf[j] = WebRtcSpl_AddSatW16( lspcb1[code0][j], lspcb2[code1][j] );

  for ( j = NC ; j < M ; j++ )
    buf[j] = WebRtcSpl_AddSatW16( lspcb1[code0][j], lspcb2[code2][j] );

  WebRtcG729fix_Lsp_expand_1_2(buf, GAP1);
  WebRtcG729fix_Lsp_expand_1_2(buf, GAP2);

  WebRtcG729fix_Lsp_prev_compose(buf, lspq, fg, freq_prev, fg_sum);

  WebRtcG729fix_Lsp_prev_update(buf, freq_prev);

  WebRtcG729fix_Lsp_stability( lspq );

  return;
}


void WebRtcG729fix_Lsp_expand_1(
  int16_t buf[],        /* (i/o) Q13 : LSP vectors */
  int16_t gap           /* (i)   Q13 : gap         */
)
{
  int16_t j, tmp;
  int16_t diff;        /* Q13 */

  for ( j = 1 ; j < NC ; j++ ) {
    diff = WebRtcSpl_SubSatW16( buf[j-1], buf[j] );
    tmp = shr( WebRtcSpl_AddSatW16( diff, gap), 1 );

    if ( tmp >  0 ) {
      buf[j-1] = WebRtcSpl_SubSatW16( buf[j-1], tmp );
      buf[j]   = WebRtcSpl_AddSatW16( buf[j], tmp );
    }
  }
  return;
}


void WebRtcG729fix_Lsp_expand_2(
  int16_t buf[],       /* (i/o) Q13 : LSP vectors */
  int16_t gap          /* (i)   Q13 : gap         */
)
{
  int16_t j, tmp;
  int16_t diff;        /* Q13 */

  for ( j = NC ; j < M ; j++ ) {
    diff = WebRtcSpl_SubSatW16( buf[j-1], buf[j] );
    tmp = shr( WebRtcSpl_AddSatW16( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = WebRtcSpl_SubSatW16( buf[j-1], tmp );
      buf[j]   = WebRtcSpl_AddSatW16( buf[j], tmp );
    }
  }
  return;
}


void WebRtcG729fix_Lsp_expand_1_2(
  int16_t buf[],       /* (i/o) Q13 : LSP vectors */
  int16_t gap          /* (i)   Q13 : gap         */
)
{
  int16_t j, tmp;
  int16_t diff;        /* Q13 */

  for ( j = 1 ; j < M ; j++ ) {
    diff = WebRtcSpl_SubSatW16( buf[j-1], buf[j] );
    tmp = shr( WebRtcSpl_AddSatW16( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = WebRtcSpl_SubSatW16( buf[j-1], tmp );
      buf[j]   = WebRtcSpl_AddSatW16( buf[j], tmp );
    }
  }
  return;
}


/*
  Functions which use previous LSP parameter (freq_prev).
*/


/*
  compose LSP parameter from elementary LSP with previous LSP.
*/
void WebRtcG729fix_Lsp_prev_compose(
  int16_t lsp_ele[],             /* (i) Q13 : LSP vectors                 */
  int16_t lsp[],                 /* (o) Q13 : quantized LSP parameters    */
  int16_t fg[][M],               /* (i) Q15 : MA prediction coef.         */
  int16_t freq_prev[][M],        /* (i) Q13 : previous LSP vector         */
  int16_t fg_sum[]               /* (i) Q15 : present MA prediction coef. */
)
{
  int16_t j, k;
  int32_t L_acc;                 /* Q29 */

  for ( j = 0 ; j < M ; j++ ) {
    L_acc = L_mult( lsp_ele[j], fg_sum[j] );
    for ( k = 0 ; k < MA_NP ; k++ )
      L_acc = L_mac( L_acc, freq_prev[k][j], fg[k][j] );

    lsp[j] = extract_h(L_acc);
  }
  return;
}


/*
  extract elementary LSP from composed LSP with previous LSP
*/
void WebRtcG729fix_Lsp_prev_extract(
  int16_t lsp[M],                /* (i) Q13 : unquantized LSP parameters  */
  int16_t lsp_ele[M],            /* (o) Q13 : target vector               */
  int16_t fg[MA_NP][M],          /* (i) Q15 : MA prediction coef.         */
  int16_t freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector         */
  int16_t fg_sum_inv[M]          /* (i) Q12 : inverse previous LSP vector */
)
{
  int16_t j, k;
  int32_t L_temp;                /* Q19 */
  int16_t temp;                  /* Q13 */


  for ( j = 0 ; j < M ; j++ ) {
    L_temp = L_deposit_h(lsp[j]);
    for ( k = 0 ; k < MA_NP ; k++ )
      L_temp = L_msu( L_temp, freq_prev[k][j], fg[k][j] );

    temp = extract_h(L_temp);
    L_temp = L_mult( temp, fg_sum_inv[j] );
    lsp_ele[j] = extract_h( L_shl( L_temp, 3 ) );

  }
  return;
}


/*
  update previous LSP parameter
*/
void WebRtcG729fix_Lsp_prev_update(
  int16_t lsp_ele[M],             /* (i)   Q13 : LSP vectors           */
  int16_t freq_prev[MA_NP][M]     /* (i/o) Q13 : previous LSP vectors  */
)
{
  int16_t k;

  for ( k = MA_NP-1 ; k > 0 ; k-- )
    WEBRTC_SPL_MEMCPY_W16(freq_prev[k], freq_prev[k-1], M);

  WEBRTC_SPL_MEMCPY_W16(freq_prev[0], lsp_ele, M);
  return;
}

void WebRtcG729fix_Lsp_stability(
  int16_t buf[]       /* (i/o) Q13 : quantized LSP parameters      */
)
{
  int16_t j;
  int16_t tmp;
  int32_t L_diff;
  int32_t L_acc, L_accb;

  for(j=0; j<M-1; j++) {
    L_acc = L_deposit_l( buf[j+1] );
    L_accb = L_deposit_l( buf[j] );
    L_diff = WebRtcSpl_SubSatW32( L_acc, L_accb );

    if( L_diff < 0L ) {
      /* exchange buf[j]<->buf[j+1] */
      tmp      = buf[j+1];
      buf[j+1] = buf[j];
      buf[j]   = tmp;
    }
  }


  if(buf[0] < L_LIMIT) {
    buf[0] = L_LIMIT;
    printf("lsp_stability warning Low \n");
  }
  for(j=0; j<M-1; j++) {
    L_acc = L_deposit_l( buf[j+1] );
    L_accb = L_deposit_l( buf[j] );
    L_diff = WebRtcSpl_SubSatW32( L_acc, L_accb );

    if( WebRtcSpl_SubSatW32(L_diff, GAP3)<0L ) {
      buf[j+1] = WebRtcSpl_AddSatW16( buf[j], GAP3 );
    }
  }

  if(buf[M-1] > M_LIMIT) {
    buf[M-1] = M_LIMIT;
    printf("lsp_stability warning High \n");
  }
  return;
}
