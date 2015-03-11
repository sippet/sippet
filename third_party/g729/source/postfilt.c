/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*------------------------------------------------------------------------*
 *                         POSTFILTER.C                                   *
 *------------------------------------------------------------------------*
 * Performs adaptive postfiltering on the synthesis speech                *
 * This file contains all functions related to the post filter.           *
 *------------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "oper_32b.h"

/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "ld8a.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   GAMMA2_PST  : Formant postfiltering factor (numerator)      *
 *   GAMMA1_PST  : Formant postfiltering factor (denominator)    *
 *   GAMMAP      : Harmonic postfiltering factor                 *
 *   MU          : Factor for tilt compensation filter           *
 *   AGC_FAC     : Factor for automatic gain control             *
 *---------------------------------------------------------------*/


/*---------------------------------------------------------------*
 * Procedure    Init_Post_Filter:                                *
 *              ~~~~~~~~~~~~~~~~                                 *
 *  Initializes the postfilter parameters:                       *
 *---------------------------------------------------------------*/

void WebRtcG729fix_Init_Post_Filter(Post_Filter_state *st)
{
  st->res2 = st->res2_buf + PIT_MAX;
  st->scal_res2 = st->scal_res2_buf + PIT_MAX;

  WebRtcSpl_ZerosArrayW16(st->mem_syn_pst, M);
  WebRtcSpl_ZerosArrayW16(st->res2_buf, PIT_MAX+L_SUBFR);
  WebRtcSpl_ZerosArrayW16(st->scal_res2_buf, PIT_MAX+L_SUBFR);

  st->mem_pre = 0;
  st->past_gain = 4096; /* past_gain = 1.0 (Q12) */

  return;
}


/*------------------------------------------------------------------------*
 *  Procedure     Post_Filter:                                            *
 *                ~~~~~~~~~~~                                             *
 *------------------------------------------------------------------------*
 *  The postfiltering process is described as follows:                    *
 *                                                                        *
 *  - inverse filtering of syn[] through A(z/GAMMA2_PST) to get res2[]    *
 *  - use res2[] to compute pitch parameters                              *
 *  - perform pitch postfiltering                                         *
 *  - tilt compensation filtering; 1 - MU*k*z^-1                          *
 *  - synthesis filtering through 1/A(z/GAMMA1_PST)                       *
 *  - adaptive gain control                                               *
 *------------------------------------------------------------------------*/

