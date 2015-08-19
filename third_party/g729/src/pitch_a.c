/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729A  Version 1.1    Last modified: September 1996 */

/*
   ITU-T G.729A Speech Coder     ANSI-C Source Code
   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*---------------------------------------------------------------------------*
 * Pitch related functions                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 *---------------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "tab_ld8a.h"

static int16_t Compute_nrj_max(int16_t *scal_sig, int16_t L_frame, int32_t max)
{
  int32_t sum;
  int16_t  max_h, max_l, ener_h, ener_l;
  int16_t i;

  sum = 0;
  for(i=0; i<L_frame; i+=2)
    sum += scal_sig[i] * scal_sig[i];
  sum <<= 1;
  sum++; /* to avoid division by zero */

  /* max1 = max/sqrt(energy)                  */
  /* This result will always be on 16 bits !! */

  sum = WebRtcG729fix_Inv_sqrt(sum);    /* 1/sqrt(energy),    result in Q30 */
  //L_Extract(max, &max_h, &max_l);
  //L_Extract(sum, &ener_h, &ener_l);
  max_h = (int16_t) (max >> 16);
  max_l = (int16_t)((max >> 1) - (max_h << 15));
  ener_h = (int16_t) (sum >> 16);
  ener_l = (int16_t)((sum >> 1) - (ener_h << 15));
  //sum  = Mpy_32(max_h, max_l, ener_h, ener_l);
  sum = (((int32_t)max_h*ener_h)<<1) +
        (( (((int32_t)max_h*ener_l)>>15) + (((int32_t)max_l*ener_h)>>15) )<<1);

  return (int16_t)sum;
}

/*---------------------------------------------------------------------------*
 * Function  Pitch_ol_fast                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 * Compute the open loop pitch lag. (fast version)                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/


int16_t WebRtcG729fix_Pitch_ol_fast(  /* output: open loop pitch lag                        */
   int16_t signal[],    /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
   int16_t   pit_max,   /* input : maximum pitch lag                          */
   int16_t   L_frame    /* input : length of frame to compute pitch           */
)
{
  int16_t  i, j;
  int16_t  max1, max2, max3;
  int16_t  T1, T2, T3;
  int16_t  *p, *p1;
  int32_t  max, sum, sum1;

  /* Scaled signal */

  int16_t scaled_signal[L_FRAME+PIT_MAX];
  int16_t *scal_sig;

  scal_sig = &scaled_signal[pit_max];

  /*--------------------------------------------------------*
   *  Verification for risk of overflow.                    *
   *--------------------------------------------------------*/

   sum = 0;
   for(i= -pit_max; i< L_frame; i+=2)
   {
     sum += ((int32_t)signal[i] * (int32_t)signal[i]) << 1;
     if (sum < 0)  // overflow
     {
       sum = WEBRTC_SPL_WORD32_MAX;
       break;
     }
   }

  /*--------------------------------------------------------*
   * Scaling of input signal.                               *
   *                                                        *
   *   if overflow        -> scal_sig[i] = signal[i]>>3     *
   *   else if sum < 1^20 -> scal_sig[i] = signal[i]<<3     *
   *   else               -> scal_sig[i] = signal[i]        *
   *--------------------------------------------------------*/
   if (sum == WEBRTC_SPL_WORD32_MAX)
   {
     for(i=-pit_max; i<L_frame; i++)
       scal_sig[i] = signal[i] >> 3;
   }
   else {
     if (sum < (int32_t)0x100000) /* if (sum < 2^20) */
     {
        for(i=-pit_max; i<L_frame; i++)
          scal_sig[i] = signal[i] << 3;
     }
     else
     {
       for(i=-pit_max; i<L_frame; i++)
         scal_sig[i] = signal[i];
     }
   }

   /*--------------------------------------------------------------------*
    *  The pitch lag search is divided in three sections.                *
    *  Each section cannot have a pitch multiple.                        *
    *  We find a maximum for each section.                               *
    *  We compare the maxima of each section by favoring small lag.      *
    *                                                                    *
    *  First section:  lag delay = 20 to 39                              *
    *  Second section: lag delay = 40 to 79                              *
    *  Third section:  lag delay = 80 to 143                             *
    *--------------------------------------------------------------------*/

    /* First section */

    max = WEBRTC_SPL_WORD32_MIN;
    T1  = 20;    /* Only to remove warning from some compilers */
    for (i = 20; i < 40; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
          sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T1 = i;   }
    }

    /* compute energy of maximum */
    max1 = Compute_nrj_max(&scal_sig[-T1], L_frame, max);

    /* Second section */

    max = WEBRTC_SPL_WORD32_MIN;
    T2  = 40;    /* Only to remove warning from some compilers */
    for (i = 40; i < 80; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
          sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T2 = i;   }
    }

    /* compute energy of maximum */
    max2 = Compute_nrj_max(&scal_sig[-T2], L_frame, max);

    /* Third section */

    max = WEBRTC_SPL_WORD32_MIN;
    T3  = 80;    /* Only to remove warning from some compilers */
    for (i = 80; i < 143; i+=2) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
            sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T3 = i;   }
    }

     /* Test around max3 */
     i = T3;
     sum = 0;
     sum1 = 0;
     for (j=0; j<L_frame; j+=2)
     {
       sum  += scal_sig[j] * scal_sig[-(i+1) + j];
       sum1 += scal_sig[j] * scal_sig[-(i-1) + j];
     }
     sum  <<= 1;
     sum1 <<= 1;

     if (sum  > max) { max = sum;  T3 = i+(int16_t)1;   }
     if (sum1 > max) { max = sum1; T3 = i-(int16_t)1;   }

    /* compute energy of maximum */
    max3 = Compute_nrj_max(&scal_sig[-T3], L_frame, max);

   /*-----------------------*
    * Test for multiple.    *
    *-----------------------*/

    /* if( abs(T2*2 - T3) < 5)  */
    /*    max2 += max3 * 0.25;  */
    i = T2*2 - T3;
    if (abs_s(i) < 5)
      max2 += max3 >> 2;

    /* if( abs(T2*3 - T3) < 7)  */
    /*    max2 += max3 * 0.25;  */
    i += T2;
    if (abs_s(i) < 7)
      max2 += max3 >> 2;

    /* if( abs(T1*2 - T2) < 5)  */
    /*    max1 += max2 * 0.20;  */
    i = T1 * 2 - T2;
    if (abs_s(i) < 5)
      max1 += mult(max2, 6554);

    /* if( abs(T1*3 - T2) < 7)  */
    /*    max1 += max2 * 0.20;  */

    i += T1;
    if (abs_s(i) < 7)
      max1 += mult(max2, 6554);

   /*--------------------------------------------------------------------*
    * Compare the 3 sections maxima.                                     *
    *--------------------------------------------------------------------*/

    if( max1 < max2 ) {max1 = max2; T1 = T2;  }
    if( max1 < max3 )  {T1 = T3; }

    return T1;
}



