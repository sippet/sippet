/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* DTX and Comfort Noise Generator - Encoder part */

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "oper_32b.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "sid.h"

/* Local functions */
static void Calc_pastfilt(Coder_ld8a_state *st, int16_t *Coeff);
static void Calc_RCoeff(int16_t *Coeff, int16_t *RCoeff, int16_t *sh_RCoeff);
static int16_t Cmp_filt(int16_t *RCoeff, int16_t sh_RCoeff, int16_t *acf,
                        int16_t alpha, int16_t Fracthresh);
static void Calc_sum_acf(int16_t *acf, int16_t *sh_acf,
                    int16_t *sum, int16_t *sh_sum, int16_t nb);
static void Update_sumAcf(Coder_ld8a_state *st);

/*-----------------------------------------------------------*
 * procedure Init_Cod_cng:                                   *
 *           ~~~~~~~~~~~~                                    *
 *   Initialize variables used for dtx at the encoder        *
 *-----------------------------------------------------------*/
void WebRtcG729fix_Init_Cod_cng(Coder_ld8a_state *st)
{
  WebRtcSpl_ZerosArrayW16(st->sumAcf, SIZ_SUMACF);
  WebRtcSpl_MemSetW16(st->sh_sumAcf, 40, NB_SUMACF);

  WebRtcSpl_ZerosArrayW16(st->Acf, SIZ_ACF);
  WebRtcSpl_MemSetW16(st->sh_Acf, 40, NB_CURACF);

  WebRtcSpl_MemSetW16(st->sh_ener, 40, NB_GAIN);
  WebRtcSpl_ZerosArrayW16(st->ener, NB_GAIN);

  st->cur_gain = 0;
  st->fr_cur = 0;
  st->flag_chang = 0;

  return;
}


/*-----------------------------------------------------------*
 * procedure Cod_cng:                                        *
 *           ~~~~~~~~                                        *
 *   computes DTX decision                                   *
 *   encodes SID frames                                      *
 *   computes CNG excitation for encoder update              *
 *-----------------------------------------------------------*/