void WebRtcG729fix_Post_Filter(
  Post_Filter_state *st,
  int16_t *syn,       /* in/out: synthesis speech (postfiltered is output)    */
  int16_t *Az_4,      /* input : interpolated LPC parameters in all subframes */
  int16_t *T,          /* input : decoded pitch lags in all subframes          */
  int16_t Vad
)
{
 /*-------------------------------------------------------------------*
  *           Declaration of parameters                               *
  *-------------------------------------------------------------------*/

 int16_t res2_pst[L_SUBFR];  /* res2[] after pitch postfiltering */
 int16_t syn_pst[L_FRAME];   /* post filtered synthesis speech   */

 int16_t Ap3[MP1], Ap4[MP1];  /* bandwidth expanded LP parameters */

 int16_t *Az;                 /* pointer to Az_4:                 */
                             /*  LPC parameters in each subframe */
 int16_t   t0_max, t0_min;    /* closed-loop pitch search range   */
 int16_t   i_subfr;           /* index for beginning of subframe  */

 int16_t h[L_H];

 int16_t  i, j;
 int16_t  temp1, temp2;
 int32_t  L_tmp;

   /*-----------------------------------------------------*
    * Post filtering                                      *
    *-----------------------------------------------------*/

    Az = Az_4;

    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
      /* Find pitch range t0_min - t0_max */

      t0_min = WebRtcSpl_SubSatW16(*T++, 3);
      t0_max = WebRtcSpl_AddSatW16(t0_min, 6);
      if (t0_max > PIT_MAX) {
        t0_max = PIT_MAX;
        t0_min = WebRtcSpl_SubSatW16(t0_max, 6);
      }

      /* Find weighted filter coefficients Ap3[] and ap[4] */

      WebRtcG729fix_Weight_Az(Az, GAMMA2_PST, M, Ap3);
      WebRtcG729fix_Weight_Az(Az, GAMMA1_PST, M, Ap4);

      /* filtering of synthesis speech by A(z/GAMMA2_PST) to find res2[] */

      WebRtcG729fix_Residu(Ap3, &syn[i_subfr], st->res2, L_SUBFR);

      /* scaling of "res2[]" to avoid energy overflow */

      for (j=0; j<L_SUBFR; j++)
      {
        st->scal_res2[j] = shr(st->res2[j], 2);
      }

      /* pitch postfiltering */
      if (Vad == 1)
        WebRtcG729fix_pit_pst_filt(st->res2, st->scal_res2, t0_min, t0_max, L_SUBFR, res2_pst);
      else
        for (j=0; j<L_SUBFR; j++)
          res2_pst[j] = st->res2[j];

      /* tilt compensation filter */

      /* impulse response of A(z/GAMMA2_PST)/A(z/GAMMA1_PST) */

      WEBRTC_SPL_MEMCPY_W16(h, Ap3, M+1);
      WebRtcSpl_ZerosArrayW16(&h[M+1], L_H-M-1);
      WebRtcG729fix_Syn_filt(Ap4, h, h, L_H, &h[M+1], 0);

      /* 1st correlation of h[] */

      L_tmp = L_mult(h[0], h[0]);
      for (i=1; i<L_H; i++) L_tmp = L_mac(L_tmp, h[i], h[i]);
      temp1 = extract_h(L_tmp);

      L_tmp = L_mult(h[0], h[1]);
      for (i=1; i<L_H-1; i++) L_tmp = L_mac(L_tmp, h[i], h[i+1]);
      temp2 = extract_h(L_tmp);

      if(temp2 <= 0) {
        temp2 = 0;
      }
      else {
        temp2 = mult(temp2, MU);
        temp2 = div_s(temp2, temp1);
      }

      WebRtcG729fix_preemphasis(&st->mem_pre, res2_pst, temp2, L_SUBFR);

      /* filtering through  1/A(z/GAMMA1_PST) */

      WebRtcG729fix_Syn_filt(Ap4, res2_pst, &syn_pst[i_subfr], L_SUBFR, st->mem_syn_pst, 1);

      /* scale output to input */

      WebRtcG729fix_agc(&st->past_gain, &syn[i_subfr], &syn_pst[i_subfr], L_SUBFR);

      /* update res2[] buffer;  shift by L_SUBFR */

      Copy(&st->res2[L_SUBFR-PIT_MAX], &st->res2[-PIT_MAX], PIT_MAX);
      Copy(&st->scal_res2[L_SUBFR-PIT_MAX], &st->scal_res2[-PIT_MAX], PIT_MAX);

      Az += MP1;
    }

    /* update syn[] buffer */

    WEBRTC_SPL_MEMCPY_W16(&syn[-M], &syn[L_FRAME-M], M);

    /* overwrite synthesis speech by postfiltered synthesis speech */

    WEBRTC_SPL_MEMCPY_W16(syn, syn_pst, L_FRAME);

    return;
}


/*---------------------------------------------------------------------------*
 * procedure pitch_pst_filt                                                  *
 * ~~~~~~~~~~~~~~~~~~~~~~~~                                                  *
 * Find the pitch period  around the transmitted pitch and perform           *
 * harmonic postfiltering.                                                   *
 * Filtering through   (1 + g z^-T) / (1+g) ;   g = min(pit_gain*gammap, 1)  *
 *--------------------------------------------------------------------------*/