/*--------------------------------------------------------------------------*
 *  Function  Dot_Product()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~                                                  *
 *--------------------------------------------------------------------------*/

static int32_t Dot_Product(/* (o)   :Result of scalar product. */
       int16_t   x[],     /* (i)   :First vector.             */
       int16_t   y[],     /* (i)   :Second vector.            */
       int16_t   lg       /* (i)   :Number of point.          */
)
{
  int16_t i;
  int32_t sum;

  sum = 0;
  for (i = 0; i < lg; i++)
    sum = L_mac(sum, x[i], y[i]);

  return sum;
}


/*--------------------------------------------------------------------------*
 *  Function  Pitch_fr3_fast()                                              *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~                                              *
 * Fast version of the pitch close loop.                                    *
 *--------------------------------------------------------------------------*/

int16_t WebRtcG729fix_Pitch_fr3_fast(/* (o)     : pitch period.            */
  int16_t exc[],       /* (i)     : excitation buffer                      */
  int16_t xn[],        /* (i)     : target vector                          */
  int16_t h[],         /* (i) Q12 : impulse response of filters.           */
  int16_t L_subfr,     /* (i)     : Length of subframe                     */
  int16_t t0_min,      /* (i)     : minimum value in the searched range.   */
  int16_t t0_max,      /* (i)     : maximum value in the searched range.   */
  int16_t i_subfr,     /* (i)     : indicator for first subframe.          */
  int16_t *pit_frac    /* (o)     : chosen fraction.                       */
)
{
  int16_t t, t0;
  int16_t Dn[L_SUBFR];
  int16_t exc_tmp[L_SUBFR];
  int32_t max, corr, L_temp;

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  WebRtcG729fix_Cor_h_X(h, xn, Dn);

 /*-----------------------------------------------------------------*
  * Find maximum integer delay.                                     *
  *-----------------------------------------------------------------*/

  max = WEBRTC_SPL_WORD32_MIN;
  t0 = t0_min; /* Only to remove warning from some compilers */

  for(t=t0_min; t<=t0_max; t++)
  {
    corr = Dot_Product(Dn, &exc[-t], L_subfr);
    L_temp = WebRtcSpl_SubSatW32(corr, max);
    if(L_temp > 0) {max = corr; t0 = t;  }
  }

 /*-----------------------------------------------------------------*
  * Test fractions.                                                 *
  *-----------------------------------------------------------------*/

  /* Fraction 0 */

  WebRtcG729fix_Pred_lt_3(exc, t0, 0, L_subfr);
  max = Dot_Product(Dn, exc, L_subfr);
  *pit_frac = 0;

  /* If first subframe and lag > 84 do not search fractional pitch */

  if( (i_subfr == 0) && (WebRtcSpl_SubSatW16(t0, 84) > 0) )
    return t0;

  WEBRTC_SPL_MEMCPY_W16(exc_tmp, exc, L_subfr);

  /* Fraction -1/3 */

  WebRtcG729fix_Pred_lt_3(exc, t0, -1, L_subfr);
  corr = Dot_Product(Dn, exc, L_subfr);
  L_temp = WebRtcSpl_SubSatW32(corr, max);
  if(L_temp > 0) {
     max = corr;
     *pit_frac = -1;
     WEBRTC_SPL_MEMCPY_W16(exc_tmp, exc, L_subfr);
  }

  /* Fraction +1/3 */

  WebRtcG729fix_Pred_lt_3(exc, t0, 1, L_subfr);
  corr = Dot_Product(Dn, exc, L_subfr);
  L_temp = WebRtcSpl_SubSatW32(corr, max);
  if(L_temp > 0) {
     max = corr;
     *pit_frac =  1;
  }
  else
    WEBRTC_SPL_MEMCPY_W16(exc, exc_tmp, L_subfr);

  return t0;
}