void WebRtcG729fix_Cod_cng(
  Coder_ld8a_state *st,
  int16_t *exc,          /* (i/o) : excitation array                     */
  int16_t pastVad,       /* (i)   : previous VAD decision                */
  int16_t *lsp_old_q,    /* (i/o) : previous quantized lsp               */
  int16_t *Aq,           /* (o)   : set of interpolated LPC coefficients */
  int16_t *ana,          /* (o)   : coded SID parameters                 */
  int16_t freq_prev[MA_NP][M],
                        /* (i/o) : previous LPS for quantization        */
  int16_t *seed         /* (i/o) : random generator seed                */
)
{

  int16_t i;

  int16_t curAcf[MP1];
  int16_t bid[M], zero[MP1];
  int16_t curCoeff[MP1];
  int16_t lsp_new[M];
  int16_t *lpcCoeff;
  int16_t cur_igain;
  int16_t energyq, temp;

  /* Update Ener and sh_ener */
  for (i = NB_GAIN-1; i >= 1; i--) {
    st->ener[i] = st->ener[i-1];
    st->sh_ener[i] = st->sh_ener[i-1];
  }

  /* Compute current Acfs */
  Calc_sum_acf(st->Acf, st->sh_Acf, curAcf, &st->sh_ener[0], NB_CURACF);

  /* Compute LPC coefficients and residual energy */
  if(curAcf[0] == 0) {
    st->ener[0] = 0;                /* should not happen */
  }
  else {
    WebRtcSpl_ZerosArrayW16(zero, MP1);
    WebRtcG729fix_Levinson(st, curAcf, zero, curCoeff, bid, &st->ener[0]);
  }

  /* if first frame of silence => SID frame */
  if(pastVad != 0) {
    ana[0] = 2;
    st->count_fr0 = 0;
    st->nb_ener = 1;
    WebRtcG729fix_Qua_Sidgain(st->ener, st->sh_ener, st->nb_ener, &energyq, &cur_igain);

  }
  else {
    st->nb_ener = WebRtcSpl_AddSatW16(st->nb_ener, 1);
    if(st->nb_ener > NB_GAIN) st->nb_ener = NB_GAIN;
    WebRtcG729fix_Qua_Sidgain(st->ener, st->sh_ener, st->nb_ener, &energyq, &cur_igain);
      
    /* Compute stationarity of current filter   */
    /* versus reference filter                  */
    if(Cmp_filt(st->RCoeff, st->sh_RCoeff, curAcf, st->ener[0], FRAC_THRESH1) != 0) {
      st->flag_chang = 1;
    }
      
    /* compare energy difference between current frame and last frame */
    temp = abs_s(WebRtcSpl_SubSatW16(st->prev_energy, energyq));
    temp = WebRtcSpl_SubSatW16(temp, 2);
    if (temp > 0) st->flag_chang = 1;
      
    st->count_fr0 = WebRtcSpl_AddSatW16(st->count_fr0, 1);
    if(st->count_fr0 < FR_SID_MIN) {
      ana[0] = 0;               /* no transmission */
    }
    else {
      if(st->flag_chang != 0) {
        ana[0] = 2;             /* transmit SID frame */
      }
      else{
        ana[0] = 0;
      }
        
      st->count_fr0 = FR_SID_MIN;   /* to avoid overflow */
    }
  }


  if(ana[0] == 2) {
      
    /* Reset frame count and change flag */
    st->count_fr0 = 0;
    st->flag_chang = 0;
      
    /* Compute past average filter */
    Calc_pastfilt(st, st->pastCoeff);
    Calc_RCoeff(st->pastCoeff, st->RCoeff, &st->sh_RCoeff);

    /* Compute stationarity of current filter   */
    /* versus past average filter               */


    /* if stationary */
    /* transmit average filter => new ref. filter */
    if(Cmp_filt(st->RCoeff, st->sh_RCoeff, curAcf, st->ener[0], FRAC_THRESH2) == 0) {
      lpcCoeff = st->pastCoeff;
    }

    /* else */
    /* transmit current filter => new ref. filter */
    else {
      lpcCoeff = curCoeff;
      Calc_RCoeff(curCoeff, st->RCoeff, &st->sh_RCoeff);
    }

    /* Compute SID frame codes */

    WebRtcG729fix_Az_lsp(lpcCoeff, lsp_new, lsp_old_q); /* From A(z) to lsp */

    /* LSP quantization */
    WebRtcG729fix_lsfq_noise(st->noise_fg, lsp_new, st->lspSid_q, freq_prev, &ana[1]);

    st->prev_energy = energyq;
    ana[4] = cur_igain;
    st->sid_gain = WebRtcG729fix_tab_Sidgain[cur_igain];


  } /* end of SID frame case */

  /* Compute new excitation */
  if(pastVad != 0) {
    st->cur_gain = st->sid_gain;
  }
  else {
    st->cur_gain = mult_r(st->cur_gain, A_GAIN0);
    st->cur_gain = WebRtcSpl_AddSatW16(st->cur_gain, mult_r(st->sid_gain, A_GAIN1));
  }

  WebRtcG729fix_Calc_exc_rand(st->L_exc_err, st->cur_gain, st->exc, seed, FLAG_COD);

  WebRtcG729fix_Int_qlpc(lsp_old_q, st->lspSid_q, Aq);
  WEBRTC_SPL_MEMCPY_W16(lsp_old_q, st->lspSid_q, M);

  /* Update sumAcf if fr_cur = 0 */
  if(st->fr_cur == 0) {
    Update_sumAcf(st);
  }

  return;
}

/*-----------------------------------------------------------*
 * procedure Update_cng:                                     *
 *           ~~~~~~~~~~                                      *
 *   Updates autocorrelation arrays                          *
 *   used for DTX/CNG                                        *
 *   If Vad=1 : updating of array sumAcf                     *
 *-----------------------------------------------------------*/
