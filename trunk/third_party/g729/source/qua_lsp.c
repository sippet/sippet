/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   Rockwell International
   All rights reserved.
*/

/*-------------------------------------------------------------------*
 * Function  Qua_lsp:                                                *
 *           ~~~~~~~~                                                *
 *-------------------------------------------------------------------*/
#include <stdint.h>
#include "basic_op.h"

#include "ld8a.h"
#include "tab_ld8a.h"

void Qua_lsp(
  Coder_ld8a_state *st,
  int16_t lsp[],       /* (i) Q15 : Unquantized LSP            */
  int16_t lsp_q[],     /* (o) Q15 : Quantized LSP              */
  int16_t ana[]        /* (o)     : indexes                    */
)
{
  int16_t lsf[M], lsf_q[M];  /* domain 0.0<= lsf <PI in Q13 */

  /* Convert LSPs to LSFs */
  Lsp_lsf2(lsp, lsf, M);

  Lsp_qua_cs(st, lsf, lsf_q, ana );

  /* Convert LSFs to LSPs */
  Lsf_lsp2(lsf_q, lsp_q, M);

  return;
}

void Lsp_encw_reset(Coder_ld8a_state *st)
{
  int16_t i;

  for(i=0; i<MA_NP; i++)
    Copy(&freq_prev_reset[0], &st->freq_prev[i][0], M);
}


void Lsp_qua_cs(
  Coder_ld8a_state *st,
  int16_t flsp_in[M],    /* (i) Q13 : Original LSP parameters    */
  int16_t lspq_out[M],   /* (o) Q13 : Quantized LSP parameters   */
  int16_t *code          /* (o)     : codes of the selected LSP  */
)
{
  int16_t wegt[M];       /* Q11->normalized : weighting coefficients */

  Get_wegt( flsp_in, wegt );

  Relspwed( flsp_in, wegt, lspq_out, lspcb1, lspcb2, fg,
    st->freq_prev, fg_sum, fg_sum_inv, code);
}

void Relspwed(
  int16_t lsp[],                 /* (i) Q13 : unquantized LSP parameters */
  int16_t wegt[],                /* (i) norm: weighting coefficients     */
  int16_t lspq[],                /* (o) Q13 : quantized LSP parameters   */
  int16_t lspcb1[][M],           /* (i) Q13 : first stage LSP codebook   */
  int16_t lspcb2[][M],           /* (i) Q13 : Second stage LSP codebook  */
  int16_t fg[MODE][MA_NP][M],    /* (i) Q15 : MA prediction coefficients */
  int16_t freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector        */
  int16_t fg_sum[MODE][M],       /* (i) Q15 : present MA prediction coef.*/
  int16_t fg_sum_inv[MODE][M],   /* (i) Q12 : inverse coef.              */
  int16_t code_ana[]             /* (o)     : codes of the selected LSP  */
)
{
  int16_t mode, j;
  int16_t index, mode_index;
  int16_t cand[MODE], cand_cur;
  int16_t tindex1[MODE], tindex2[MODE];
  int32_t L_tdist[MODE];         /* Q26 */
  int16_t rbuf[M];               /* Q13 */
  int16_t buf[M];                /* Q13 */

  for(mode = 0; mode<MODE; mode++) {
    Lsp_prev_extract(lsp, rbuf, fg[mode], freq_prev, fg_sum_inv[mode]);

    Lsp_pre_select(rbuf, lspcb1, &cand_cur );
    cand[mode] = cand_cur;

    Lsp_select_1(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);

    tindex1[mode] = index;

    for( j = 0 ; j < NC ; j++ )
      buf[j] = add( lspcb1[cand_cur][j], lspcb2[index][j] );

    Lsp_expand_1(buf, GAP1);

    Lsp_select_2(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);

    tindex2[mode] = index;

    for( j = NC ; j < M ; j++ )
      buf[j] = add( lspcb1[cand_cur][j], lspcb2[index][j] );

    Lsp_expand_2(buf, GAP1);

    Lsp_expand_1_2(buf, GAP2);

    Lsp_get_tdist(wegt, buf, &L_tdist[mode], rbuf, fg_sum[mode]);
  }

  Lsp_last_select(L_tdist, &mode_index);

  code_ana[0] = shl( mode_index,NC0_B ) | cand[mode_index];
  code_ana[1] = shl( tindex1[mode_index],NC1_B ) | tindex2[mode_index];

  Lsp_get_quant(lspcb1, lspcb2, cand[mode_index],
      tindex1[mode_index], tindex2[mode_index],
      fg[mode_index], freq_prev, lspq, fg_sum[mode_index]) ;

  return;
}


