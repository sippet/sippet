/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.5    Last modified: October 2006
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/


#include <stdio.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "sid.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* local functions */
static void Qnt_e(int16_t *errlsf,    /* (i)  : error lsf vector             */
                  int16_t *weight,    /* (i)  : weighting vector             */
                  int16_t DIn,        /* (i)  : number of input candidates   */
                  int16_t *qlsf,      /* (o)  : quantized error lsf vector   */
                  int16_t *Pptr,      /* (o)  : predictor index              */
                  int16_t DOut,       /* (i)  : number of quantized vectors  */
                  int16_t *cluster,   /* (o)  : quantizer indices            */
                  int16_t *MS         /* (i)  : size of the quantizers       */
                  );

static void New_ML_search_1(int16_t *d_data,    /* (i) : error vector             */
                            int16_t J,          /* (i) : number of input vectors  */
                            int16_t *new_d_data,/* (o) : output vector            */
                            int16_t K,          /* (i) : number of candidates     */
                            int16_t *best_indx, /* (o) : best indices             */
                            int16_t *ptr_back,  /* (o) : pointer for backtracking */
                            int16_t *PtrTab,    /* (i) : quantizer table          */
                            int16_t MQ          /* (i) : size of quantizer        */
                            );

static void New_ML_search_2(int16_t *d_data,    /* (i) : error vector             */
                            int16_t *weight,    /* (i) : weighting vector         */
                            int16_t J,          /* (i) : number of input vectors  */
                            int16_t *new_d_data,/* (o) : output vector            */
                            int16_t K,          /* (i) : number of candidates     */
                            int16_t *best_indx, /* (o) : best indices             */
                            int16_t *ptr_prd,   /* (i) : pointer for backtracking */
                            int16_t *ptr_back,  /* (o) : pointer for backtracking */
                            int16_t PtrTab[2][16],/* (i) : quantizer table        */
                            int16_t MQ          /* (i) : size of quantizer        */
                            );


/*-----------------------------------------------------------------*
 * Functions lsfq_noise                                            *
 *           ~~~~~~~~~~                                            *
 * Input:                                                          *
 *   lsp[]         : unquantized lsp vector                        *
 *   freq_prev[][] : memory of the lsf predictor                   *
 *                                                                 *
 * Output:                                                         *
 *                                                                 *
 *   lspq[]        : quantized lsp vector                          * 
 *   ana[]         : indices                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/
void WebRtcG729fix_lsfq_noise(int16_t noise_fg[MODE][MA_NP][M],
                int16_t *lsp,
                int16_t *lspq,
                int16_t freq_prev[MA_NP][M],
                int16_t *ana
                )
{
  int16_t i, lsf[M], lsfq[M], weight[M], tmpbuf[M];
  int16_t MS[MODE]={32, 16}, Clust[MODE], mode, errlsf[M*MODE];

  /* convert lsp to lsf */
  WebRtcG729fix_Lsp_lsf2(lsp, lsf, M);

  /* spacing to ~100Hz */
  if (lsf[0] < L_LIMIT)
    lsf[0] = L_LIMIT;
  for (i=0 ; i < M-1 ; i++)
    if (WebRtcSpl_SubSatW16(lsf[i+1], lsf[i]) < 2*GAP3) 
      lsf[i+1] = WebRtcSpl_AddSatW16(lsf[i], 2*GAP3);
  if (lsf[M-1] > M_LIMIT)
    lsf[M-1] = M_LIMIT;
  if (lsf[M-1] < lsf[M-2]) 
    lsf[M-2] = WebRtcSpl_SubSatW16(lsf[M-1], GAP3);

  /* get the lsf weighting */
  WebRtcG729fix_Get_wegt(lsf, weight);
  
  /**********************/
  /* quantize the lsf's */
  /**********************/
  
  /* get the prediction error vector */
  for (mode=0; mode<MODE; mode++) {
    WebRtcG729fix_Lsp_prev_extract(lsf, errlsf+mode*M, noise_fg[mode],
        freq_prev, WebRtcG729fix_noise_fg_sum_inv[mode]);
  }

  /* quantize the lsf and get the corresponding indices */
  Qnt_e(errlsf, weight, MODE, tmpbuf, &mode, 1, Clust, MS);
  ana[0] = mode;
  ana[1] = Clust[0];
  ana[2] = Clust[1];

  /* guarantee minimum distance of 0.0012 (~10 in Q13) between tmpbuf[j]
     and tmpbuf[j+1] */
  WebRtcG729fix_Lsp_expand_1_2(tmpbuf, 10);

  /* compute the quantized lsf vector */
  WebRtcG729fix_Lsp_prev_compose(tmpbuf, lsfq, noise_fg[mode], freq_prev, 
                   WebRtcG729fix_noise_fg_sum[mode]);
  
  /* update the prediction memory */
  WebRtcG729fix_Lsp_prev_update(tmpbuf, freq_prev);
  
  /* lsf stability check */
  WebRtcG729fix_Lsp_stability(lsfq);

  /* convert lsf to lsp */
  WebRtcG729fix_Lsf_lsp2(lsfq, lspq, M);

}

