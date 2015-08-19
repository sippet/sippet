/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.5    Last modified: October 2006 

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 *   Functions Init_Decod_ld8a  and Decod_ld8a                     *
 *-----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "tab_ld8a.h"

#include "dtx.h"
#include "sid.h"

/*---------------------------------------------------------------*
 *   Decoder constant parameters (defined in "ld8a.h")           *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MIN     : Minimum pitch lag.                            *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation            *
 *   PRM_SIZE    : Size of vector containing analysis parameters *
 *---------------------------------------------------------------*/


/*-----------------------------------------------------------------*
 *   Function Init_Decod_ld8a                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/

void WebRtcG729fix_Init_Decod_ld8a(Decod_ld8a_state *st)
{
  /* Initialize lsp_old[] */

  WEBRTC_SPL_MEMCPY_W16(st->lsp_old, WebRtcG729fix_lsp_old_reset, M);

  /* Initialize static pointer */

  st->exc = st->old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */

  WebRtcSpl_ZerosArrayW16(st->old_exc, PIT_MAX+L_INTERPOL);
  WebRtcSpl_ZerosArrayW16(st->mem_syn, M);

  st->sharp  = SHARPMIN;
  st->old_T0 = 60;
  st->gain_code = 0;
  st->gain_pitch = 0;

  WebRtcG729fix_Lsp_decw_reset(st);

  /* for G.729B */
  st->seed_fer = 21845;
  st->past_ftyp = 1;
  st->seed = INIT_SEED;
  st->sid_sav = 0;
  st->sh_sid_sav = 1;
  WebRtcG729fix_Init_lsfq_noise(st->noise_fg);

  /* Initialize Dec_gain */
  WEBRTC_SPL_MEMCPY_W16(st->past_qua_en, WebRtcG729fix_past_qua_en_reset, 4);

  return;
}

