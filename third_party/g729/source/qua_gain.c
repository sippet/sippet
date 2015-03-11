/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

static void Gbk_presel(
   int16_t best_gain[],     /* (i) [0] Q9 : unquantized pitch gain     */
                           /* (i) [1] Q2 : unquantized code gain      */
   int16_t *cand1,          /* (o)    : index of best 1st stage vector */
   int16_t *cand2,          /* (o)    : index of best 2nd stage vector */
   int16_t gcode0           /* (i) Q4 : presearch for gain codebook    */
);


/*---------------------------------------------------------------------------*
 * Function  Qua_gain                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Inputs:                                                                   *
 *   code[]     :Innovative codebook.                                        *
 *   g_coeff[]  :Correlations compute for pitch.                             *
 *   L_subfr    :Subframe length.                                            *
 *                                                                           *
 * Outputs:                                                                  *
 *   gain_pit   :Quantized pitch gain.                                       *
 *   gain_cod   :Quantized code gain.                                        *
 *                                                                           *
 * Return:                                                                   *
 *   Index of quantization.                                                  *
 *                                                                           *
 *--------------------------------------------------------------------------*/
int16_t WebRtcG729fix_Qua_gain(
   Coder_ld8a_state *st,
   int16_t code[],       /* (i) Q13 :Innovative vector.             */
   int16_t g_coeff[],    /* (i)     :Correlations <xn y1> -2<y1 y1> */
                        /*            <y2,y2>, -2<xn,y2>, 2<y1,y2> */
   int16_t exp_coeff[],  /* (i)     :Q-Format g_coeff[]             */
   int16_t L_subfr,      /* (i)     :Subframe length.               */
   int16_t *gain_pit,    /* (o) Q14 :Pitch gain.                    */
   int16_t *gain_cod,    /* (o) Q1  :Code gain.                     */
   int16_t tameflag      /* (i)     : set to 1 if taming is needed  */
)
{
   int16_t  i, j, index1, index2;
   int16_t  cand1, cand2;
   int16_t  exp, gcode0, exp_gcode0, gcode0_org, e_min ;
   int16_t  nume, denom, inv_denom;
   int16_t  exp1,exp2,exp_nume,exp_denom,exp_inv_denom,sft,tmp;
   int16_t  g_pitch, g2_pitch, g_code, g2_code, g_pit_cod;
   int16_t  coeff[5], coeff_lsf[5];
   int16_t  exp_min[5];
   int32_t  L_gbk12;
   int32_t  L_tmp, L_dist_min, L_temp, L_tmp1, L_tmp2, L_acc, L_accb;
   int16_t  best_gain[2];

  /*---------------------------------------------------*
   *-  energy due to innovation                       -*
   *-  predicted energy                               -*
   *-  predicted codebook gain => gcode0[exp_gcode0]  -*
   *---------------------------------------------------*/

   WebRtcG729fix_Gain_predict(st->past_qua_en, code, L_subfr, &gcode0, &exp_gcode0);

  /*-----------------------------------------------------------------*
   *  pre-selection                                                  *
   *-----------------------------------------------------------------*/
  /*-----------------------------------------------------------------*
   *  calculate best gain                                            *
   *                                                                 *
   *  tmp = -1./(4.*coeff[0]*coeff[2]-coeff[4]*coeff[4]) ;           *
   *  best_gain[0] = (2.*coeff[2]*coeff[1]-coeff[3]*coeff[4])*tmp ;  *
   *  best_gain[1] = (2.*coeff[0]*coeff[3]-coeff[1]*coeff[4])*tmp ;  *
   *  gbk_presel(best_gain,&cand1,&cand2,gcode0) ;                   *
   *                                                                 *
   *-----------------------------------------------------------------*/

  /*-----------------------------------------------------------------*
   *  tmp = -1./(4.*coeff[0]*coeff[2]-coeff[4]*coeff[4]) ;           *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[0], g_coeff[2] );
   exp1   = WebRtcSpl_AddSatW16( WebRtcSpl_AddSatW16( exp_coeff[0], exp_coeff[2] ), 1-2 );
   L_tmp2 = L_mult( g_coeff[4], g_coeff[4] );
   exp2   = WebRtcSpl_AddSatW16( WebRtcSpl_AddSatW16( exp_coeff[4], exp_coeff[4] ), 1 );

   if(exp1 > exp2){
      L_tmp = WebRtcSpl_SubSatW32( L_shr( L_tmp1, WebRtcSpl_SubSatW16(exp1,exp2) ), L_tmp2 );
      exp = exp2;
   }
   else{
      L_tmp = WebRtcSpl_SubSatW32( L_tmp1, L_shr( L_tmp2, WebRtcSpl_SubSatW16(exp2,exp1) ) );
      exp = exp1;
   }
   sft = WebRtcSpl_NormW32( L_tmp );
   denom = extract_h( L_shl(L_tmp, sft) );
   exp_denom = WebRtcSpl_SubSatW16( WebRtcSpl_AddSatW16( exp, sft ), 16 );

   inv_denom = div_s(16384,denom);
   inv_denom = negate( inv_denom );
   exp_inv_denom = WebRtcSpl_SubSatW16( 14+15, exp_denom );

  /*-----------------------------------------------------------------*
   *  best_gain[0] = (2.*coeff[2]*coeff[1]-coeff[3]*coeff[4])*tmp ;  *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[2], g_coeff[1] );
   exp1   = WebRtcSpl_AddSatW16( exp_coeff[2], exp_coeff[1] );
   L_tmp2 = L_mult( g_coeff[3], g_coeff[4] );
   exp2   = WebRtcSpl_AddSatW16( WebRtcSpl_AddSatW16( exp_coeff[3], exp_coeff[4] ), 1 );

   if(exp1 > exp2){
      L_tmp = WebRtcSpl_SubSatW32( L_shr( L_tmp1, WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(exp1,exp2),1 )), L_shr( L_tmp2,1 ) );
      exp = WebRtcSpl_SubSatW16(exp2,1);
   }
   else{
      L_tmp = WebRtcSpl_SubSatW32( L_shr( L_tmp1,1 ), L_shr( L_tmp2, WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(exp2,exp1),1 )) );
      exp = WebRtcSpl_SubSatW16(exp1,1);
   }
   sft = WebRtcSpl_NormW32( L_tmp );
   nume = extract_h( L_shl(L_tmp, sft) );
   exp_nume = WebRtcSpl_SubSatW16( WebRtcSpl_AddSatW16( exp, sft ), 16 );

   sft = WebRtcSpl_SubSatW16( WebRtcSpl_AddSatW16( exp_nume, exp_inv_denom ), (9+16-1) );
   L_acc = L_shr( L_mult( nume,inv_denom ), sft );
   best_gain[0] = extract_h( L_acc );             /*-- best_gain[0]:Q9 --*/

   if (tameflag == 1){
     if(best_gain[0] > GPCLIP2) best_gain[0] = GPCLIP2;
   }

  /*-----------------------------------------------------------------*
   *  best_gain[1] = (2.*coeff[0]*coeff[3]-coeff[1]*coeff[4])*tmp ;  *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[0], g_coeff[3] );
   exp1   = WebRtcSpl_AddSatW16( exp_coeff[0], exp_coeff[3] ) ;
   L_tmp2 = L_mult( g_coeff[1], g_coeff[4] );
   exp2   = WebRtcSpl_AddSatW16( WebRtcSpl_AddSatW16( exp_coeff[1], exp_coeff[4] ), 1 );

   if(exp1 > exp2){
      L_tmp = WebRtcSpl_SubSatW32( L_shr( L_tmp1, WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(exp1,exp2),1) ), L_shr( L_tmp2,1 ) );
      exp = WebRtcSpl_SubSatW16(exp2,1);
   }
   else{
      L_tmp = WebRtcSpl_SubSatW32( L_shr( L_tmp1,1 ), L_shr( L_tmp2, WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(exp2,exp1),1) ) );
      exp = WebRtcSpl_SubSatW16(exp1,1);
   }
   sft = WebRtcSpl_NormW32( L_tmp );
   nume = extract_h( L_shl(L_tmp, sft) );
   exp_nume = WebRtcSpl_SubSatW16( WebRtcSpl_AddSatW16( exp, sft ), 16 );

   sft = WebRtcSpl_SubSatW16( WebRtcSpl_AddSatW16( exp_nume, exp_inv_denom ), (2+16-1) );
   L_acc = L_shr( L_mult( nume,inv_denom ), sft );
   best_gain[1] = extract_h( L_acc );             /*-- best_gain[1]:Q2 --*/

   /*--- Change Q-format of gcode0 ( Q[exp_gcode0] -> Q4 ) ---*/
   if(exp_gcode0 >= 4){
      gcode0_org = shr( gcode0, WebRtcSpl_SubSatW16(exp_gcode0,4) );
   }
   else{
      L_acc = L_deposit_l( gcode0 );
      L_acc = L_shl( L_acc, WebRtcSpl_SubSatW16( (4+16), exp_gcode0 ) );
      gcode0_org = extract_h( L_acc );              /*-- gcode0_org:Q4 --*/
   }

  /*----------------------------------------------*
   *   - presearch for gain codebook -            *
   *----------------------------------------------*/

   Gbk_presel(best_gain, &cand1, &cand2, gcode0_org );

/*---------------------------------------------------------------------------*
 *                                                                           *
 * Find the best quantizer.                                                  *
 *                                                                           *
 *  dist_min = WEBRTC_SPL_WORD32_MAX;                                                       *
 *  for ( i=0 ; i<NCAN1 ; i++ ){                                             *
 *    for ( j=0 ; j<NCAN2 ; j++ ){                                           *
 *      g_pitch = gbk1[cand1+i][0] + gbk2[cand2+j][0];                       *
 *      g_code = gcode0 * (gbk1[cand1+i][1] + gbk2[cand2+j][1]);             *
 *      dist = g_pitch*g_pitch * coeff[0]                                    *
 *           + g_pitch         * coeff[1]                                    *
 *           + g_code*g_code   * coeff[2]                                    *
 *           + g_code          * coeff[3]                                    *
 *           + g_pitch*g_code  * coeff[4] ;                                  *
 *                                                                           *
 *      if (dist < dist_min){                                                *
 *        dist_min = dist;                                                   *
 *        indice1 = cand1 + i ;                                              *
 *        indice2 = cand2 + j ;                                              *
 *      }                                                                    *
 *    }                                                                      *
 *  }                                                                        *
 *                                                                           *
 * g_pitch         = Q13                                                     *
 * g_pitch*g_pitch = Q11:(13+13+1-16)                                        *
 * g_code          = Q[exp_gcode0-3]:(exp_gcode0+(13-1)+1-16)                *
 * g_code*g_code   = Q[2*exp_gcode0-21]:(exp_gcode0-3+exp_gcode0-3+1-16)     *
 * g_pitch*g_code  = Q[exp_gcode0-5]:(13+exp_gcode0-3+1-16)                  *
 *                                                                           *
 * term 0: g_pitch*g_pitch*coeff[0] ;exp_min0 = 13             +exp_coeff[0] *
 * term 1: g_pitch        *coeff[1] ;exp_min1 = 14             +exp_coeff[1] *
 * term 2: g_code*g_code  *coeff[2] ;exp_min2 = 2*exp_gcode0-21+exp_coeff[2] *
 * term 3: g_code         *coeff[3] ;exp_min3 = exp_gcode0  - 3+exp_coeff[3] *
 * term 4: g_pitch*g_code *coeff[4] ;exp_min4 = exp_gcode0  - 4+exp_coeff[4] *
 *                                                                           *
 *---------------------------------------------------------------------------*/

   exp_min[0] = WebRtcSpl_AddSatW16( exp_coeff[0], 13 );
   exp_min[1] = WebRtcSpl_AddSatW16( exp_coeff[1], 14 );
   exp_min[2] = WebRtcSpl_AddSatW16( exp_coeff[2], WebRtcSpl_SubSatW16( shl( exp_gcode0, 1 ), 21 ) );
   exp_min[3] = WebRtcSpl_AddSatW16( exp_coeff[3], WebRtcSpl_SubSatW16( exp_gcode0, 3 ) );
   exp_min[4] = WebRtcSpl_AddSatW16( exp_coeff[4], WebRtcSpl_SubSatW16( exp_gcode0, 4 ) );

   e_min = exp_min[0];
   for(i=1; i<5; i++){
      if(exp_min[i] < e_min){
         e_min = exp_min[i];
      }
   }

   /* align coeff[] and save in special 32 bit double precision */

   for(i=0; i<5; i++){
     j = WebRtcSpl_SubSatW16( exp_min[i], e_min );
     L_tmp = L_deposit_h( g_coeff[i] );
     L_tmp = L_shr( L_tmp, j );          /* L_tmp:Q[exp_g_coeff[i]+16-j] */
     WebRtcG729fix_L_Extract( L_tmp, &coeff[i], &coeff_lsf[i] );          /* DPF */
   }

   /* Codebook search */

   L_dist_min = WEBRTC_SPL_WORD32_MAX;

   /* initialization used only to suppress Microsoft Visual C++  warnings */
   index1 = cand1;
   index2 = cand2;

if(tameflag == 1){
   for(i=0; i<NCAN1; i++){
      for(j=0; j<NCAN2; j++){
         g_pitch = WebRtcSpl_AddSatW16(WebRtcG729fix_gbk1[cand1+i][0],
             WebRtcG729fix_gbk2[cand2+j][0]);                     /* Q14 */
         if(g_pitch < GP0999) {
         L_acc = L_deposit_l(WebRtcG729fix_gbk1[cand1+i][1]);
         L_accb = L_deposit_l(WebRtcG729fix_gbk2[cand2+j][1] );   /* Q13 */
         L_tmp = WebRtcSpl_AddSatW32(L_acc, L_accb);
         tmp = extract_l(L_shr(L_tmp, 1));                        /* Q12 */

         g_code   = mult(gcode0, tmp);           /*  Q[exp_gcode0+12-15] */
         g2_pitch = mult(g_pitch, g_pitch);                       /* Q13 */
         g2_code  = mult(g_code,  g_code);       /* Q[2*exp_gcode0-6-15] */
         g_pit_cod= mult(g_code,  g_pitch);      /* Q[exp_gcode0-3+14-15] */

         L_tmp = WebRtcG729fix_Mpy_32_16(coeff[0], coeff_lsf[0], g2_pitch);
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[2], coeff_lsf[2], g2_code) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[3], coeff_lsf[3], g_code) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod) );

         L_temp = WebRtcSpl_SubSatW32(L_tmp, L_dist_min);

         if( L_temp < 0L ){
            L_dist_min = L_tmp;
            index1 = WebRtcSpl_AddSatW16(cand1,i);
            index2 = WebRtcSpl_AddSatW16(cand2,j);
         }
        }
      }
   }

}
else{
   for(i=0; i<NCAN1; i++){
      for(j=0; j<NCAN2; j++){
         g_pitch = WebRtcSpl_AddSatW16(WebRtcG729fix_gbk1[cand1+i][0],
             WebRtcG729fix_gbk2[cand2+j][0]);                     /* Q14 */
         L_acc = L_deposit_l(WebRtcG729fix_gbk1[cand1+i][1]);
         L_accb = L_deposit_l(WebRtcG729fix_gbk2[cand2+j][1]);    /* Q13 */
         L_tmp = WebRtcSpl_AddSatW32( L_acc,L_accb );
         tmp = extract_l( L_shr( L_tmp,1 ) );                     /* Q12 */

         g_code   = mult( gcode0, tmp );         /*  Q[exp_gcode0+12-15] */
         g2_pitch = mult(g_pitch, g_pitch);                       /* Q13 */
         g2_code  = mult(g_code,  g_code);       /* Q[2*exp_gcode0-6-15] */
         g_pit_cod= mult(g_code,  g_pitch);      /* Q[exp_gcode0-3+14-15] */

         L_tmp = WebRtcG729fix_Mpy_32_16(coeff[0], coeff_lsf[0], g2_pitch);
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[2], coeff_lsf[2], g2_code) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[3], coeff_lsf[3], g_code) );
         L_tmp = WebRtcSpl_AddSatW32(L_tmp, WebRtcG729fix_Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod) );

         L_temp = WebRtcSpl_SubSatW32(L_tmp, L_dist_min);

         if( L_temp < 0L ){
            L_dist_min = L_tmp;
            index1 = WebRtcSpl_AddSatW16(cand1,i);
            index2 = WebRtcSpl_AddSatW16(cand2,j);
         }

      }
   }
}
   /* Read the quantized gain */

  /*-----------------------------------------------------------------*
   * *gain_pit = gbk1[indice1][0] + gbk2[indice2][0];                *
   *-----------------------------------------------------------------*/
   *gain_pit = WebRtcSpl_AddSatW16(WebRtcG729fix_gbk1[index1][0],
       WebRtcG729fix_gbk2[index2][0] );                       /* Q14 */

  /*-----------------------------------------------------------------*
   * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
   *-----------------------------------------------------------------*/
   L_acc = L_deposit_l(WebRtcG729fix_gbk1[index1][1]);
   L_accb = L_deposit_l(WebRtcG729fix_gbk2[index2][1]);
   L_gbk12 = WebRtcSpl_AddSatW32(L_acc, L_accb);              /* Q13 */
   tmp = extract_l(L_shr(L_gbk12, 1));                        /* Q12 */
   L_acc = L_mult(tmp, gcode0);                /* Q[exp_gcode0+12+1] */

   L_acc = L_shl(L_acc, WebRtcSpl_AddSatW16(negate(exp_gcode0), (-12-1+1+16)));
   *gain_cod = extract_h(L_acc);                              /* Q1 */

  /*----------------------------------------------*
   * update table of past quantized energies      *
   *----------------------------------------------*/
   WebRtcG729fix_Gain_update(st->past_qua_en, L_gbk12);

   return WebRtcSpl_AddSatW16(WebRtcG729fix_map1[index1]*(int16_t)NCODE2,
       WebRtcG729fix_map2[index2]);
}