void Lsp_pre_select(
  int16_t rbuf[],              /* (i) Q13 : target vetor             */
  int16_t lspcb1[][M],         /* (i) Q13 : first stage LSP codebook */
  int16_t *cand                /* (o)     : selected code            */
)
{
  int16_t i, j;
  int16_t tmp;                 /* Q13 */
  int32_t L_dmin;              /* Q26 */
  int32_t L_tmp;               /* Q26 */
  int32_t L_temp;

  /* avoid the worst case. (all over flow) */

  *cand = 0;
  L_dmin = MAX_32;
  for ( i = 0 ; i < NC0 ; i++ ) {
    L_tmp = 0;
    for ( j = 0 ; j < M ; j++ ) {
      tmp = sub(rbuf[j], lspcb1[i][j]);
      L_tmp = L_mac( L_tmp, tmp, tmp );
    }

    L_temp = L_sub(L_tmp,L_dmin);
    if (  L_temp< 0L) {
      L_dmin = L_tmp;
      *cand = i;
    }
  }
  return;
}



void Lsp_select_1(
  int16_t rbuf[],              /* (i) Q13 : target vector             */
  int16_t lspcb1[],            /* (i) Q13 : first stage lsp codebook  */
  int16_t wegt[],              /* (i) norm: weighting coefficients    */
  int16_t lspcb2[][M],         /* (i) Q13 : second stage lsp codebook */
  int16_t *index               /* (o)     : selected codebook index   */
)
{
  int16_t j, k1;
  int16_t buf[M];              /* Q13 */
  int32_t L_dist;              /* Q26 */
  int32_t L_dmin;              /* Q26 */
  int16_t tmp,tmp2;            /* Q13 */
  int32_t L_temp;

  for ( j = 0 ; j < NC ; j++ )
    buf[j] = sub(rbuf[j], lspcb1[j]);

                   /* avoid the worst case. (all over flow) */
  *index = 0;
  L_dmin = MAX_32;
  for ( k1 = 0 ; k1 < NC1 ; k1++ ) {
    L_dist = 0;
    for ( j = 0 ; j < NC ; j++ ) {
      tmp = sub(buf[j], lspcb2[k1][j]);
      tmp2 = mult( wegt[j], tmp );
      L_dist = L_mac( L_dist, tmp2, tmp );
    }

    L_temp =L_sub(L_dist,L_dmin);
    if ( L_temp <0L ) {
      L_dmin = L_dist;
      *index = k1;
    }
  }
  return;
}



void Lsp_select_2(
  int16_t rbuf[],              /* (i) Q13 : target vector             */
  int16_t lspcb1[],            /* (i) Q13 : first stage lsp codebook  */
  int16_t wegt[],              /* (i) norm: weighting coef.           */
  int16_t lspcb2[][M],         /* (i) Q13 : second stage lsp codebook */
  int16_t *index               /* (o)     : selected codebook index   */
)
{
  int16_t j, k1;
  int16_t buf[M];              /* Q13 */
  int32_t L_dist;              /* Q26 */
  int32_t L_dmin;              /* Q26 */
  int16_t tmp,tmp2;            /* Q13 */
  int32_t L_temp;

  for ( j = NC ; j < M ; j++ )
    buf[j] = sub(rbuf[j], lspcb1[j]);

                            /* avoid the worst case. (all over flow) */
  *index = 0;
  L_dmin = MAX_32;
  for ( k1 = 0 ; k1 < NC1 ; k1++ ) {
    L_dist = 0;
    for ( j = NC ; j < M ; j++ ) {
      tmp = sub(buf[j], lspcb2[k1][j]);
      tmp2 = mult( wegt[j], tmp );
      L_dist = L_mac( L_dist, tmp2, tmp );
    }

    L_temp = L_sub(L_dist, L_dmin);
    if ( L_temp <0L ) {
      L_dmin = L_dist;
      *index = k1;
    }
  }
  return;
}



