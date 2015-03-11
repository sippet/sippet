/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*-------------------------------------------------------------*
 *  Procedure Lsp_Az:                                          *
 *            ~~~~~~                                           *
 *   Compute the LPC coefficients from lsp (order=10)          *
 *-------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/* local function */

static void Get_lsp_pol(int16_t *lsp, int32_t *f);

void WebRtcG729fix_Lsp_Az(
  int16_t lsp[],    /* (i) Q15 : line spectral frequencies            */
  int16_t a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
)
{
  int16_t i;
  int32_t f1[6], f2[6];
  int32_t ff1, ff2, fff1, fff2;

  Get_lsp_pol(&lsp[0],f1);
  Get_lsp_pol(&lsp[1],f2);

  a[0] = 4096;
  for (i = 1; i <= 5; i++)
  {
    ff1  = WebRtcSpl_AddSatW32(f1[i], f1[i-1]);               /* f1[i] += f1[i-1];         */
    ff2  = WebRtcSpl_SubSatW32(f2[i], f2[i-1]);               /* f2[i] -= f2[i-1];         */

    fff1 = WebRtcSpl_AddSatW32(ff1, ff2);                     /* f1[i] + f2[i]             */
    fff2 = WebRtcSpl_SubSatW32(ff1, ff2);                     /* f1[i] - f2[i]             */

    a[i] = extract_l(L_shr_r(fff1, 13));        /* from Q24 to Q12 and * 0.5 */
    a[11-i] = extract_l(L_shr_r(fff2, 13));     /* from Q24 to Q12 and * 0.5 */
  }
}

/*-----------------------------------------------------------*
 * procedure Get_lsp_pol:                                    *
 *           ~~~~~~~~~~~                                     *
 *   Find the polynomial F1(z) or F2(z) from the LSPs        *
 *-----------------------------------------------------------*
 *                                                           *
 * Parameters:                                               *
 *  lsp[]   : line spectral freq. (cosine domain)    in Q15  *
 *  f[]     : the coefficients of F1 or F2           in Q24  *
 *-----------------------------------------------------------*/