static void Qnt_e(int16_t *errlsf,    /* (i)  : error lsf vector             */
                  int16_t *weight,    /* (i)  : weighting vector             */
                  int16_t DIn,        /* (i)  : number of input candidates   */
                  int16_t *qlsf,      /* (o)  : quantized error lsf vector   */
                  int16_t *Pptr,      /* (o)  : predictor index              */
                  int16_t DOut,       /* (i)  : number of quantized vectors  */
                  int16_t *cluster,   /* (o)  : quantizer indices            */
                  int16_t *MS         /* (i)  : size of the quantizers       */
)
{
  int16_t d_data[2][R_LSFQ*M], best_indx[2][R_LSFQ];
  int16_t ptr_back[2][R_LSFQ], ptr, i;

  New_ML_search_1(errlsf, DIn, d_data[0], 4, best_indx[0], ptr_back[0], 
                  WebRtcG729fix_PtrTab_1, MS[0]);
  New_ML_search_2(d_data[0], weight, 4, d_data[1], DOut, best_indx[1], 
                  ptr_back[0], ptr_back[1], WebRtcG729fix_PtrTab_2, MS[1]);
  
  /* backward path for the indices */
  cluster[1] = best_indx[1][0];
  ptr = ptr_back[1][0];
  cluster[0] = best_indx[0][ptr];

  /* this is the pointer to the best predictor */
  *Pptr = ptr_back[0][ptr];
  
  /* generating the quantized vector */
  WEBRTC_SPL_MEMCPY_W16(qlsf, WebRtcG729fix_lspcb1[WebRtcG729fix_PtrTab_1[cluster[0]]], M);
  for (i=0; i<M/2; i++) {
    qlsf[i] = WebRtcSpl_AddSatW16(qlsf[i],
        WebRtcG729fix_lspcb2[WebRtcG729fix_PtrTab_2[0][cluster[1]]][i]);
  }
  for (i=M/2; i<M; i++) {
    qlsf[i] = WebRtcSpl_AddSatW16(qlsf[i],
        WebRtcG729fix_lspcb2[WebRtcG729fix_PtrTab_2[1][cluster[1]]][i]);
  }
}

static void New_ML_search_1(int16_t *d_data,    /* (i) : error vector             */
                            int16_t J,          /* (i) : number of input vectors  */
                            int16_t *new_d_data,/* (o) : output vector            */
                            int16_t K,          /* (i) : number of candidates     */
                            int16_t *best_indx, /* (o) : best indices             */
                            int16_t *ptr_back,  /* (o) : pointer for backtracking */
                            int16_t *PtrTab,    /* (i) : quantizer table          */
                            int16_t MQ          /* (i) : size of quantizer        */
)
{
  int16_t tmp, m, l, p, q, sum[R_LSFQ*R_LSFQ];
  int16_t min[R_LSFQ], min_indx_p[R_LSFQ], min_indx_m[R_LSFQ];
  int32_t acc0;

  for (q=0; q<K; q++)
    min[q] = WEBRTC_SPL_WORD16_MAX;

  /* compute the errors */
  for (p=0; p<J; p++)
    for (m=0; m<MQ; m++){
      acc0 = 0;
      for (l=0; l<M; l++){
        tmp = WebRtcSpl_SubSatW16(d_data[p*M+l],
                                  WebRtcG729fix_lspcb1[PtrTab[m]][l]);
        acc0 = L_mac(acc0, tmp, tmp);
      }
      sum[p*MQ+m] = extract_h(acc0);
      sum[p*MQ+m] = mult(sum[p*MQ+m], WebRtcG729fix_Mp[p]);
    }
      
  /* select the candidates */
  for (q=0; q<K; q++){
    min_indx_p[q] = 0;  /* G.729 maintenance */
    min_indx_m[q] = 0;  /* G.729 maintenance */
    for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sum[p*MQ+m] < min[q]){
          min[q] = sum[p*MQ+m];
          min_indx_p[q] = p;
          min_indx_m[q] = m;
        }
    
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = WEBRTC_SPL_WORD16_MAX;
  }

  /* compute the candidates */
  for (q=0; q<K; q++){
    for (l=0; l<M; l++) {
      new_d_data[q*M+l] = WebRtcSpl_SubSatW16(d_data[min_indx_p[q]*M+l], 
          WebRtcG729fix_lspcb1[PtrTab[min_indx_m[q]]][l]);
    }
    
    ptr_back[q] = min_indx_p[q];
    best_indx[q] = min_indx_m[q];
  }
}