void WebRtcG729fix_Update_cng(
  Coder_ld8a_state *st,
  int16_t *r_h,      /* (i) :   MSB of frame autocorrelation        */
  int16_t exp_r,     /* (i) :   scaling factor associated           */
  int16_t Vad        /* (i) :   current Vad decision                */
)
{
  int16_t i;
  int16_t *ptr1, *ptr2;

  /* Update Acf and shAcf */
  ptr1 = st->Acf + SIZ_ACF - 1;
  ptr2 = ptr1 - MP1;
  for (i = 0; i < (SIZ_ACF-MP1); i++) {
    *ptr1-- = *ptr2--;
  }
  for (i = NB_CURACF - 1; i >= 1; i--) {
    st->sh_Acf[i] = st->sh_Acf[i-1];
  }

  /* Save current Acf */
  st->sh_Acf[0] = negate(WebRtcSpl_AddSatW16(16, exp_r));
  WEBRTC_SPL_MEMCPY_W16(st->Acf, r_h, MP1);

  st->fr_cur = WebRtcSpl_AddSatW16(st->fr_cur, 1);
  if (st->fr_cur == NB_CURACF) {
    st->fr_cur = 0;
    if (Vad != 0) {
      Update_sumAcf(st);
    }
  }

  return;
}


/*-----------------------------------------------------------*
 *         Local procedures                                  *
 *         ~~~~~~~~~~~~~~~~                                  *
 *-----------------------------------------------------------*/

/* Compute scaled autocorr of LPC coefficients used for Itakura distance */
/*************************************************************************/
static void Calc_RCoeff(int16_t *Coeff, int16_t *RCoeff, int16_t *sh_RCoeff)
{
  int16_t i, j;
  int16_t sh1;
  int32_t L_acc;
  
  /* RCoeff[0] = SUM(j=0->M) Coeff[j] ** 2 */
  L_acc = 0L;
  for (j = 0; j <= M; j++) {
    L_acc = L_mac(L_acc, Coeff[j], Coeff[j]);
  }
  
  /* Compute exponent RCoeff */
  sh1 = WebRtcSpl_NormW32(L_acc);
  L_acc = L_shl(L_acc, sh1);
  RCoeff[0] = L_round(L_acc);
  
  /* RCoeff[i] = SUM(j=0->M-i) Coeff[j] * Coeff[j+i] */
  for (i = 1; i <= M; i++) {
    L_acc = 0L;
    for (j = 0; j <= M - i; j++) {
      L_acc = L_mac(L_acc, Coeff[j], Coeff[j+i]);
    }
    L_acc = L_shl(L_acc, sh1);
    RCoeff[i] = L_round(L_acc);
  }
  *sh_RCoeff = sh1;
  return;
}

/* Compute Itakura distance and compare to threshold */
/*****************************************************/
static int16_t Cmp_filt(int16_t *RCoeff, int16_t sh_RCoeff, int16_t *acf,
                       int16_t alpha, int16_t FracThresh)
{
  int32_t L_temp0, L_temp1;
  int16_t temp1, temp2, sh[2], ind;
  int32_t temp3;
  int16_t i;
  int16_t diff, flag;
  int Overflow;

  sh[0] = 0;
  sh[1] = 0;
  ind = 1;
  flag = 0;
  do {
    Overflow = 0;
    temp1 = shr(RCoeff[0], sh[0]);
    temp2 = shr(acf[0], sh[1]);
    temp3 = temp1 * temp2; /* L_mult inline to check overflow */
    if (temp3 != (int32_t)0x40000000L) {
      temp3 *= 2;
    } else {
      Overflow = 1;
      temp3 = WEBRTC_SPL_WORD32_MAX;
    }
    L_temp0 = L_shr(temp3,1);
    for (i = 1; i <= M; i++) {
      temp1 = shr(RCoeff[i], sh[0]);
      temp2 = shr(acf[i], sh[1]);
      L_temp0 = L_mac(L_temp0, temp1, temp2);
    }
    if (Overflow != 0) {
      sh[(int)ind] = WebRtcSpl_AddSatW16(sh[(int)ind], 1);
      ind = WebRtcSpl_SubSatW16(1, ind);
    }
    else flag = 1;
  } while (flag == 0);
  
  temp1 = mult_r(alpha, FracThresh);
  L_temp1 = WebRtcSpl_AddSatW32(L_deposit_l(temp1), L_deposit_l(alpha));
  temp1 = WebRtcSpl_AddSatW16(sh_RCoeff, 9);  /* 9 = Lpc_justif. * 2 - 16 + 1 */
  temp2 = WebRtcSpl_AddSatW16(sh[0], sh[1]);
  temp1 = WebRtcSpl_SubSatW16(temp1, temp2);
  L_temp1 = L_shl(L_temp1, temp1);
  
  L_temp0 = WebRtcSpl_SubSatW32(L_temp0, L_temp1);
  if (L_temp0 > 0L) diff = 1;
  else diff = 0;

  return diff;
}