void WebRtcG729fix_pit_pst_filt(
  int16_t *signal,      /* (i)     : input signal                        */
  int16_t *scal_sig,    /* (i)     : input signal (scaled, divided by 4) */
  int16_t t0_min,       /* (i)     : minimum value in the searched range */
  int16_t t0_max,       /* (i)     : maximum value in the searched range */
  int16_t L_subfr,      /* (i)     : size of filtering                   */
  int16_t *signal_pst   /* (o)     : harmonically postfiltered signal    */
)
{
  int16_t i, j, t0;
  int16_t g0, gain, cmax, en, en0;
  int16_t *p, *p1, *deb_sig;
  int32_t corr, cor_max, ener, ener0, temp;
  int32_t L_temp;

/*---------------------------------------------------------------------------*
 * Compute the correlations for all delays                                   *
 * and select the delay which maximizes the correlation                      *
 *---------------------------------------------------------------------------*/

  deb_sig = &scal_sig[-t0_min];
  cor_max = WEBRTC_SPL_WORD32_MIN;
  t0 = t0_min;             /* Only to remove warning from some compilers */
  for (i=t0_min; i<=t0_max; i++)
  {
    corr = 0;
    p    = scal_sig;
    p1   = deb_sig;
    for (j=0; j<L_subfr; j++)
       corr = L_mac(corr, *p++, *p1++);

    L_temp = WebRtcSpl_SubSatW32(corr, cor_max);
    if (L_temp > (int32_t)0)
    {
      cor_max = corr;
      t0 = i;
    }
    deb_sig--;
  }

  /* Compute the energy of the signal delayed by t0 */

  ener = 1;
  p = scal_sig - t0;
  for ( i=0; i<L_subfr ;i++, p++)
    ener = L_mac(ener, *p, *p);

  /* Compute the signal energy in the present subframe */

  ener0 = 1;
  p = scal_sig;
  for ( i=0; i<L_subfr; i++, p++)
    ener0 = L_mac(ener0, *p, *p);

  if (cor_max < 0)
  {
    cor_max = 0;
  }

  /* scale "cor_max", "ener" and "ener0" on 16 bits */

  temp = cor_max;
  if (ener > temp)
  {
    temp = ener;
  }
  if (ener0 > temp)
  {
    temp = ener0;
  }
  j = WebRtcSpl_NormW32(temp);
  cmax = L_round(L_shl(cor_max, j));
  en = L_round(L_shl(ener, j));
  en0 = L_round(L_shl(ener0, j));

  /* prediction gain (dB)= -10 log(1-cor_max*cor_max/(ener*ener0)) */

  /* temp = (cor_max * cor_max) - (0.5 * ener * ener0)  */
  temp = L_mult(cmax, cmax);
  temp = WebRtcSpl_SubSatW32(temp, L_shr(L_mult(en, en0), 1));

  if (temp < (int32_t)0)           /* if prediction gain < 3 dB   */
  {                               /* switch off pitch postfilter */
    for (i = 0; i < L_subfr; i++)
      signal_pst[i] = signal[i];
    return;
  }

  if (cmax > en)      /* if pitch gain > 1 */
  {
    g0 = INV_GAMMAP;
    gain = GAMMAP_2;
  }
  else {
    cmax = shr(mult(cmax, GAMMAP), 1);  /* cmax(Q14) = cmax(Q15) * GAMMAP */
    en = shr(en, 1);          /* Q14 */
    i = WebRtcSpl_AddSatW16(cmax, en);
    if(i > 0)
    {
      gain = div_s(cmax, i);    /* gain(Q15) = cor_max/(cor_max+ener)  */
      g0 = WebRtcSpl_SubSatW16(32767, gain);    /* g0(Q15) = 1 - gain */
    }
    else
    {
      g0 =  32767;
      gain = 0;
    }
  }


  for (i = 0; i < L_subfr; i++)
  {
    /* signal_pst[i] = g0*signal[i] + gain*signal[i-t0]; */

    signal_pst[i] = WebRtcSpl_AddSatW16(mult(g0, signal[i]), mult(gain, signal[i-t0]));

  }

  return;
}

/*---------------------------------------------------------------------*
 * routine preemphasis()                                               *
 * ~~~~~~~~~~~~~~~~~~~~~                                               *
 * Preemphasis: filtering through 1 - g z^-1                           *
 *---------------------------------------------------------------------*/

