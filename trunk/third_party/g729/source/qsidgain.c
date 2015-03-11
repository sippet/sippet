/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* Quantize SID gain                                      */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"

/* Local function */
static int16_t Quant_Energy(
  int32_t L_x,    /* (i)  : Energy                 */
  int16_t sh,     /* (i)  : Exponent of the energy */
  int16_t *enerq  /* (o)  : quantized energy in dB */
);

/*-------------------------------------------------------------------*
 * Function  Qua_Sidgain                                             *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*/
void WebRtcG729fix_Qua_Sidgain(
  int16_t *ener,     /* (i)   array of energies                   */
  int16_t *sh_ener,  /* (i)   corresponding scaling factors       */
  int16_t nb_ener,   /* (i)   number of energies or               */
  int16_t *enerq,    /* (o)   decoded energies in dB              */
  int16_t *idx       /* (o)   SID gain quantization index         */
)
{
  int16_t i;
  int32_t L_x;
  int16_t sh1, temp;
  int16_t hi, lo;
  int32_t L_acc;
  
  if(nb_ener == 0) {
    /* Quantize energy saved for frame erasure case                */
    /* L_x = average_ener                                          */
    L_acc = L_deposit_l(*ener);
    L_acc = L_shl(L_acc, *sh_ener); /* >> if *sh_ener < 0 */
    WebRtcG729fix_L_Extract(L_acc, &hi, &lo);
    L_x = WebRtcG729fix_Mpy_32_16(hi, lo, WebRtcG729fix_fact[0]);
    sh1 = 0;
  }
  else {
    
    /*
     * Compute weighted average of energies
     * ener[i] = enerR[i] x 2**sh_ener[i]
     * L_x = k[nb_ener] x SUM(i=0->nb_ener-1) enerR[i]
     * with k[nb_ener] =  fact_ener / nb_ener x L_FRAME x nbAcf
     */
    sh1 = sh_ener[0];
    for(i=1; i<nb_ener; i++) {
      if(sh_ener[i] < sh1) sh1 = sh_ener[i];
    }
    sh1 = WebRtcSpl_AddSatW16(sh1, (16-WebRtcG729fix_marg[nb_ener]));
    L_x = 0L;
    for(i=0; i<nb_ener; i++) {
      temp = WebRtcSpl_SubSatW16(sh1, sh_ener[i]);
      L_acc = L_deposit_l(ener[i]);
      L_acc = L_shl(L_acc, temp);
      L_x = WebRtcSpl_AddSatW32(L_x, L_acc);
    }
    WebRtcG729fix_L_Extract(L_x, &hi, &lo);
    L_x = WebRtcG729fix_Mpy_32_16(hi, lo, WebRtcG729fix_fact[i]);
  }
  
  *idx = Quant_Energy(L_x, sh1, enerq);
  
  return;
}


/* Local function */

static int16_t Quant_Energy(
  int32_t L_x,    /* (i)  : Energy                 */
  int16_t sh,     /* (i)  : Exponent of the energy */
  int16_t *enerq  /* (o)  : quantized energy in dB */
)
{

  int16_t exp, frac;
  int16_t e_tmp, temp, index;

  WebRtcG729fix_Log2(L_x, &exp, &frac);
  temp = WebRtcSpl_SubSatW16(exp, sh);
  e_tmp = shl(temp, 10);
  e_tmp = WebRtcSpl_AddSatW16(e_tmp, mult_r(frac, 1024)); /* 2^10 x log2(L_x . 2^-sh) */
  /* log2(ener) = 10log10(ener) / K */
  /* K = 10 Log2 / Log10 */

  temp = WebRtcSpl_SubSatW16(e_tmp, -2721);      /* -2721 -> -8dB */
  if(temp <= 0) {
    *enerq = -12;
    return(0);
  }

  temp = WebRtcSpl_SubSatW16(e_tmp, 22111);      /* 22111 -> 65 dB */  
  if(temp > 0) {
    *enerq = 66;
    return(31);
  }

  temp = WebRtcSpl_SubSatW16(e_tmp, 4762);       /* 4762 -> 14 dB */
  if(temp <= 0){
    e_tmp = WebRtcSpl_AddSatW16(e_tmp, 3401);
    index = mult(e_tmp, 24);
    if (index < 1) index = 1;
    *enerq = WebRtcSpl_SubSatW16(shl(index, 2), 8);
    return(index);
  }

  e_tmp = WebRtcSpl_SubSatW16(e_tmp, 340);
  index = WebRtcSpl_SubSatW16(shr(mult(e_tmp, 193), 2), 1);
  if (index < 6) index = 6;
  *enerq = WebRtcSpl_AddSatW16(shl(index, 1), 4);
  return(index);
}