/*---------------------------------------------------------------------------*
 * Function Gbk_presel                                                       *
 * ~~~~~~~~~~~~~~~~~~~                                                       *
 *   - presearch for gain codebook -                                         *
 *---------------------------------------------------------------------------*/
static void Gbk_presel(
   int16_t best_gain[],     /* (i) [0] Q9 : unquantized pitch gain     */
                           /* (i) [1] Q2 : unquantized code gain      */
   int16_t *cand1,          /* (o)    : index of best 1st stage vector */
   int16_t *cand2,          /* (o)    : index of best 2nd stage vector */
   int16_t gcode0           /* (i) Q4 : presearch for gain codebook    */
)
{
   int16_t    acc_h;
   int16_t    sft_x,sft_y;
   int32_t    L_acc,L_preg,L_cfbg,L_tmp,L_tmp_x,L_tmp_y;
   int32_t L_temp;

 /*--------------------------------------------------------------------------*
   x = (best_gain[1]-(coef[0][0]*best_gain[0]+coef[1][1])*gcode0) * inv_coef;
  *--------------------------------------------------------------------------*/
   L_cfbg = L_mult(WebRtcG729fix_coef[0][0], best_gain[0]); /* L_cfbg:Q20 -> !!y */
   L_acc = L_shr(WebRtcG729fix_L_coef[1][1], 15);      /* L_acc:Q20     */
   L_acc = WebRtcSpl_AddSatW32(L_cfbg, L_acc);
   acc_h = extract_h(L_acc);                           /* acc_h:Q4      */
   L_preg = L_mult(acc_h, gcode0);                     /* L_preg:Q9     */
   L_acc = L_shl(L_deposit_l(best_gain[1]), 7);        /* L_acc:Q9      */
   L_acc = WebRtcSpl_SubSatW32(L_acc, L_preg);
   acc_h = extract_h(L_shl(L_acc, 2));                 /* L_acc_h:Q[-5] */
   L_tmp_x = L_mult(acc_h, INV_COEF);                  /* L_tmp_x:Q15   */

 /*--------------------------------------------------------------------------*
   y = (coef[1][0]*(-coef[0][1]+best_gain[0]*coef[0][0])*gcode0
                                      -coef[0][0]*best_gain[1]) * inv_coef;
  *--------------------------------------------------------------------------*/
   L_acc = L_shr(WebRtcG729fix_L_coef[0][1], 10);      /* L_acc:Q20   */
   L_acc = WebRtcSpl_SubSatW32(L_cfbg, L_acc);   /* !!x -> L_cfbg:Q20 */
   acc_h = extract_h( L_acc );                         /* acc_h:Q4    */
   acc_h = mult( acc_h, gcode0 );                      /* acc_h:Q[-7] */
   L_tmp = L_mult(acc_h, WebRtcG729fix_coef[1][0] );   /* L_tmp:Q10   */

   L_preg = L_mult(WebRtcG729fix_coef[0][0], best_gain[1]); /* L_preg:Q13  */
   L_acc = WebRtcSpl_SubSatW32(L_tmp, L_shr(L_preg, 3));    /* L_acc:Q10   */

   acc_h = extract_h(L_shl(L_acc, 2));                 /* acc_h:Q[-4] */
   L_tmp_y = L_mult(acc_h, INV_COEF);                  /* L_tmp_y:Q16 */

   sft_y = (14+4+1)-16;         /* (Q[thr1]+Q[gcode0]+1)-Q[L_tmp_y] */
   sft_x = (15+4+1)-15;         /* (Q[thr2]+Q[gcode0]+1)-Q[L_tmp_x] */

   if (gcode0 > 0) {
      /*-- pre select codebook #1 --*/
      *cand1 = 0 ;
      do {
        L_temp = WebRtcSpl_SubSatW32(L_tmp_y,
            L_shr(L_mult(WebRtcG729fix_thr1[*cand1], gcode0), sft_y));
        if (L_temp > 0L) {
          (*cand1) = WebRtcSpl_AddSatW16(*cand1,1);
        }
        else {
          break;
        }
      } while (*cand1 < NCODE1-NCAN1);
      /*-- pre select codebook #2 --*/
      *cand2 = 0 ;
      do {
        L_temp = WebRtcSpl_SubSatW32(L_tmp_x,
            L_shr(L_mult(WebRtcG729fix_thr2[*cand2], gcode0), sft_x));
        if (L_temp > 0L) {
          (*cand2) = WebRtcSpl_AddSatW16(*cand2, 1);
        }
        else {
          break;
        }
      } while (*cand2 < NCODE2-NCAN2);
   }
   else {
      /*-- pre select codebook #1 --*/
      *cand1 = 0 ;
      do {
        L_temp = WebRtcSpl_SubSatW32(L_tmp_y,
            L_shr(L_mult(WebRtcG729fix_thr1[*cand1], gcode0), sft_y));
        if (L_temp < 0L) {
          (*cand1) = WebRtcSpl_AddSatW16(*cand1, 1);
        }
        else {
          break;
        }
      } while (*cand1 != NCODE1-NCAN1);
      /*-- pre select codebook #2 --*/
      *cand2 = 0 ;
      do {
        L_temp = WebRtcSpl_SubSatW32(L_tmp_x,
            L_shr(L_mult(WebRtcG729fix_thr2[*cand2], gcode0), sft_x));
        if (L_temp < 0L) {
          (*cand2) = WebRtcSpl_AddSatW16(*cand2,1);
        }
        else {
          break;
        }
      } while (*cand2 != NCODE2-NCAN2);
   }
}