static void New_ML_search_2(int16_t *d_data,    /* (i) : error vector             */
                            int16_t *weight,    /* (i) : weighting vector         */
                            int16_t J,          /* (i) : number of input vectors  */
                            int16_t *new_d_data,/* (o) : output vector            */
                            int16_t K,          /* (i) : number of candidates     */
                            int16_t *best_indx, /* (o) : best indices             */
                            int16_t *ptr_prd,   /* (i) : pointer for backtracking */
                            int16_t *ptr_back,  /* (o) : pointer for backtracking */
                            int16_t PtrTab[2][16],/* (i) : quantizer table        */
                            int16_t MQ          /* (i) : size of quantizer        */
)
{
  int16_t m, l, p, q, sum[R_LSFQ*R_LSFQ];
  int16_t min[R_LSFQ], min_indx_p[R_LSFQ], min_indx_m[R_LSFQ];
  int16_t tmp1, tmp2;
  int32_t acc0;

  for (q=0; q<K; q++)
    min[q] = WEBRTC_SPL_WORD16_MAX;

  /* compute the errors */
  for (p=0; p<J; p++)
    for (m=0; m<MQ; m++){
      acc0 = 0;
      for (l=0; l<M/2; l++){
        tmp1 = extract_h(L_shl(L_mult(WebRtcG729fix_noise_fg_sum[ptr_prd[p]][l],
            WebRtcG729fix_noise_fg_sum[ptr_prd[p]][l]), 2));
        tmp1 = mult(tmp1, weight[l]);
        tmp2 = WebRtcSpl_SubSatW16(d_data[p*M+l],
            WebRtcG729fix_lspcb2[PtrTab[0][m]][l]);
        tmp1 = extract_h(L_shl(L_mult(tmp1, tmp2), 3));
        acc0 = L_mac(acc0, tmp1, tmp2);
      }

      for (l=M/2; l<M; l++){
        tmp1 = extract_h(L_shl(L_mult(WebRtcG729fix_noise_fg_sum[ptr_prd[p]][l], 
            WebRtcG729fix_noise_fg_sum[ptr_prd[p]][l]), 2));
        tmp1 = mult(tmp1, weight[l]);
        tmp2 = WebRtcSpl_SubSatW16(d_data[p*M+l],
            WebRtcG729fix_lspcb2[PtrTab[1][m]][l]);
        tmp1 = extract_h(L_shl(L_mult(tmp1, tmp2), 3));
        acc0 = L_mac(acc0, tmp1, tmp2);
      }
      
      sum[p*MQ+m] = extract_h(acc0);
    }
      
  /* select the candidates */
  for (q=0; q<K; q++){
    min_indx_p[q] = 0;  /* G.729 maintenance */
    min_indx_m[q] = 0;  /* G.729 maintenance */
    for (p=0; p<J; p++)
      for (m=0; m<MQ; m++)
        if (sum[p*MQ+m] < min[q]){
          min[q] = sum[p*MQ+m];
          min_indx_p[q] = p;
          min_indx_m[q] = m;
        }
    
    sum[min_indx_p[q]*MQ+min_indx_m[q]] = WEBRTC_SPL_WORD16_MAX;
  }

  /* compute the candidates */
  for (q=0; q<K; q++){
    for (l=0; l<M/2; l++) {
      new_d_data[q*M+l] = WebRtcSpl_SubSatW16(d_data[min_indx_p[q]*M+l], 
          WebRtcG729fix_lspcb2[PtrTab[0][min_indx_m[q]]][l]);
    }
    for (l=M/2; l<M; l++) {
      new_d_data[q*M+l] = WebRtcSpl_SubSatW16(d_data[min_indx_p[q]*M+l], 
          WebRtcG729fix_lspcb2[PtrTab[1][min_indx_m[q]]][l]);
    }
    ptr_back[q] = min_indx_p[q];
    best_indx[q] = min_indx_m[q];
  }
}












