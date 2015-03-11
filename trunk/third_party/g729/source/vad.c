/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#include <stdio.h>
#include <stdint.h>
#include "ld8a.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* local function */
static int16_t MakeDec(
  int16_t dSLE,    /* (i)  : differential low band energy */
  int16_t dSE,     /* (i)  : differential full band energy */
  int16_t SD,      /* (i)  : differential spectral distortion */
  int16_t dSZC     /* (i)  : differential zero crossing rate */
);

/*---------------------------------------------------------------------------*
 * Function  vad_init                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 *                                                                           *
 * -> Initialization of variables for voice activity detection               *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void WebRtcG729fix_vad_init(vad_state *st)
{
  /* Static vectors to zero */
  WebRtcSpl_ZerosArrayW16(st->MeanLSF, M);

  /* Initialize VAD parameters */
  st->MeanSE = 0;
  st->MeanSLE = 0;
  st->MeanE = 0;
  st->MeanSZC = 0;
  st->count_sil = 0;
  st->count_update = 0;
  st->count_ext = 0;
  st->less_count = 0;
  st->flag = 1;
  st->Min = WEBRTC_SPL_WORD16_MAX;

  WebRtcSpl_ZerosArrayW16(st->Min_buffer, 16);
}


/*-----------------------------------------------------------------*
 * Functions vad                                                   *
 *           ~~~                                                   *
 * Input:                                                          *
 *   rc            : reflection coefficient                        *
 *   lsf[]         : unquantized lsf vector                        *
 *   r_h[]         : upper 16-bits of the autocorrelation vector   *
 *   r_l[]         : lower 16-bits of the autocorrelation vector   *
 *   exp_R0        : exponent of the autocorrelation vector        *
 *   sigpp[]       : preprocessed input signal                     *
 *   frm_count     : frame counter                                 *
 *   prev_marker   : VAD decision of the last frame                *
 *   pprev_marker  : VAD decision of the frame before last frame   *
 *                                                                 *
 * Output:                                                         *
 *                                                                 *
 *   marker        : VAD decision of the current frame             *
 *                                                                 *
 *-----------------------------------------------------------------*/