void Lsp_get_tdist(
  int16_t wegt[],        /* (i) norm: weight coef.                */
  int16_t buf[],         /* (i) Q13 : candidate LSP vector        */
  int32_t *L_tdist,      /* (o) Q27 : distortion                  */
  int16_t rbuf[],        /* (i) Q13 : target vector               */
  int16_t fg_sum[]       /* (i) Q15 : present MA prediction coef. */
)
{
  int16_t j;
  int16_t tmp, tmp2;     /* Q13 */
  int32_t L_acc;         /* Q25 */

  *L_tdist = 0;
  for ( j = 0 ; j < M ; j++ ) {
    /* tmp = (buf - rbuf)*fg_sum */
    tmp = sub( buf[j], rbuf[j] );
    tmp = mult( tmp, fg_sum[j] );

    /* *L_tdist += wegt * tmp * tmp */
    L_acc = L_mult( wegt[j], tmp );
    tmp2 = extract_h( L_shl( L_acc, 4 ) );
    *L_tdist = L_mac( *L_tdist, tmp2, tmp );
  }

  return;
}



void Lsp_last_select(
  int32_t L_tdist[],     /* (i) Q27 : distortion         */
  int16_t *mode_index    /* (o)     : the selected mode  */
)
{
    int32_t L_temp;
  *mode_index = 0;
  L_temp =L_sub(L_tdist[1] ,L_tdist[0]);
  if (  L_temp<0L){
    *mode_index = 1;
  }
  return;
}

void Get_wegt(
  int16_t flsp[],    /* (i) Q13 : M LSP parameters  */
  int16_t wegt[]     /* (o) Q11->norm : M weighting coefficients */
)
{
  int16_t i;
  int16_t tmp;
  int32_t L_acc;
  int16_t sft;
  int16_t buf[M]; /* in Q13 */


  buf[0] = sub( flsp[1], (PI04+8192) );           /* 8192:1.0(Q13) */

  for ( i = 1 ; i < M-1 ; i++ ) {
    tmp = sub( flsp[i+1], flsp[i-1] );
    buf[i] = sub( tmp, 8192 );
  }

  buf[M-1] = sub( (PI92-8192), flsp[M-2] );

  /* */
  for ( i = 0 ; i < M ; i++ ) {
    if ( buf[i] > 0 ){
      wegt[i] = 2048;                    /* 2048:1.0(Q11) */
    }
    else {
      L_acc = L_mult( buf[i], buf[i] );           /* L_acc in Q27 */
      tmp = extract_h( L_shl( L_acc, 2 ) );       /* tmp in Q13 */

      L_acc = L_mult( tmp, CONST10 );             /* L_acc in Q25 */
      tmp = extract_h( L_shl( L_acc, 2 ) );       /* tmp in Q11 */

      wegt[i] = add( tmp, 2048 );                 /* wegt in Q11 */
    }
  }

  /* */
  L_acc = L_mult( wegt[4], CONST12 );             /* L_acc in Q26 */
  wegt[4] = extract_h( L_shl( L_acc, 1 ) );       /* wegt in Q11 */

  L_acc = L_mult( wegt[5], CONST12 );             /* L_acc in Q26 */
  wegt[5] = extract_h( L_shl( L_acc, 1 ) );       /* wegt in Q11 */

  /* wegt: Q11 -> normalized */
  tmp = 0;
  for ( i = 0; i < M; i++ ) {
    if (wegt[i] > tmp) {
      tmp = wegt[i];
    }
  }

  sft = norm_s(tmp);
  for ( i = 0; i < M; i++ ) {
    wegt[i] = shl(wegt[i], sft);                  /* wegt in Q(11+sft) */
  }

  return;
}


void Get_freq_prev(Coder_ld8a_state *st, int16_t x[MA_NP][M])
{
  int16_t i;

  for (i=0; i<MA_NP; i++)
    Copy(&st->freq_prev[i][0], &x[i][0], M);
}
  
void Update_freq_prev(Coder_ld8a_state *st, int16_t x[MA_NP][M])
{
  int16_t i;

  for (i=0; i<MA_NP; i++)
    Copy(&x[i][0], &st->freq_prev[i][0], M);
}
  



