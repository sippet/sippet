/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a and Init_Coder_ld8a                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  WebRtcG729fix_Init_Coder_ld8a();                               *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  WebRtcG729fix_Coder_ld8a(int16_t ana[]);                       *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_ld8a.h"

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  WebRtcG729fix_Init_Coder_ld8a();                               *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void WebRtcG729fix_Init_Coder_ld8a(Coder_ld8a_state *st)
{

  /*----------------------------------------------------------------------*
  *      Initialize pointers to speech vector.                            *
  *                                                                       *
  *                                                                       *
  *   |--------------------|-------------|-------------|------------|     *
  *     previous speech           sf1           sf2         L_NEXT        *
  *                                                                       *
  *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
  *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
  *   |                   <-- present frame (L_FRAME) -->                 *
  * old_speech            |              <-- new speech (L_FRAME) -->     *
  * p_window              |              |                                *
  *                     speech           |                                *
  *                             new_speech                                *
  *-----------------------------------------------------------------------*/

  st->new_speech = st->old_speech + L_TOTAL - L_FRAME;  /* New speech     */
  st->speech     = st->new_speech - L_NEXT;             /* Present frame  */
  st->p_window   = st->old_speech + L_TOTAL - L_WINDOW; /* For LPC window */

  /* Initialize static pointers */

  st->wsp    = st->old_wsp + PIT_MAX;
  st->exc    = st->old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */

  WebRtcSpl_ZerosArrayW16(st->old_speech, L_TOTAL);
  WebRtcSpl_ZerosArrayW16(st->old_exc, L_FRAME+PIT_MAX+L_INTERPOL);
  WebRtcSpl_ZerosArrayW16(st->old_wsp, PIT_MAX);
  WebRtcSpl_ZerosArrayW16(st->mem_w, M);
  WebRtcSpl_ZerosArrayW16(st->mem_w0, M);
  WebRtcSpl_ZerosArrayW16(st->mem_zero, M);
  st->sharp = SHARPMIN;

  /* Initialize lsp_old[] */

  WEBRTC_SPL_MEMCPY_W16(st->lsp_old, WebRtcG729fix_lsp_old_reset, M);

  /* Initialize lsp_old_q[] */

  WEBRTC_SPL_MEMCPY_W16(st->lsp_old_q, st->lsp_old, M);
  WebRtcG729fix_Lsp_encw_reset(st);
  WebRtcG729fix_Init_exc_err(st->L_exc_err);

  /* For G.729B */
  /* Initialize VAD/DTX parameters */
  st->pastVad = 1;
  st->ppastVad = 1;
  st->seed = INIT_SEED;
  WebRtcG729fix_vad_init(&st->vad_state);
  WebRtcG729fix_Init_lsfq_noise(st->noise_fg);

  /* Initialize Qua_gain */
  WEBRTC_SPL_MEMCPY_W16(st->past_qua_en, WebRtcG729fix_past_qua_en_reset, 4);

  /* Initialize Levinson */
  WEBRTC_SPL_MEMCPY_W16(st->old_A, WebRtcG729fix_old_A_reset, M+1);
  WebRtcSpl_ZerosArrayW16(st->old_rc, 2);
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a                                          *
 *            ~~~~~~~~~~                                           *
 *  WebRtcG729fix_Coder_ld8a(int16_t ana[]);                       *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void WebRtcG729fix_Coder_ld8a(
     Coder_ld8a_state *st,
     int16_t ana[],       /* output  : Analysis parameters */
     int16_t frame,       /* input   : frame counter       */
     int16_t vad_enable   /* input   : VAD enable flag     */
)
{

  /* LPC analysis */

  int16_t Aq_t[(MP1)*2];         /* A(z)   quantized for the 2 subframes */
  int16_t Ap_t[(MP1)*2];         /* A(z/gamma)       for the 2 subframes */
  int16_t *Aq, *Ap;              /* Pointer on Aq_t and Ap_t             */

  /* Other vectors */

  int16_t h1[L_SUBFR];            /* Impulse response h1[]              */
  int16_t xn[L_SUBFR];            /* Target vector for pitch search     */
  int16_t xn2[L_SUBFR];           /* Target vector for codebook search  */
  int16_t code[L_SUBFR];          /* Fixed codebook excitation          */
  int16_t y1[L_SUBFR];            /* Filtered adaptive excitation       */
  int16_t y2[L_SUBFR];            /* Filtered fixed codebook excitation */
  int16_t g_coeff[4];             /* Correlations between xn & y1       */

  int16_t g_coeff_cs[5];
  int16_t exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                     <y1,y1>, -2<xn,y1>,
                                          <y2,y2>, -2<xn,y2>, 2<y1,y2> */

  /* Scalars */

  int16_t i, j, k, i_subfr;
  int16_t T_op, T0, T0_min, T0_max, T0_frac;
  int16_t gain_pit, gain_code, index;
  int16_t temp, taming;
  int32_t L_temp;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
  {
     /* Temporary vectors */
    int16_t r_l[NP+1], r_h[NP+1];     /* Autocorrelations low and hi          */
    int16_t rc[M];                    /* Reflection coefficients.             */
    int16_t lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */

    /* For G.729B */
    int16_t rh_nbe[MP1];             
    int16_t lsf_new[M];
    int16_t lsfq_mem[MA_NP][M];
    int16_t exp_R0, Vad;

    /* LP analysis */
    WebRtcG729fix_Autocorr(st->p_window, NP, r_h, r_l, &exp_R0); /* Autocorrelations */
    WEBRTC_SPL_MEMCPY_W16(rh_nbe, r_h, MP1);
    WebRtcG729fix_Lag_window(NP, r_h, r_l);                      /* Lag windowing    */
    WebRtcG729fix_Levinson(st, r_h, r_l, Ap_t, rc, &temp);       /* Levinson Durbin  */
    WebRtcG729fix_Az_lsp(Ap_t, lsp_new, st->lsp_old);            /* From A(z) to lsp */

    /* For G.729B */
    /* ------ VAD ------- */
    WebRtcG729fix_Lsp_lsf(lsp_new, lsf_new, M);
    WebRtcG729fix_vad(&st->vad_state, rc[1], lsf_new, r_h, r_l, exp_R0, st->p_window,
        frame, st->pastVad, st->ppastVad, &Vad);

    WebRtcG729fix_Update_cng(st, rh_nbe, exp_R0, Vad);
    
    /* ---------------------- */
    /* Case of Inactive frame */
    /* ---------------------- */

    if ((Vad == 0) && (vad_enable == 1)){

      WebRtcG729fix_Get_freq_prev(st, lsfq_mem);
      WebRtcG729fix_Cod_cng(st, st->exc, st->pastVad, st->lsp_old_q, Aq_t, ana, lsfq_mem, &st->seed);
      WebRtcG729fix_Update_freq_prev(st, lsfq_mem);
      st->ppastVad = st->pastVad;
      st->pastVad = Vad;

      /* Update wsp, mem_w and mem_w0 */
      Aq = Aq_t;
      for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
        
        /* Residual signal in xn */
        WebRtcG729fix_Residu(Aq, &st->speech[i_subfr], xn, L_SUBFR);
        
        WebRtcG729fix_Weight_Az(Aq, GAMMA1, M, Ap_t);
        
        /* Compute wsp and mem_w */
        Ap = Ap_t + MP1;
        Ap[0] = 4096;
        for (i = 1; i <= M; i++)    /* Ap[i] = Ap_t[i] - 0.7 * Ap_t[i-1]; */
          Ap[i] = WebRtcSpl_SubSatW16(Ap_t[i], mult(Ap_t[i-1], 22938));
        WebRtcG729fix_Syn_filt(Ap, xn, &st->wsp[i_subfr], L_SUBFR, st->mem_w, 1);
        
        /* Compute mem_w0 */
        for (i = 0; i < L_SUBFR; i++)
          xn[i] = WebRtcSpl_SubSatW16(xn[i], st->exc[i_subfr+i]);  /* residu[] - exc[] */
        WebRtcG729fix_Syn_filt(Ap_t, xn, xn, L_SUBFR, st->mem_w0, 1);
                
        Aq += MP1;
      }
      
      
      st->sharp = SHARPMIN;
      
      /* Update memories for next frames */
      Copy(&st->old_speech[L_FRAME], &st->old_speech[0], L_TOTAL-L_FRAME);
      Copy(&st->old_wsp[L_FRAME], &st->old_wsp[0], PIT_MAX);
      Copy(&st->old_exc[L_FRAME], &st->old_exc[0], PIT_MAX+L_INTERPOL);
      
      return;
    }  /* End of inactive frame case */
    


    /* -------------------- */
    /* Case of Active frame */
    /* -------------------- */
    
    /* Case of active frame */
    *ana++ = 1;
    st->seed = INIT_SEED;
    st->ppastVad = st->pastVad;
    st->pastVad = Vad;

    /* LSP quantization */
    WebRtcG729fix_Qua_lsp(st, lsp_new, lsp_new_q, ana);
    ana += 2;                         /* Advance analysis parameters pointer */

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all subframes                  *
     * The interpolated parameters are in array Aq_t[].                   *
     *--------------------------------------------------------------------*/

    WebRtcG729fix_Int_qlpc(st->lsp_old_q, lsp_new_q, Aq_t);

    /* Compute A(z/gamma) */

    WebRtcG729fix_Weight_Az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
    WebRtcG729fix_Weight_Az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);

    /* update the LSPs for the next frame */

    WEBRTC_SPL_MEMCPY_W16(st->lsp_old, lsp_new, M);
    WEBRTC_SPL_MEMCPY_W16(st->lsp_old_q, lsp_new_q, M);
  }

 /*----------------------------------------------------------------------*
  * - Find the weighted input speech w_sp[] for the whole speech frame   *
  * - Find the open-loop pitch delay                                     *
  *----------------------------------------------------------------------*/

  WebRtcG729fix_Residu(&Aq_t[0], &st->speech[0], &st->exc[0], L_SUBFR);
  WebRtcG729fix_Residu(&Aq_t[MP1], &st->speech[L_SUBFR], &st->exc[L_SUBFR], L_SUBFR);

  {
    int16_t Ap1[MP1];

    Ap = Ap_t;
    Ap1[0] = 4096;
    for (i = 1; i <= M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
      Ap1[i] = WebRtcSpl_SubSatW16(Ap[i], mult(Ap[i-1], 22938));
    WebRtcG729fix_Syn_filt(Ap1, &st->exc[0], &st->wsp[0], L_SUBFR, st->mem_w, 1);

    Ap += MP1;
    for (i = 1; i <= M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
      Ap1[i] = WebRtcSpl_SubSatW16(Ap[i], mult(Ap[i-1], 22938));
    WebRtcG729fix_Syn_filt(Ap1, &st->exc[L_SUBFR], &st->wsp[L_SUBFR], L_SUBFR, st->mem_w, 1);
  }

  /* Find open loop pitch lag */

  T_op = WebRtcG729fix_Pitch_ol_fast(st->wsp, PIT_MAX, L_FRAME);

  /* Range for closed loop pitch search in 1st subframe */

  T0_min = WebRtcSpl_SubSatW16(T_op, 3);
  if (T0_min < PIT_MIN) {
    T0_min = PIT_MIN;
  }

  T0_max = WebRtcSpl_AddSatW16(T0_min, 6);
  if (T0_max > PIT_MAX)
  {
     T0_max = PIT_MAX;
     T0_min = WebRtcSpl_SubSatW16(T0_max, 6);
  }


 /*------------------------------------------------------------------------*
  *          Loop for every subframe in the analysis frame                 *
  *------------------------------------------------------------------------*
  *  To find the pitch and innovation parameters. The subframe size is     *
  *  L_SUBFR and the loop is repeated 2 times.                             *
  *     - find the weighted LPC coefficients                               *
  *     - find the LPC residual signal res[]                               *
  *     - compute the target signal for pitch search                       *
  *     - compute impulse response of weighted synthesis filter (h1[])     *
  *     - find the closed-loop pitch parameters                            *
  *     - encode the pitch delay                                           *
  *     - find target vector for codebook search                           *
  *     - codebook search                                                  *
  *     - VQ of pitch and codebook gains                                   *
  *     - update states of weighting filter                                *
  *------------------------------------------------------------------------*/

  Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
  Ap = Ap_t;    /* pointer to weighted LPC coefficients             */

  for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    /*---------------------------------------------------------------*
     * Compute impulse response, h1[], of weighted synthesis filter  *
     *---------------------------------------------------------------*/

    h1[0] = 4096;
    WebRtcSpl_ZerosArrayW16(&h1[1], L_SUBFR-1);
    WebRtcG729fix_Syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);

   /*----------------------------------------------------------------------*
    *  Find the target vector for pitch search:                            *
    *----------------------------------------------------------------------*/

    WebRtcG729fix_Syn_filt(Ap, &st->exc[i_subfr], xn, L_SUBFR, st->mem_w0, 0);

    /*---------------------------------------------------------------------*
     *                 Closed-loop fractional pitch search                 *
     *---------------------------------------------------------------------*/

    T0 = WebRtcG729fix_Pitch_fr3_fast(&st->exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                    i_subfr, &T0_frac);

    index = WebRtcG729fix_Enc_lag3(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);

    *ana++ = index;

    if (i_subfr == 0) {
      *ana++ = WebRtcG729fix_Parity_Pitch(index);
    }

   /*-----------------------------------------------------------------*
    *   - find filtered pitch exc                                     *
    *   - compute pitch gain and limit between 0 and 1.2              *
    *   - update target vector for codebook search                    *
    *-----------------------------------------------------------------*/

    WebRtcG729fix_Syn_filt(Ap, &st->exc[i_subfr], y1, L_SUBFR, st->mem_zero, 0);

    gain_pit = WebRtcG729fix_G_pitch(xn, y1, g_coeff, L_SUBFR);

    /* clip pitch gain if taming is necessary */

    taming = WebRtcG729fix_test_err(st->L_exc_err, T0, T0_frac);

    if( taming == 1){
      if (gain_pit > GPCLIP) {
        gain_pit = GPCLIP;
      }
    }

    /* xn2[i]   = xn[i] - y1[i] * gain_pit  */

    for (i = 0; i < L_SUBFR; i++)
    {
      L_temp = L_mult(y1[i], gain_pit);
      L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
      xn2[i] = WebRtcSpl_SubSatW16(xn[i], extract_h(L_temp));
    }


   /*-----------------------------------------------------*
    * - Innovative codebook search.                       *
    *-----------------------------------------------------*/

    index = WebRtcG729fix_ACELP_Code_A(xn2, h1, T0, st->sharp, code, y2, &i);

    *ana++ = index;        /* Positions index */
    *ana++ = i;            /* Signs index     */


   /*-----------------------------------------------------*
    * - Quantization of gains.                            *
    *-----------------------------------------------------*/

    g_coeff_cs[0]     = g_coeff[0];            /* <y1,y1> */
    exp_g_coeff_cs[0] = negate(g_coeff[1]);    /* Q-Format:XXX -> JPN */
    g_coeff_cs[1]     = negate(g_coeff[2]);    /* (xn,y1) -> -2<xn,y1> */
    exp_g_coeff_cs[1] = negate(WebRtcSpl_AddSatW16(g_coeff[3], 1)); /* Q-Format:XXX -> JPN */

    WebRtcG729fix_Corr_xy2( xn, y1, y2, g_coeff_cs, exp_g_coeff_cs );  /* Q0 Q0 Q12 ^Qx ^Q0 */
                         /* g_coeff_cs[3]:exp_g_coeff_cs[3] = <y2,y2>   */
                         /* g_coeff_cs[4]:exp_g_coeff_cs[4] = -2<xn,y2> */
                         /* g_coeff_cs[5]:exp_g_coeff_cs[5] = 2<y1,y2>  */

    *ana++ = WebRtcG729fix_Qua_gain(st, code, g_coeff_cs, exp_g_coeff_cs,
                      L_SUBFR, &gain_pit, &gain_code, taming);


   /*------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pit  *
    *------------------------------------------------------------*/

    st->sharp = gain_pit;
    if (st->sharp > SHARPMAX)      { st->sharp = SHARPMAX; }
    else if (st->sharp < SHARPMIN) { st->sharp = SHARPMIN; }

   /*------------------------------------------------------*
    * - Find the total excitation                          *
    * - update filters memories for finding the target     *
    *   vector in the next subframe                        *
    *------------------------------------------------------*/

    for (i = 0; i < L_SUBFR; i++)
    {
      /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
      /* exc[i]  in Q0   gain_pit in Q14               */
      /* code[i] in Q13  gain_cod in Q1                */

      L_temp = L_mult(st->exc[i+i_subfr], gain_pit);
      L_temp = L_mac(L_temp, code[i], gain_code);
      L_temp = L_shl(L_temp, 1);
      st->exc[i+i_subfr] = L_round(L_temp);
    }

    WebRtcG729fix_update_exc_err(st->L_exc_err, gain_pit, T0);

    for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
    {
      temp          = extract_h(L_shl( L_mult(y1[i], gain_pit),  1) );
      k             = extract_h(L_shl( L_mult(y2[i], gain_code), 2) );
      st->mem_w0[j] = WebRtcSpl_SubSatW16(xn[i], WebRtcSpl_AddSatW16(temp, k));
    }

    Aq += MP1;           /* interpolated LPC parameters for next subframe */
    Ap += MP1;
  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME:                 *
  *     speech[], wsp[] and  exc[]                   *
  *--------------------------------------------------*/

  Copy(&st->old_speech[L_FRAME], &st->old_speech[0], L_TOTAL-L_FRAME);
  Copy(&st->old_wsp[L_FRAME], &st->old_wsp[0], PIT_MAX);
  Copy(&st->old_exc[L_FRAME], &st->old_exc[0], PIT_MAX+L_INTERPOL);
}