/*---------------------------------------------------------------------*
 * Function  G_pitch:                                                  *
 *           ~~~~~~~~                                                  *
 *---------------------------------------------------------------------*
 * Compute correlations <xn,y1> and <y1,y1> to use in gains quantizer. *
 * Also compute the gain of pitch. Result in Q14                       *
 *  if (gain < 0)  gain =0                                             *
 *  if (gain >1.2) gain =1.2                                           *
 *---------------------------------------------------------------------*/


int16_t WebRtcG729fix_G_pitch(/* (o) Q14: Gain of pitch lag saturated to 1.2*/
  int16_t xn[],       /* (i)     : Pitch target.                            */
  int16_t y1[],       /* (i)     : Filtered adaptive codebook.              */
  int16_t g_coeff[],  /* (i)     : Correlations need for gain quantization. */
  int16_t L_subfr     /* (i)     : Length of subframe.                      */
)
{
   int16_t i;
   int16_t xy, yy, exp_xy, exp_yy, gain;
   int32_t s, s1, L_temp;

   //int16_t scaled_y1[L_SUBFR];
   int16_t scaled_y1;

   s = 1; /* Avoid case of all zeros */
   for(i=0; i<L_subfr; i++)
   {
     /* divide "y1[]" by 4 to avoid overflow */
     //scaled_y1[i] = y1[i] >> 2;
     /* Compute scalar product <y1[],y1[]> */
     s += y1[i] * y1[i] << 1;
     if (s < 0)
       break;
   }

   if (i == L_subfr) {
     exp_yy = WebRtcSpl_NormW32(s);
     yy     = L_round( L_shl(s, exp_yy) );
   }
   else {
     //for(; i<L_subfr; i++)
       /* divide "y1[]" by 4 to avoid overflow */
       //scaled_y1[i] = y1[i] >> 2;

     s = 0;
     for(i=0; i<L_subfr; i++)
     {
       /* divide "y1[]" by 4 to avoid overflow */
       scaled_y1 = y1[i] >> 2;
       //s += scaled_y1[i] * scaled_y1[i];
       s += scaled_y1 * scaled_y1;
     }
     s <<= 1;
     s++; /* Avoid case of all zeros */

     exp_yy = WebRtcSpl_NormW32(s);
     yy     = L_round( L_shl(s, exp_yy) );
     exp_yy = exp_yy - 4;
   }

   /* Compute scalar product <xn[],y1[]> */
   s = 0;
   for(i=0; i<L_subfr; i++)
   {
     L_temp = xn[i] * y1[i];
     if (L_temp == 0x40000000)
       break;
     s1 = s;
     s = (L_temp << 1) + s1;

     if (((s1 ^ L_temp) > 0) && ((s ^ s1) < 0))
       break;
     //s = L_mac(s, xn[i], y1[i]);
   }

   if (i == L_subfr) {
     exp_xy = WebRtcSpl_NormW32(s);
     xy     = L_round( L_shl(s, exp_xy) );
   }
   else {
     s = 0;
     for(i=0; i<L_subfr; i++)
       //s += xn[i] * scaled_y1[i];
       s += xn[i] * (y1[i] >> 2);
     s <<= 1;
     exp_xy = WebRtcSpl_NormW32(s);
     xy     = L_round( L_shl(s, exp_xy) );
     exp_xy = exp_xy - 2;
   }

   g_coeff[0] = yy;
   g_coeff[1] = 15 - exp_yy;
   g_coeff[2] = xy;
   g_coeff[3] = 15 - exp_xy;

   /* If (xy <= 0) gain = 0 */


   if (xy <= 0)
   {
      g_coeff[3] = -15;   /* Force exp_xy to -15 = (15-30) */
      return( (int16_t) 0);
   }

   /* compute gain = xy/yy */

   xy >>= 1;             /* Be sure xy < yy */
   gain = div_s( xy, yy);

   i = exp_xy - exp_yy;
   gain = shr(gain, i);         /* saturation if > 1.99 in Q14 */

   /* if(gain >1.2) gain = 1.2  in Q14 */

   if (gain > 19661)
   {
     gain = 19661;
   }


   return(gain);
}