/*-----------------------------------------------------------------*
 *   Function Decod_ld8a                                           *
 *           ~~~~~~~~~~                                            *
 *   ->Main decoder routine.                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/

void WebRtcG729fix_Decod_ld8a(
  Decod_ld8a_state *st,
  int16_t  parm[],      /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)   */
  int16_t  synth[],     /* (o)   : synthesis speech                     */
  int16_t  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
  int16_t  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
  int16_t  *Vad,        /* (o)   : frame type                           */
  int16_t  bad_lsf      /* (i)   : bad LSF indicator                    */
)
{
  int16_t  *Az;                  /* Pointer on A_t   */
  int16_t  lsp_new[M];           /* LSPs             */
  int16_t  code[L_SUBFR];        /* ACELP codevector */

  /* Scalars */

  int16_t  i, j, i_subfr;
  int16_t  T0, T0_frac, index;
  int16_t  bfi;
  int32_t  L_temp;

  int16_t bad_pitch;             /* bad pitch indicator */

  /* for G.729B */
  int16_t ftyp;
  int16_t lsfq_mem[MA_NP][M];

  int Overflow;

  /* Test bad frame indicator (bfi) */

  bfi = *parm++;
  /* for G.729B */
  ftyp = *parm;

  if(bfi == 1) {
    if(st->past_ftyp == 1) {
      ftyp = 1;
      parm[4] = 1;    /* G.729 maintenance */
    }
    else ftyp = 0;
    *parm = ftyp;  /* modification introduced in version V1.3 */
  }
  
  *Vad = ftyp;

  /* Processing non active frames (SID & not transmitted) */
  if(ftyp != 1) {
    
    WebRtcG729fix_Get_decfreq_prev(st, lsfq_mem);
    WebRtcG729fix_Dec_cng(st, st->past_ftyp, st->sid_sav, st->sh_sid_sav,
            parm, st->exc, st->lsp_old, A_t, &st->seed, lsfq_mem);
    WebRtcG729fix_Update_decfreq_prev(st, lsfq_mem);

    Az = A_t;
    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
      Overflow = WebRtcG729fix_Syn_filt(Az, &st->exc[i_subfr], &synth[i_subfr], L_SUBFR, st->mem_syn, 0);
      if(Overflow != 0) {
        /* In case of overflow in the synthesis          */
        /* -> Scale down vector exc[] and redo synthesis */
        
        for (i = 0; i < PIT_MAX + L_INTERPOL + L_FRAME; i++)
          st->old_exc[i] = shr(st->old_exc[i], 2);
        
        WebRtcG729fix_Syn_filt(Az, &st->exc[i_subfr], &synth[i_subfr], L_SUBFR, st->mem_syn, 1);
      }
      else
        WEBRTC_SPL_MEMCPY_W16(st->mem_syn, &synth[i_subfr+L_SUBFR-M], M);
      
      Az += MP1;

      *T2++ = st->old_T0;
    }
    st->sharp = SHARPMIN;
    
  }
  /* Processing active frame */
  else {
    
    st->seed = INIT_SEED;
    parm++;

    /* Decode the LSPs */
    
    WebRtcG729fix_D_lsp(st, parm, lsp_new, WebRtcSpl_AddSatW16(bfi, bad_lsf));
    parm += 2;
    
    /*
       Note: "bad_lsf" is introduce in case the standard is used with
       channel protection.
       */
    
    /* Interpolation of LPC for the 2 subframes */
    
    WebRtcG729fix_Int_qlpc(st->lsp_old, lsp_new, A_t);
    
    /* update the LSFs for the next frame */
    
    WEBRTC_SPL_MEMCPY_W16(st->lsp_old, lsp_new, M);
    
    /*------------------------------------------------------------------------*
     *          Loop for every subframe in the analysis frame                 *
     *------------------------------------------------------------------------*
     * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR  *
     *  times                                                                 *
     *     - decode the pitch delay                                           *
     *     - decode algebraic code                                            *
     *     - decode pitch and codebook gains                                  *
     *     - find the excitation and compute synthesis speech                 *
     *------------------------------------------------------------------------*/
    
    Az = A_t;            /* pointer to interpolated LPC parameters */
    
    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
      {

        index = *parm++;        /* pitch index */

        if(i_subfr == 0)
          {
            i = *parm++;        /* get parity check result */
            bad_pitch = WebRtcSpl_AddSatW16(bfi, i);
            if( bad_pitch == 0)
              {
                WebRtcG729fix_Dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                st->old_T0 = T0;
              }
            else                /* Bad frame, or parity error */
              {
                T0  = st->old_T0;
                T0_frac = 0;
                st->old_T0 = WebRtcSpl_AddSatW16(st->old_T0, 1);
                if(st->old_T0 > PIT_MAX) {
                  st->old_T0 = PIT_MAX;
                }
              }
          }
        else                    /* second subframe */
          {
            if(bfi == 0)
              {
                WebRtcG729fix_Dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
                st->old_T0 = T0;
              }
            else
              {
                T0 = st->old_T0;
                T0_frac = 0;
                st->old_T0 = WebRtcSpl_AddSatW16(st->old_T0, 1);
                if (st->old_T0 > PIT_MAX) {
                  st->old_T0 = PIT_MAX;
                }
              }
          }
        *T2++ = T0;

        /*-------------------------------------------------*
         * - Find the adaptive codebook vector.            *
         *-------------------------------------------------*/

        WebRtcG729fix_Pred_lt_3(&st->exc[i_subfr], T0, T0_frac, L_SUBFR);

        /*-------------------------------------------------------*
         * - Decode innovative codebook.                         *
         * - Add the fixed-gain pitch contribution to code[].    *
         *-------------------------------------------------------*/

        if(bfi != 0)            /* Bad frame */
          {

            parm[0] = WebRtcG729fix_Random(&st->seed_fer) & (int16_t)0x1fff; /* 13 bits random */
            parm[1] = WebRtcG729fix_Random(&st->seed_fer) & (int16_t)0x000f; /*  4 bits random */
          }

        WebRtcG729fix_Decod_ACELP(parm[1], parm[0], code);
        parm +=2;

        j = shl(st->sharp, 1);      /* From Q14 to Q15 */
        if(T0 < L_SUBFR) {
          for (i = T0; i < L_SUBFR; i++) {
            code[i] = WebRtcSpl_AddSatW16(code[i], mult(code[i-T0], j));
          }
        }

        /*-------------------------------------------------*
         * - Decode pitch and codebook gains.              *
         *-------------------------------------------------*/

        index = *parm++;        /* index of energy VQ */

        WebRtcG729fix_Dec_gain(st, index, code, L_SUBFR, bfi, &st->gain_pitch, &st->gain_code);

        /*-------------------------------------------------------------*
         * - Update pitch sharpening "sharp" with quantized gain_pitch *
         *-------------------------------------------------------------*/

        st->sharp = st->gain_pitch;
        if (st->sharp > SHARPMAX)      { st->sharp = SHARPMAX; }
        else if (st->sharp < SHARPMIN) { st->sharp = SHARPMIN; }

        /*-------------------------------------------------------*
         * - Find the total excitation.                          *
         * - Find synthesis speech corresponding to exc[].       *
         *-------------------------------------------------------*/

        for (i = 0; i < L_SUBFR;  i++)
          {
            /* exc[i] = gain_pitch*exc[i] + gain_code*code[i]; */
            /* exc[i]  in Q0   gain_pitch in Q14               */
            /* code[i] in Q13  gain_codeode in Q1              */
            
            L_temp = L_mult(st->exc[i+i_subfr], st->gain_pitch);
            L_temp = L_mac(L_temp, code[i], st->gain_code);
            L_temp = L_shl(L_temp, 1);
            st->exc[i+i_subfr] = L_round(L_temp);
          }
        
        Overflow = WebRtcG729fix_Syn_filt(Az, &st->exc[i_subfr], &synth[i_subfr], L_SUBFR, st->mem_syn, 0);
        if(Overflow != 0)
          {
            /* In case of overflow in the synthesis          */
            /* -> Scale down vector exc[] and redo synthesis */

            for (i = 0; i < PIT_MAX + L_INTERPOL + L_FRAME; i++)
              st->old_exc[i] = shr(st->old_exc[i], 2);

            WebRtcG729fix_Syn_filt(Az, &st->exc[i_subfr], &synth[i_subfr], L_SUBFR, st->mem_syn, 1);
          }
        else
          WEBRTC_SPL_MEMCPY_W16(st->mem_syn, &synth[i_subfr+L_SUBFR-M], M);

        Az += MP1;              /* interpolated LPC parameters for next subframe */
      }
  }
  
  /*------------*
   *  For G729b
   *-----------*/
  if(bfi == 0) {
    L_temp = 0L;
    for (i = 0; i < L_FRAME; i++) {
      L_temp = L_mac(L_temp, st->exc[i], st->exc[i]);
    } /* may overflow => last level of SID quantizer */
    st->sh_sid_sav = WebRtcSpl_NormW32(L_temp);
    st->sid_sav = L_round(L_shl(L_temp, st->sh_sid_sav));
    st->sh_sid_sav = WebRtcSpl_SubSatW16(16, st->sh_sid_sav);
  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME  exc[]           *
  *--------------------------------------------------*/

  Copy(&st->old_exc[L_FRAME], &st->old_exc[0], PIT_MAX+L_INTERPOL);

  /* for G729b */
  st->past_ftyp = ftyp;

  return;
}