static void Get_lsp_pol(int16_t *lsp, int32_t *f)
{
  int16_t i,j, hi, lo;
  int32_t t0;

   /* All computation in Q24 */

   *f = L_mult(4096, 2048);             /* f[0] = 1.0;             in Q24  */
   f++;
   *f = L_msu((int32_t)0, *lsp, 512);    /* f[1] =  -2.0 * lsp[0];  in Q24  */

   f++;
   lsp += 2;                            /* Advance lsp pointer             */

   for(i=2; i<=5; i++)
   {
     *f = f[-2];

     for(j=1; j<i; j++, f--)
     {
       WebRtcG729fix_L_Extract(f[-1] ,&hi, &lo);
       t0 = WebRtcG729fix_Mpy_32_16(hi, lo, *lsp);         /* t0 = f[-1] * lsp    */
       t0 = L_shl(t0, 1);
       *f = WebRtcSpl_AddSatW32(*f, f[-2]);                /* *f += f[-2]         */
       *f = WebRtcSpl_SubSatW32(*f, t0);                   /* *f -= t0            */
     }
     *f   = L_msu(*f, *lsp, 512);            /* *f -= lsp<<9        */
     f   += i;                               /* Advance f pointer   */
     lsp += 2;                               /* Advance lsp pointer */
   }
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions : Lsp_lsf and Lsf_lsp                                         |
 |                                                                           |
 |      Lsp_lsf   Transformation lsp to lsf                                  |
 |      Lsf_lsp   Transformation lsf to lsp                                  |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are       |
 |   approximated by a look-up table and interpolation.                      |
 |___________________________________________________________________________|
*/


void WebRtcG729fix_Lsf_lsp(
  int16_t lsf[],    /* (i) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  int16_t lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)                */
  int16_t m         /* (i)     : LPC order                                */
)
{
  int16_t i, ind, offset;
  int32_t L_tmp;

  for(i=0; i<m; i++)
  {
    ind    = shr(lsf[i], 8);               /* ind    = b8-b15 of lsf[i] */
    offset = lsf[i] & (int16_t)0x00ff;      /* offset = b0-b7  of lsf[i] */

    /* lsp[i] = table[ind]+ ((table[ind+1]-table[ind])*offset) / 256 */

    L_tmp   = L_mult(WebRtcSpl_SubSatW16(WebRtcG729fix_table[ind+1], WebRtcG729fix_table[ind]), offset);
    lsp[i] = WebRtcSpl_AddSatW16(WebRtcG729fix_table[ind], extract_l(L_shr(L_tmp, 9)));
  }
}


void WebRtcG729fix_Lsp_lsf(
  int16_t lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)                */
  int16_t lsf[],    /* (o) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  int16_t m         /* (i)     : LPC order                                */
)
{
  int16_t i, ind, tmp;
  int32_t L_tmp;

  ind = 63;    /* begin at end of table -1 */

  for(i= m-(int16_t)1; i >= 0; i--)
  {
    /* find value in table that is just greater than lsp[i] */
    while( WebRtcSpl_SubSatW16(WebRtcG729fix_table[ind], lsp[i]) < 0 )
    {
      ind = WebRtcSpl_SubSatW16(ind,1);
    }

    /* acos(lsp[i])= ind*256 + ( ( lsp[i]-table[ind] ) * slope[ind] )/4096 */

    L_tmp  = L_mult(WebRtcSpl_SubSatW16(lsp[i], WebRtcG729fix_table[ind]), WebRtcG729fix_slope[ind]);
    tmp = L_round(L_shl(L_tmp, 3));     /*(lsp[i]-table[ind])*slope[ind])>>12*/
    lsf[i] = WebRtcSpl_AddSatW16(tmp, shl(ind, 8));
  }
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions : Lsp_lsf and Lsf_lsp                                         |
 |                                                                           |
 |      Lsp_lsf   Transformation lsp to lsf                                  |
 |      Lsf_lsp   Transformation lsf to lsp                                  |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are       |
 |   approximated by a look-up table and interpolation.                      |
 |___________________________________________________________________________|
*/

void WebRtcG729fix_Lsf_lsp2(
  int16_t lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
  int16_t lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
  int16_t m         /* (i)     : LPC order                   */
)
{
  int16_t i, ind;
  int16_t offset;   /* in Q8 */
  int16_t freq;     /* normalized frequency in Q15 */
  int32_t L_tmp;

  for(i=0; i<m; i++)
  {
/*    freq = abs_s(freq);*/
    freq = mult(lsf[i], 20861);          /* 20861: 1.0/(2.0*PI) in Q17 */
    ind    = shr(freq, 8);               /* ind    = b8-b15 of freq */
    offset = freq & (int16_t)0x00ff;      /* offset = b0-b7  of freq */

    if (ind > 63){
      ind = 63;                 /* 0 <= ind <= 63 */
    }

    /* lsp[i] = table2[ind]+ (slope_cos[ind]*offset >> 12) */

    L_tmp   = L_mult(WebRtcG729fix_slope_cos[ind], offset);   /* L_tmp in Q28 */
    lsp[i] = WebRtcSpl_AddSatW16(WebRtcG729fix_table2[ind], extract_l(L_shr(L_tmp, 13)));

  }
}



void WebRtcG729fix_Lsp_lsf2(
  int16_t lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
  int16_t lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
  int16_t m         /* (i)     : LPC order                   */
)
{
  int16_t i, ind;
  int16_t offset;   /* in Q15 */
  int16_t freq;     /* normalized frequency in Q16 */
  int32_t L_tmp;

  ind = 63;           /* begin at end of table2 -1 */

  for(i= m-(int16_t)1; i >= 0; i--)
  {
    /* find value in table2 that is just greater than lsp[i] */
    while( WebRtcSpl_SubSatW16(WebRtcG729fix_table2[ind], lsp[i]) < 0 )
    {
      ind = WebRtcSpl_SubSatW16(ind,1);
      if ( ind <= 0 )
        break;
    }

    offset = WebRtcSpl_SubSatW16(lsp[i], WebRtcG729fix_table2[ind]);

    /* acos(lsp[i])= ind*512 + (slope_acos[ind]*offset >> 11) */

    L_tmp  = L_mult( WebRtcG729fix_slope_acos[ind], offset );   /* L_tmp in Q28 */
    freq = WebRtcSpl_AddSatW16(shl(ind, 9), extract_l(L_shr(L_tmp, 12)));
    lsf[i] = mult(freq, 25736);           /* 25736: 2.0*PI in Q12 */

  }
}

/*-------------------------------------------------------------*
 *  procedure Weight_Az                                        *
 *            ~~~~~~~~~                                        *
 * Weighting of LPC coefficients.                              *
 *   ap[i]  =  a[i] * (gamma ** i)                             *
 *                                                             *
 *-------------------------------------------------------------*/


void WebRtcG729fix_Weight_Az(
  int16_t a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
  int16_t gamma,    /* (i) Q15 : Spectral expansion factor.           */
  int16_t m,        /* (i)     : LPC order.                           */
  int16_t ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
)
{
  int16_t i, fac;

  ap[0] = a[0];
  fac   = gamma;
  for(i=1; i<m; i++)
  {
    ap[i] = L_round( L_mult(a[i], fac) );
    fac   = L_round( L_mult(fac, gamma) );
  }
  ap[m] = L_round( L_mult(a[m], fac) );
}

/*----------------------------------------------------------------------*
 * Function Int_qlpc()                                                  *
 * ~~~~~~~~~~~~~~~~~~~                                                  *
 * Interpolation of the LPC parameters.                                 *
 *----------------------------------------------------------------------*/

/* Interpolation of the quantized LSP's */

void WebRtcG729fix_Int_qlpc(
 int16_t lsp_old[], /* input : LSP vector of past frame              */
 int16_t lsp_new[], /* input : LSP vector of present frame           */
 int16_t Az[]       /* output: interpolated Az() for the 2 subframes */
)
{
  int16_t i;
  int16_t lsp[M];

  /*  lsp[i] = lsp_new[i] * 0.5 + lsp_old[i] * 0.5 */

  for (i = 0; i < M; i++) {
    lsp[i] = WebRtcSpl_AddSatW16(shr(lsp_new[i], 1), shr(lsp_old[i], 1));
  }

  WebRtcG729fix_Lsp_Az(lsp, Az);              /* Subframe 1 */

  WebRtcG729fix_Lsp_Az(lsp_new, &Az[MP1]);    /* Subframe 2 */
}

