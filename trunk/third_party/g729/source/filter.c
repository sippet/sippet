/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996
   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*-------------------------------------------------------------------*
 * Function  Convolve:                                               *
 *           ~~~~~~~~~                                               *
 *-------------------------------------------------------------------*
 * Perform the convolution between two vectors x[] and h[] and       *
 * write the result in the vector y[].                               *
 * All vectors are of length N.                                      *
 *-------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

void WebRtcG729fix_Convolve(
  int16_t x[],      /* (i)     : input vector                           */
  int16_t h[],      /* (i) Q12 : impulse response                       */
  int16_t y[],      /* (o)     : output vector                          */
  int16_t L         /* (i)     : vector size                            */
)
{
   int16_t i, n;
   int32_t s;

   for (n = 0; n < L; n++)
   {
     s = 0;
     for (i = 0; i <= n; i++)
       s = L_mac(s, x[i], h[n-i]);

     s    = L_shl(s, 3);                   /* h is in Q12 and saturation */
     y[n] = extract_h(s);
   }

   return;
}

/*-----------------------------------------------------*
 * procedure Syn_filt:                                 *
 *           ~~~~~~~~                                  *
 * Do the synthesis filtering 1/A(z).                  *
 *-----------------------------------------------------*/


int WebRtcG729fix_Syn_filt(
  int16_t a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  int16_t x[],     /* (i)     : input signal                             */
  int16_t y[],     /* (o)     : output signal                            */
  int16_t lg,      /* (i)     : size of filtering                        */
  int16_t mem[],   /* (i/o)   : memory associated with this filtering.   */
  int16_t update   /* (i)     : 0=no update, 1=update of memory.         */
)
{
  int16_t i, j;
  int32_t s, L_tmp, L_tmp1;
  int16_t tmp[100];     /* This is usually done by memory allocation (lg+M) */
  int16_t *yy;
  int Overflow = 0;

  /* Copy mem[] to yy[] */

  yy = tmp;

  for(i=0; i<M; i++)
  {
    *yy++ = mem[i];
  }

  /* Do the filtering. */

  for (i = 0; i < lg; i++)
  {
    /* L_mult inline to catch overflow */
    s = x[i] * a[0];
    if (s != (int32_t)0x40000000L) {
      s <<= 1;
    } else {
      Overflow = 1;
      s = WEBRTC_SPL_WORD32_MAX;
    }
    for (j = 1; j <= M; j++) {
      /* s = L_msu(s, a[j], yy[-j]); */
      L_tmp = a[j] * yy[-j];
      if (L_tmp != (int32_t)0x40000000L) {
        L_tmp <<= 1;
      } else {
        Overflow = 1;
        L_tmp = WEBRTC_SPL_WORD32_MAX;
      }
      L_tmp1 = s - L_tmp;
      if ((s ^ L_tmp) < 0) {
        if ((L_tmp1 ^ s) & WEBRTC_SPL_WORD32_MIN) {
          L_tmp1 = (s < 0L) ? WEBRTC_SPL_WORD32_MIN : WEBRTC_SPL_WORD32_MAX;
          Overflow = 1;
        }
      }
      s = L_tmp1;
    }

    /* s = L_shl(s, 3); */
    L_tmp = s << 3;
    if (L_tmp >> 3 != s)
      L_tmp = (s & WEBRTC_SPL_WORD32_MIN ? WEBRTC_SPL_WORD32_MIN : WEBRTC_SPL_WORD32_MAX);
    s = L_tmp;

    *yy++ = L_round(s);
  }

  for(i=0; i<lg; i++)
  {
    y[i] = tmp[i+M];
  }

  /* Update of memory if update==1 */

  if(update != 0)
     for (i = 0; i < M; i++)
     {
       mem[i] = y[lg-M+i];
     }

 return Overflow;
}

/*-----------------------------------------------------------------------*
 * procedure Residu:                                                     *
 *           ~~~~~~                                                      *
 * Compute the LPC residual  by filtering the input speech through A(z)  *
 *-----------------------------------------------------------------------*/

void WebRtcG729fix_Residu(
  int16_t a[],    /* (i) Q12 : prediction coefficients                     */
  int16_t x[],    /* (i)     : speech (values x[-m..-1] are needed         */
  int16_t y[],    /* (o)     : residual signal                             */
  int16_t lg      /* (i)     : size of filtering                           */
)
{
  int16_t i, j;
  int32_t s;

  for (i = 0; i < lg; i++)
  {
    s = L_mult(x[i], a[0]);
    for (j = 1; j <= M; j++)
      s = L_mac(s, a[j], x[i-j]);

    s = L_shl(s, 3);
    y[i] = L_round(s);
  }
  return;
}