/*----------------------------------------------------------------------*
 *    Function Enc_lag3                                                 *
 *             ~~~~~~~~                                                 *
 *   Encoding of fractional pitch lag with 1/3 resolution.              *
 *----------------------------------------------------------------------*
 * The pitch range for the first subframe is divided as follows:        *
 *   19 1/3  to   84 2/3   resolution 1/3                               *
 *   85      to   143      resolution 1                                 *
 *                                                                      *
 * The period in the first subframe is encoded with 8 bits.             *
 * For the range with fractions:                                        *
 *   index = (T-19)*3 + frac - 1;   where T=[19..85] and frac=[-1,0,1]  *
 * and for the integer only range                                       *
 *   index = (T - 85) + 197;        where T=[86..143]                   *
 *----------------------------------------------------------------------*
 * For the second subframe a resolution of 1/3 is always used, and the  *
 * search range is relative to the lag in the first subframe.           *
 * If t0 is the lag in the first subframe then                          *
 *  t_min=t0-5   and  t_max=t0+4   and  the range is given by           *
 *       t_min - 2/3   to  t_max + 2/3                                  *
 *                                                                      *
 * The period in the 2nd subframe is encoded with 5 bits:               *
 *   index = (T-(t_min-1))*3 + frac - 1;    where T[t_min-1 .. t_max+1] *
 *----------------------------------------------------------------------*/


int16_t WebRtcG729fix_Enc_lag3(/* output: Return index of encoding */
  int16_t T0,         /* input : Pitch delay              */
  int16_t T0_frac,    /* input : Fractional pitch delay   */
  int16_t *T0_min,    /* in/out: Minimum search delay     */
  int16_t *T0_max,    /* in/out: Maximum search delay     */
  int16_t pit_min,    /* input : Minimum pitch delay      */
  int16_t pit_max,    /* input : Maximum pitch delay      */
  int16_t pit_flag    /* input : int for 1st subframe    */
)
{
  int16_t index, i;

  if (pit_flag == 0)   /* if 1st subframe */
  {
    /* encode pitch delay (with fraction) */

    if (T0 <= 85)
    {
      /* index = t0*3 - 58 + t0_frac   */
      i = WebRtcSpl_AddSatW16(WebRtcSpl_AddSatW16(T0, T0), T0);
      index = WebRtcSpl_AddSatW16(WebRtcSpl_SubSatW16(i, 58), T0_frac);
    }
    else {
      index = WebRtcSpl_AddSatW16(T0, 112);
    }

    /* find T0_min and T0_max for second subframe */

    *T0_min = WebRtcSpl_SubSatW16(T0, 5);
    if (*T0_min < pit_min)
    {
      *T0_min = pit_min;
    }

    *T0_max = WebRtcSpl_AddSatW16(*T0_min, 9);
    if (*T0_max > pit_max)
    {
      *T0_max = pit_max;
      *T0_min = WebRtcSpl_SubSatW16(*T0_max, 9);
    }
  }
  else      /* if second subframe */
  {

    /* i = t0 - t0_min;               */
    /* index = i*3 + 2 + t0_frac;     */
    i = WebRtcSpl_SubSatW16(T0, *T0_min);
    i = WebRtcSpl_AddSatW16(WebRtcSpl_AddSatW16(i, i), i);
    index = WebRtcSpl_AddSatW16(WebRtcSpl_AddSatW16(i, 2), T0_frac);
  }


  return index;
}