void WebRtcG729fix_vad(vad_state *st,
         int16_t rc,
         int16_t *lsf,
         int16_t *r_h,
         int16_t *r_l,
         int16_t exp_R0,
         int16_t *sigpp,
         int16_t frm_count,
         int16_t prev_marker,
         int16_t pprev_marker,
         int16_t *marker)
{
 /* scalar */
  int32_t acc0;
  int16_t i, j, exp, frac;
  int16_t ENERGY, ENERGY_low, SD, ZC, dSE, dSLE, dSZC;
  int16_t COEF, C_COEF, COEFZC, C_COEFZC, COEFSD, C_COEFSD;

  /* compute the frame energy */
  acc0 = WebRtcG729fix_L_Comp(r_h[0], r_l[0]);
  WebRtcG729fix_Log2(acc0, &exp, &frac);
  acc0 = WebRtcG729fix_Mpy_32_16(exp, frac, 9864);
  i = WebRtcSpl_SubSatW16(exp_R0, 1);
  i = WebRtcSpl_SubSatW16(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY = extract_h(acc0);
  ENERGY = WebRtcSpl_SubSatW16(ENERGY, 4875);

  /* compute the low band energy */
  acc0 = 0;
  for (i=1; i<=NP; i++)
    acc0 = L_mac(acc0, r_h[i], WebRtcG729fix_lbf_corr[i]);
  acc0 = L_shl(acc0, 1);
  acc0 = L_mac(acc0, r_h[0], WebRtcG729fix_lbf_corr[0]);
  WebRtcG729fix_Log2(acc0, &exp, &frac);
  acc0 = WebRtcG729fix_Mpy_32_16(exp, frac, 9864);
  i = WebRtcSpl_SubSatW16(exp_R0, 1);
  i = WebRtcSpl_SubSatW16(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY_low = extract_h(acc0);
  ENERGY_low = WebRtcSpl_SubSatW16(ENERGY_low, 4875);

  /* compute SD */
  acc0 = 0;
  for (i=0; i<M; i++){
    j = WebRtcSpl_SubSatW16(lsf[i], st->MeanLSF[i]);
    acc0 = L_mac(acc0, j, j);
  }
  SD = extract_h(acc0);      /* Q15 */

  /* compute # zero crossing */
  ZC = 0;
  for (i=ZC_START+1; i<=ZC_END; i++)
    if (mult(sigpp[i-1], sigpp[i]) < 0)
      ZC = WebRtcSpl_AddSatW16(ZC, 410);     /* Q15 */

  /* Initialize and update Mins */
  if (frm_count < 129) {
    if (ENERGY < st->Min) {
      st->Min = ENERGY;
      st->Prev_Min = ENERGY;
    }

    if ((frm_count & 0x0007) == 0) {
      i = WebRtcSpl_SubSatW16(shr(frm_count,3),1);
      st->Min_buffer[i] = st->Min;
      st->Min = WEBRTC_SPL_WORD16_MAX;
    }
  }

  if ((frm_count & 0x0007) == 0){
    st->Prev_Min = st->Min_buffer[0];
    for (i=1; i<16; i++){
      if (st->Min_buffer[i] < st->Prev_Min)
        st->Prev_Min = st->Min_buffer[i];
    }
  }

  if (frm_count >= 129){
    if (((frm_count & 0x0007) ^ (0x0001)) == 0){
      st->Min = st->Prev_Min;
      st->Next_Min = WEBRTC_SPL_WORD16_MAX;
    }
    if (ENERGY < st->Min)
      st->Min = ENERGY;
    if (ENERGY < st->Next_Min)
      st->Next_Min = ENERGY;

    if((frm_count & 0x0007) == 0){
      for (i=0; i<15; i++)
        st->Min_buffer[i] = st->Min_buffer[i+1];
      st->Min_buffer[15] = st->Next_Min;
      st->Prev_Min = st->Min_buffer[0];
      for (i=1; i<16; i++)
        if (st->Min_buffer[i] < st->Prev_Min)
          st->Prev_Min = st->Min_buffer[i];
    }

  }

  if (frm_count <= INIT_FRAME) {
    if(ENERGY < 3072){
      *marker = NOISE;
      st->less_count++;
    }
    else{
      *marker = VOICE;
      acc0 = L_deposit_h(st->MeanE);
      acc0 = L_mac(acc0, ENERGY, 1024);
      st->MeanE = extract_h(acc0);
      acc0 = L_deposit_h(st->MeanSZC);
      acc0 = L_mac(acc0, ZC, 1024);
      st->MeanSZC = extract_h(acc0);
      for (i=0; i<M; i++){
        acc0 = L_deposit_h(st->MeanLSF[i]);
        acc0 = L_mac(acc0, lsf[i], 1024);
        st->MeanLSF[i] = extract_h(acc0);
      }
    }
  }

  if (frm_count >= INIT_FRAME){
    if (frm_count == INIT_FRAME){
      acc0 = L_mult(st->MeanE, WebRtcG729fix_factor_fx[st->less_count]);
      acc0 = L_shl(acc0, WebRtcG729fix_shift_fx[st->less_count]);
      st->MeanE = extract_h(acc0);

      acc0 = L_mult(st->MeanSZC, WebRtcG729fix_factor_fx[st->less_count]);
      acc0 = L_shl(acc0, WebRtcG729fix_shift_fx[st->less_count]);
      st->MeanSZC = extract_h(acc0);

      for (i=0; i<M; i++){
        acc0 = L_mult(st->MeanLSF[i], WebRtcG729fix_factor_fx[st->less_count]);
        acc0 = L_shl(acc0, WebRtcG729fix_shift_fx[st->less_count]);
        st->MeanLSF[i] = extract_h(acc0);
      }

      st->MeanSE = WebRtcSpl_SubSatW16(st->MeanE, 2048);   /* Q11 */
      st->MeanSLE = WebRtcSpl_SubSatW16(st->MeanE, 2458);  /* Q11 */
    }

    dSE = WebRtcSpl_SubSatW16(st->MeanSE, ENERGY);
    dSLE = WebRtcSpl_SubSatW16(st->MeanSLE, ENERGY_low);
    dSZC = WebRtcSpl_SubSatW16(st->MeanSZC, ZC);

    if(ENERGY < 3072) {
      *marker = NOISE;
    }
    else {
      *marker = MakeDec(dSLE, dSE, SD, dSZC);
    }

    st->v_flag = 0;
    if((prev_marker==VOICE) && (*marker==NOISE) && (WebRtcSpl_AddSatW16(dSE,410) < 0)
       && (ENERGY>3072)){
      *marker = VOICE;
      st->v_flag = 1;
    }

    if(st->flag == 1){
      if((pprev_marker == VOICE) &&
         (prev_marker == VOICE) &&
         (*marker == NOISE) &&
         (WebRtcSpl_SubSatW16(abs_s(WebRtcSpl_SubSatW16(st->prev_energy,ENERGY)), 614) <= 0)){
        st->count_ext++;
        *marker = VOICE;
        st->v_flag = 1;
        if(st->count_ext <= 4)
          st->flag=1;
        else{
          st->count_ext=0;
          st->flag=0;
        }
      }
    }
    else
      st->flag=1;

    if(*marker == NOISE)
      st->count_sil++;

    if((*marker == VOICE) && (st->count_sil > 10) &&
       (WebRtcSpl_SubSatW16(ENERGY,st->prev_energy) <= 614)){
      *marker = NOISE;
      st->count_sil=0;
    }

    if(*marker == VOICE)
      st->count_sil=0;

    if ((WebRtcSpl_SubSatW16(ENERGY, 614) < st->MeanSE) && (frm_count > 128)
        && (!st->v_flag) && (rc < 19661))
      *marker = NOISE;

    if ((WebRtcSpl_SubSatW16(ENERGY,614) < st->MeanSE) && (rc < 24576)
        && (SD < 83)){
      st->count_update++;
      if (st->count_update < INIT_COUNT) {
        COEF = 24576;
        C_COEF = 8192;
        COEFZC = 26214;
        C_COEFZC = 6554;
        COEFSD = 19661;
        C_COEFSD = 13017;
      }
      else if (st->count_update < INIT_COUNT+10) {
        COEF = 31130;
        C_COEF = 1638;
        COEFZC = 30147;
        C_COEFZC = 2621;
        COEFSD = 21299;
        C_COEFSD = 11469;
      }
      else if (st->count_update < INIT_COUNT+20) {
        COEF = 31785;
        C_COEF = 983;
        COEFZC = 30802;
        C_COEFZC = 1966;
        COEFSD = 22938;
        C_COEFSD = 9830;
      }
      else if (st->count_update < INIT_COUNT+30) {
        COEF = 32440;
        C_COEF = 328;
        COEFZC = 31457;
        C_COEFZC = 1311;
        COEFSD = 24576;
        C_COEFSD = 8192;
      }
      else if (st->count_update < INIT_COUNT+40) {
        COEF = 32604;
        C_COEF = 164;
        COEFZC = 32440;
        C_COEFZC = 328;
        COEFSD = 24576;
        C_COEFSD = 8192;
      }
      else {
        COEF = 32604;
        C_COEF = 164;
        COEFZC = 32702;
        C_COEFZC = 66;
        COEFSD = 24576;
        C_COEFSD = 8192;
      }

      /* compute MeanSE */
      acc0 = L_mult(COEF, st->MeanSE);
      acc0 = L_mac(acc0, C_COEF, ENERGY);
      st->MeanSE = extract_h(acc0);

      /* compute MeanSLE */
      acc0 = L_mult(COEF, st->MeanSLE);
      acc0 = L_mac(acc0, C_COEF, ENERGY_low);
      st->MeanSLE = extract_h(acc0);

      /* compute MeanSZC */
      acc0 = L_mult(COEFZC, st->MeanSZC);
      acc0 = L_mac(acc0, C_COEFZC, ZC);
      st->MeanSZC = extract_h(acc0);

      /* compute MeanLSF */
      for (i=0; i<M; i++){
        acc0 = L_mult(COEFSD, st->MeanLSF[i]);
        acc0 = L_mac(acc0, C_COEFSD, lsf[i]);
        st->MeanLSF[i] = extract_h(acc0);
      }
    }

    if((frm_count > 128) && (((st->MeanSE < st->Min) &&
                        (SD < 83)) || (WebRtcSpl_SubSatW16(st->MeanSE, st->Min) > 2048))){
      st->MeanSE = st->Min;
      st->count_update = 0;
    }
  }

  st->prev_energy = ENERGY;
}

/* local function */
static int16_t MakeDec(
               int16_t dSLE,    /* (i)  : differential low band energy */
               int16_t dSE,     /* (i)  : differential full band energy */
               int16_t SD,      /* (i)  : differential spectral distortion */
               int16_t dSZC     /* (i)  : differential zero crossing rate */
               )
{
  int32_t acc0;

  /* SD vs dSZC */
  acc0 = L_mult(dSZC, -14680);          /* Q15*Q23*2 = Q39 */
  acc0 = L_mac(acc0, 8192, -28521);     /* Q15*Q23*2 = Q39 */
  acc0 = L_shr(acc0, 8);                /* Q39 -> Q31 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(SD));
  if (acc0 > 0) return VOICE;

  acc0 = L_mult(dSZC, 19065);           /* Q15*Q22*2 = Q38 */
  acc0 = L_mac(acc0, 8192, -19446);     /* Q15*Q22*2 = Q38 */
  acc0 = L_shr(acc0, 7);                /* Q38 -> Q31 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(SD));
  if (acc0 > 0) return VOICE;

  /* dSE vs dSZC */
  acc0 = L_mult(dSZC, 20480);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 16384);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(dSZC, -16384);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 19660);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 1024, 30802);      /* Q10*Q16*2 = Q27 */
  if (acc0 < 0) return VOICE;

  /* dSE vs SD */
  acc0 = L_mult(SD, -28160);            /* Q15*Q5*2 = Q22 */
  acc0 = L_mac(acc0, 64, 19988);        /* Q6*Q14*2 = Q22 */
  acc0 = L_mac(acc0, dSE, 512);         /* Q11*Q9*2 = Q22 */
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(SD, 32767);             /* Q15*Q15*2 = Q31 */
  acc0 = L_mac(acc0, 32, -30199);       /* Q5*Q25*2 = Q31 */
  if (acc0 > 0) return VOICE;

  /* dSLE vs dSZC */
  acc0 = L_mult(dSZC, -20480);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 22938);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(dSZC, 23831);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 4096, 31576);      /* Q12*Q16*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 2048, 17367);      /* Q11*Q15*2 = Q27 */
  if (acc0 < 0) return VOICE;

  /* dSLE vs SD */
  acc0 = L_mult(SD, -22400);            /* Q15*Q4*2 = Q20 */
  acc0 = L_mac(acc0, 32, 25395);        /* Q5*Q14*2 = Q20 */
  acc0 = L_mac(acc0, dSLE, 256);        /* Q11*Q8*2 = Q20 */
  if (acc0 < 0) return VOICE;

  /* dSLE vs dSE */
  acc0 = L_mult(dSE, -30427);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 256, -29959);      /* Q8*Q18*2 = Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSLE));
  if (acc0 > 0) return VOICE;

  acc0 = L_mult(dSE, -23406);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 512, 28087);       /* Q19*Q17*2 = Q27 */
  acc0 = WebRtcSpl_AddSatW32(acc0, L_deposit_h(dSLE));
  if (acc0 < 0) return VOICE;

  acc0 = L_mult(dSE, 24576);            /* Q11*Q14*2 = Q26 */
  acc0 = L_mac(acc0, 1024, 29491);      /* Q10*Q15*2 = Q26 */
  acc0 = L_mac(acc0, dSLE, 16384);      /* Q11*Q14*2 = Q26 */
  if (acc0 < 0) return VOICE;

  return NOISE;
}