/* Compute past average filter */
/*******************************/
static void Calc_pastfilt(Coder_ld8a_state *st, int16_t *Coeff)
{
  int16_t s_sumAcf[MP1];
  int16_t bid[M], zero[MP1];
  int16_t temp;
  
  Calc_sum_acf(st->sumAcf, st->sh_sumAcf, s_sumAcf, &temp, NB_SUMACF);
  
  if(s_sumAcf[0] == 0L) {
    WebRtcSpl_ZerosArrayW16(&Coeff[1], M);
    Coeff[0] = 4096;
    return;
  }

  WebRtcSpl_ZerosArrayW16(zero, MP1);
  WebRtcG729fix_Levinson(st, s_sumAcf, zero, Coeff, bid, &temp);
  return;
}

/* Update sumAcf */
/*****************/
static void Update_sumAcf(Coder_ld8a_state *st)
{
  int16_t *ptr1, *ptr2;
  int16_t i;

  /*** Move sumAcf ***/
  ptr1 = st->sumAcf + SIZ_SUMACF - 1;
  ptr2 = ptr1 - MP1;
  for (i = 0; i < (SIZ_SUMACF - MP1); i++) {
    *ptr1-- = *ptr2--;
  }
  for (i = NB_SUMACF - 1; i >= 1; i--) {
    st->sh_sumAcf[i] = st->sh_sumAcf[i-1];
  }

  /* Compute new sumAcf */
  Calc_sum_acf(st->Acf, st->sh_Acf, st->sumAcf, st->sh_sumAcf, NB_CURACF);
  return;
}

/* Compute sum of acfs (curAcf, sumAcf or s_sumAcf) */
/****************************************************/
static void Calc_sum_acf(int16_t *acf, int16_t *sh_acf,
                         int16_t *sum, int16_t *sh_sum, int16_t nb)
{

  int16_t *ptr1;
  int32_t L_temp, L_tab[MP1];
  int16_t sh0, temp;
  int16_t i, j;
  
  /* Compute sum = sum of nb acfs */
  /* Find sh_acf minimum */
  sh0 = WebRtcSpl_MinValueW16(sh_acf, nb);
  sh0 = WebRtcSpl_AddSatW16(sh0, 14);           /* 2 bits of margin */

  WebRtcSpl_ZerosArrayW32(L_tab, MP1);
  ptr1 = acf;
  for (i = 0; i < nb; i++) {
    temp = WebRtcSpl_SubSatW16(sh0, sh_acf[i]);
    for (j = 0; j < MP1; j++) {
      L_temp = L_deposit_l(*ptr1++);
      L_temp = L_shl(L_temp, temp); /* shift right if temp<0 */
      L_tab[j] = WebRtcSpl_AddSatW32(L_tab[j], L_temp);
    }
  } 
  temp = WebRtcSpl_NormW32(L_tab[0]);
  for (i = 0; i <= M; i++) {
    sum[i] = extract_h(L_shl(L_tab[i], temp));
  }
  temp = WebRtcSpl_SubSatW16(temp, 16);
  *sh_sum = WebRtcSpl_AddSatW16(sh0, temp);
  return;
}