void WebRtcG729fix_preemphasis(
  int16_t *mem_pre,
  int16_t *signal,  /* (i/o)   : input signal overwritten by the output */
  int16_t g,        /* (i) Q15 : preemphasis coefficient                */
  int16_t L         /* (i)     : size of filtering                      */
)
{
  int16_t *p1, *p2, temp, i;

  p1 = signal + L - 1;
  p2 = p1 - 1;
  temp = *p1;

  for (i = 0; i <= L-2; i++)
  {
    p1[-i] = WebRtcSpl_SubSatW16(p1[-i], mult(g, p2[-i]));
  }

  p1[-i] = WebRtcSpl_SubSatW16(p1[-i], mult(g, *mem_pre));

  *mem_pre = temp;

  return;
}

/*----------------------------------------------------------------------*
 *   routine agc()                                                      *
 *   ~~~~~~~~~~~~~                                                      *
 * Scale the postfilter output on a subframe basis by automatic control *
 * of the subframe gain.                                                *
 *  gain[n] = AGC_FAC * gain[n-1] + (1 - AGC_FAC) g_in/g_out            *
 *----------------------------------------------------------------------*/

void WebRtcG729fix_agc(
  int16_t *past_gain,/* (i/o)   : past gain in Q12         */
  int16_t *sig_in,   /* (i)     : postfilter input signal  */
  int16_t *sig_out,  /* (i/o)   : postfilter output signal */
  int16_t l_trm      /* (i)     : subframe size            */
)
{
  int16_t i, exp;
  int16_t gain_in, gain_out, g0, gain;                     /* Q12 */
  int32_t s;

  int16_t signal[L_SUBFR];

  /* calculate gain_out with exponent */

  for(i=0; i<l_trm; i++)
    signal[i] = shr(sig_out[i], 2);

  s = 0;
  for(i=0; i<l_trm; i++)
    s = L_mac(s, signal[i], signal[i]);

  if (s == 0) {
    *past_gain = 0;
    return;
  }
  exp = WebRtcSpl_SubSatW16(WebRtcSpl_NormW32(s), 1);
  gain_out = L_round(L_shl(s, exp));

  /* calculate gain_in with exponent */

  for(i=0; i<l_trm; i++)
    signal[i] = shr(sig_in[i], 2);

  s = 0;
  for(i=0; i<l_trm; i++)
    s = L_mac(s, signal[i], signal[i]);

  if (s == 0) {
    g0 = 0;
  }
  else {
    i = WebRtcSpl_NormW32(s);
    gain_in = L_round(L_shl(s, i));
    exp = WebRtcSpl_SubSatW16(exp, i);

   /*---------------------------------------------------*
    *  g0(Q12) = (1-AGC_FAC) * sqrt(gain_in/gain_out);  *
    *---------------------------------------------------*/

    s = L_deposit_l(div_s(gain_out,gain_in));   /* Q15 */
    s = L_shl(s, 7);           /* s(Q22) = gain_out / gain_in */
    s = L_shr(s, exp);         /* Q22, add exponent */

    /* i(Q12) = s(Q19) = 1 / sqrt(s(Q22)) */
    s = WebRtcG729fix_Inv_sqrt(s);           /* Q19 */
    i = L_round(L_shl(s,9));     /* Q12 */

    /* g0(Q12) = i(Q12) * (1-AGC_FAC)(Q15) */
    g0 = mult(i, AGC_FAC1);       /* Q12 */
  }

  /* compute gain(n) = AGC_FAC gain(n-1) + (1-AGC_FAC)gain_in/gain_out */
  /* sig_out(n) = gain(n) sig_out(n)                                   */

  gain = *past_gain;
  for(i=0; i<l_trm; i++) {
    gain = mult(gain, AGC_FAC);
    gain = WebRtcSpl_AddSatW16(gain, g0);
    sig_out[i] = extract_h(L_shl(L_mult(sig_out[i], gain), 3));
  }
  *past_gain = gain;

  return;
}
