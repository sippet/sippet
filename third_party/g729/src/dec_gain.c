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
#include "ld8a.h"
#include "tab_ld8a.h"

/*---------------------------------------------------------------------------*
 * Function  Dec_gain                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Decode the pitch and codebook gains                                       *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * input arguments:                                                          *
 *                                                                           *
 *   index      :Quantization index                                          *
 *   code[]     :Innovative code vector                                      *
 *   L_subfr    :Subframe size                                               *
 *   bfi        :Bad frame indicator                                         *
 *                                                                           *
 * output arguments:                                                         *
 *                                                                           *
 *   gain_pit   :Quantized pitch gain                                        *
 *   gain_cod   :Quantized codebook gain                                     *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void WebRtcG729fix_Dec_gain(
   Decod_ld8a_state *st,
   int16_t index,        /* (i)     :Index of quantization.         */
   int16_t code[],       /* (i) Q13 :Innovative vector.             */
   int16_t L_subfr,      /* (i)     :Subframe length.               */
   int16_t bfi,          /* (i)     :Bad frame indicator            */
   int16_t *gain_pit,    /* (o) Q14 :Pitch gain.                    */
   int16_t *gain_cod     /* (o) Q1  :Code gain.                     */
)
{
   int16_t  index1, index2, tmp;
   int16_t  gcode0, exp_gcode0;
   int32_t  L_gbk12, L_acc, L_accb;

   /*-------------- Case of erasure. ---------------*/

   if(bfi != 0){
      *gain_pit = mult( *gain_pit, 29491 );      /* *0.9 in Q15 */
      if (*gain_pit > 29491) *gain_pit = 29491;
      *gain_cod = mult( *gain_cod, 32111 );      /* *0.98 in Q15 */

     /*----------------------------------------------*
      * update table of past quantized energies      *
      *                              (frame erasure) *
      *----------------------------------------------*/
      WebRtcG729fix_Gain_update_erasure(st->past_qua_en);

      return;
   }

   /*-------------- Decode pitch gain ---------------*/

   index1 = WebRtcG729fix_imap1[ shr(index,NCODE2_B) ] ;
   index2 = WebRtcG729fix_imap2[ index & (NCODE2-1) ] ;
   *gain_pit = WebRtcSpl_AddSatW16( WebRtcG729fix_gbk1[index1][0], WebRtcG729fix_gbk2[index2][0] );

   /*-------------- Decode codebook gain ---------------*/

  /*---------------------------------------------------*
   *-  energy due to innovation                       -*
   *-  predicted energy                               -*
   *-  predicted codebook gain => gcode0[exp_gcode0]  -*
   *---------------------------------------------------*/

   WebRtcG729fix_Gain_predict(st->past_qua_en, code, L_subfr, &gcode0, &exp_gcode0 );

  /*-----------------------------------------------------------------*
   * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
   *-----------------------------------------------------------------*/

   L_acc = L_deposit_l( WebRtcG729fix_gbk1[index1][1] );
   L_accb = L_deposit_l( WebRtcG729fix_gbk2[index2][1] );
   L_gbk12 = WebRtcSpl_AddSatW32( L_acc, L_accb );                       /* Q13 */
   tmp = extract_l( L_shr( L_gbk12,1 ) );                  /* Q12 */
   L_acc = L_mult(tmp, gcode0);             /* Q[exp_gcode0+12+1] */

   L_acc = L_shl(L_acc, WebRtcSpl_AddSatW16( negate(exp_gcode0),(-12-1+1+16) ));
   *gain_cod = extract_h( L_acc );                          /* Q1 */

  /*----------------------------------------------*
   * update table of past quantized energies      *
   *----------------------------------------------*/
   WebRtcG729fix_Gain_update(st->past_qua_en, L_gbk12 );

   return;

}
