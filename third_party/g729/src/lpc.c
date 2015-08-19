/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------*
 * Function Autocorr()                                 *
 *                                                     *
 *   Compute autocorrelations of signal with windowing *
 *                                                     *
 *-----------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

void WebRtcG729fix_Autocorr(
  int16_t x[],      /* (i)    : Input signal                      */
  int16_t m,        /* (i)    : LPC order                         */
  int16_t r_h[],    /* (o)    : Autocorrelations  (msb)           */
  int16_t r_l[],    /* (o)    : Autocorrelations  (lsb)           */
  int16_t *exp_R0
)
{
  int16_t i, j, norm;
  int16_t y[L_WINDOW];
  int32_t sum;

  int Overflow;

  /* Windowing of signal */

  for(i=0; i<L_WINDOW; i++)
  {
    y[i] = mult_r(x[i], WebRtcG729fix_hamwindow[i]);
  }

  /* Compute r[0] and test for overflow */

  *exp_R0 = 1;

  do {
    Overflow = 0;
    sum = 1;                   /* Avoid case of all zeros */
    for(i=0; i<L_WINDOW; i++) {
      /* sum = L_mac(sum, y[i], y[i]); */
      sum += y[i] * y[i] << 1;
      if (sum < 0) {
        sum = WEBRTC_SPL_WORD32_MAX;
        Overflow = 1;
        break;
      }
    }

    /* If overflow divide y[] by 4 */

    if (Overflow != 0)
    {
      for(i=0; i<L_WINDOW; i++)
      {
        y[i] = shr(y[i], 2);
      }
      *exp_R0 = WebRtcSpl_AddSatW16((*exp_R0), 4);
    }
  } while (Overflow != 0);

  /* Normalization of r[0] */

  norm = WebRtcSpl_NormW32(sum);
  sum  = L_shl(sum, norm);
  WebRtcG729fix_L_Extract(sum, &r_h[0], &r_l[0]);     /* Put in DPF format (see oper_32b) */
  *exp_R0 = WebRtcSpl_SubSatW16(*exp_R0, norm);

  /* r[1] to r[m] */

  for (i = 1; i <= m; i++)
  {
    sum = 0;
    for(j=0; j<L_WINDOW-i; j++)
      sum = L_mac(sum, y[j], y[j+i]);

    sum = L_shl(sum, norm);
    WebRtcG729fix_L_Extract(sum, &r_h[i], &r_l[i]);
  }
}


/*-------------------------------------------------------*
 * Function Lag_window()                                 *
 *                                                       *
 * Lag_window on autocorrelations.                       *
 *                                                       *
 * r[i] *= lag_wind[i]                                   *
 *                                                       *
 *  r[i] and lag_wind[i] are in special double precision.*
 *  See "oper_32b.c" for the format                      *
 *                                                       *
 *-------------------------------------------------------*/

void WebRtcG729fix_Lag_window(
  int16_t m,         /* (i)     : LPC order                        */
  int16_t r_h[],     /* (i/o)   : Autocorrelations  (msb)          */
  int16_t r_l[]      /* (i/o)   : Autocorrelations  (lsb)          */
)
{
  int16_t i;
  int32_t x;

  for(i=1; i<=m; i++)
  {
     x  = WebRtcG729fix_Mpy_32(r_h[i], r_l[i], WebRtcG729fix_lag_h[i-1], WebRtcG729fix_lag_l[i-1]);
     WebRtcG729fix_L_Extract(x, &r_h[i], &r_l[i]);
  }
  return;
}


/*___________________________________________________________________________
 |                                                                           |
 |      LEVINSON-DURBIN algorithm in double precision                        |
 |      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                        |
 |---------------------------------------------------------------------------|
 |                                                                           |
 | Algorithm                                                                 |
 |                                                                           |
 |       R[i]    autocorrelations.                                           |
 |       A[i]    filter coefficients.                                        |
 |       K       reflection coefficients.                                    |
 |       Alpha   prediction gain.                                            |
 |                                                                           |
 |       Initialization:                                                     |
 |               A[0] = 1                                                    |
 |               K    = -R[1]/R[0]                                           |
 |               A[1] = K                                                    |
 |               Alpha = R[0] * (1-K**2]                                     |
 |                                                                           |
 |       Do for  i = 2 to M                                                  |
 |                                                                           |
 |            S =  SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i]                      |
 |                                                                           |
 |            K = -S / Alpha                                                 |
 |                                                                           |
 |            An[j] = A[j] + K*A[i-j]   for j=1 to i-1                       |
 |                                      where   An[i] = new A[i]             |
 |            An[i]=K                                                        |
 |                                                                           |
 |            Alpha=Alpha * (1-K**2)                                         |
 |                                                                           |
 |       END                                                                 |
 |                                                                           |
 | Remarks on the dynamics of the calculations.                              |
 |                                                                           |
 |       The numbers used are in double precision in the following format :  |
 |       A = AH <<16 + AL<<1.  AH and AL are 16 bit signed integers.         |
 |       Since the LSB's also contain a sign bit, this format does not       |
 |       correspond to standard 32 bit integers.  We use this format since   |
 |       it allows fast execution of multiplications and divisions.          |
 |                                                                           |
 |       "DPF" will refer to this special format in the following text.      |
 |       See oper_32b.c                                                      |
 |                                                                           |
 |       The R[i] were normalized in routine AUTO (hence, R[i] < 1.0).       |
 |       The K[i] and Alpha are theoretically < 1.0.                         |
 |       The A[i], for a sampling frequency of 8 kHz, are in practice        |
 |       always inferior to 16.0.                                            |
 |                                                                           |
 |       These characteristics allow straigthforward fixed-point             |
 |       implementation.  We choose to represent the parameters as           |
 |       follows :                                                           |
 |                                                                           |
 |               R[i]    Q31   +- .99..                                      |
 |               K[i]    Q31   +- .99..                                      |
 |               Alpha   Normalized -> mantissa in Q31 plus exponent         |
 |               A[i]    Q27   +- 15.999..                                   |
 |                                                                           |
 |       The additions are performed in 32 bit.  For the summation used      |
 |       to calculate the K[i], we multiply numbers in Q31 by numbers        |
 |       in Q27, with the result of the multiplications in Q27,              |
 |       resulting in a dynamic of +- 16.  This is sufficient to avoid       |
 |       overflow, since the final result of the summation is                |
 |       necessarily < 1.0 as both the K[i] and Alpha are                    |
 |       theoretically < 1.0.                                                |
 |___________________________________________________________________________|
*/


void WebRtcG729fix_Levinson(
  Coder_ld8a_state *st,
  int16_t Rh[],      /* (i)     : Rh[M+1] Vector of autocorrelations (msb) */
  int16_t Rl[],      /* (i)     : Rl[M+1] Vector of autocorrelations (lsb) */
  int16_t A[],       /* (o) Q12 : A[M]    LPC coefficients  (m = 10)       */
  int16_t rc[],      /* (o) Q15 : rc[M]   Reflection coefficients.         */
  int16_t *Err       /* (o)     : Residual energy                          */
)
{
 int16_t i, j;
 int16_t hi, lo;
 int16_t Kh, Kl;                /* reflection coefficient; hi and lo           */
 int16_t alp_h, alp_l, alp_exp; /* Prediction gain; hi lo and exponent         */
 int16_t Ah[M+1], Al[M+1];      /* LPC coef. in double prec.                   */
 int16_t Anh[M+1], Anl[M+1];    /* LPC coef.for next iteration in double prec. */
 int32_t t0, t1, t2;            /* temporary variable                          */


/* K = A[1] = -R[1] / R[0] */

  t1  = WebRtcG729fix_L_Comp(Rh[1], Rl[1]);           /* R[1] in Q31      */
  t2  = L_abs(t1);                                    /* abs R[1]         */
  t0  = WebRtcG729fix_Div_32(t2, Rh[0], Rl[0]);       /* R[1]/R[0] in Q31 */
  if(t1 > 0) t0= L_negate(t0);                        /* -R[1]/R[0]       */
  WebRtcG729fix_L_Extract(t0, &Kh, &Kl);              /* K in DPF         */
  rc[0] = Kh;
  t0 = L_shr(t0,4);                                   /* A[1] in Q27      */
  WebRtcG729fix_L_Extract(t0, &Ah[1], &Al[1]);        /* A[1] in DPF      */

/*  Alpha = R[0] * (1-K**2) */

  t0 = WebRtcG729fix_Mpy_32(Kh ,Kl, Kh, Kl);          /* K*K      in Q31 */
  t0 = L_abs(t0);                                     /* Some case <0 !! */
  t0 = WebRtcSpl_SubSatW32( (int32_t)0x7fffffffL, t0 ); /* 1 - K*K  in Q31 */
  WebRtcG729fix_L_Extract(t0, &hi, &lo);              /* DPF format      */
  t0 = WebRtcG729fix_Mpy_32(Rh[0] ,Rl[0], hi, lo);    /* Alpha in Q31    */

/* Normalize Alpha */

  alp_exp = WebRtcSpl_NormW32(t0);
  t0 = L_shl(t0, alp_exp);
  WebRtcG729fix_L_Extract(t0, &alp_h, &alp_l);         /* DPF format    */

/*--------------------------------------*
 * ITERATIONS  I=2 to M                 *
 *--------------------------------------*/

  for(i= 2; i<=M; i++)
  {

    /* t0 = SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i] */

    t0 = 0;
    for(j=1; j<i; j++)
      t0 = WebRtcSpl_AddSatW32(t0, WebRtcG729fix_Mpy_32(Rh[j], Rl[j], Ah[i-j], Al[i-j]));

    t0 = L_shl(t0,4);                  /* result in Q27 -> convert to Q31 */
                                       /* No overflow possible            */
    t1 = WebRtcG729fix_L_Comp(Rh[i],Rl[i]);
    t0 = WebRtcSpl_AddSatW32(t0, t1);                /* add R[i] in Q31                 */

    /* K = -t0 / Alpha */

    t1 = L_abs(t0);
    t2 = WebRtcG729fix_Div_32(t1, alp_h, alp_l);     /* abs(t0)/Alpha                   */
    if(t0 > 0) t2= L_negate(t2);       /* K =-t0/Alpha                    */
    t2 = L_shl(t2, alp_exp);           /* denormalize; compare to Alpha   */
    WebRtcG729fix_L_Extract(t2, &Kh, &Kl);           /* K in DPF                        */
    rc[i-1] = Kh;

    /* Test for unstable filter. If unstable keep old A(z) */

    if (abs_s(Kh) > 32750)
    {
      for(j=0; j<=M; j++)
      {
        A[j] = st->old_A[j];
      }
      rc[0] = st->old_rc[0];        /* only two rc coefficients are needed */
      rc[1] = st->old_rc[1];
      return;
    }

    /*------------------------------------------*
     *  Compute new LPC coeff. -> An[i]         *
     *  An[j]= A[j] + K*A[i-j]     , j=1 to i-1 *
     *  An[i]= K                                *
     *------------------------------------------*/


    for(j=1; j<i; j++)
    {
      t0 = WebRtcG729fix_Mpy_32(Kh, Kl, Ah[i-j], Al[i-j]);
      t0 = WebRtcSpl_AddSatW32(t0, WebRtcG729fix_L_Comp(Ah[j], Al[j]));
      WebRtcG729fix_L_Extract(t0, &Anh[j], &Anl[j]);
    }
    t2 = L_shr(t2, 4);                  /* t2 = K in Q31 ->convert to Q27  */
    WebRtcG729fix_L_Extract(t2, &Anh[i], &Anl[i]);    /* An[i] in Q27                    */

    /*  Alpha = Alpha * (1-K**2) */

    t0 = WebRtcG729fix_Mpy_32(Kh ,Kl, Kh, Kl);          /* K*K      in Q31 */
    t0 = L_abs(t0);                                     /* Some case <0 !! */
    t0 = WebRtcSpl_SubSatW32( (int32_t)0x7fffffffL, t0 ); /* 1 - K*K  in Q31 */
    WebRtcG729fix_L_Extract(t0, &hi, &lo);              /* DPF format      */
    t0 = WebRtcG729fix_Mpy_32(alp_h , alp_l, hi, lo);   /* Alpha in Q31    */

    /* Normalize Alpha */

    j = WebRtcSpl_NormW32(t0);
    t0 = L_shl(t0, j);
    WebRtcG729fix_L_Extract(t0, &alp_h, &alp_l);         /* DPF format    */
    alp_exp = WebRtcSpl_AddSatW16(alp_exp, j);             /* Add normalization to alp_exp */

    /* A[j] = An[j] */

    WEBRTC_SPL_MEMCPY_W16(&Ah[1], &Anh[1], i);
    WEBRTC_SPL_MEMCPY_W16(&Al[1], &Anl[1], i);
  }

  *Err = shr(alp_h, alp_exp);

  /* Truncate A[i] in Q27 to Q12 with rounding */

  A[0] = 4096;
  for(i=1; i<=M; i++)
  {
    t0   = WebRtcG729fix_L_Comp(Ah[i], Al[i]);
    st->old_A[i] = A[i] = L_round(L_shl(t0, 1));
  }
  st->old_rc[0] = rc[0];
  st->old_rc[1] = rc[1];

  return;
}




/*-------------------------------------------------------------*
 *  procedure Az_lsp:                                          *
 *            ~~~~~~                                           *
 *   Compute the LSPs from  the LPC coefficients  (order=10)   *
 *-------------------------------------------------------------*/

/* local function */

static int16_t Chebps_11(int16_t x, int16_t f[], int16_t n);
static int16_t Chebps_10(int16_t x, int16_t f[], int16_t n);

void WebRtcG729fix_Az_lsp(
  int16_t a[],        /* (i) Q12 : predictor coefficients              */
  int16_t lsp[],      /* (o) Q15 : line spectral pairs                 */
  int16_t old_lsp[]   /* (i)     : old lsp[] (in case not found 10 roots) */
)
{
 int16_t i, j, nf, ip;
 int16_t xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
 int16_t x, y, sign, exp;
 int16_t *coef;
 int16_t f1[M/2+1], f2[M/2+1];
 int32_t t0, L_temp1, L_temp2;
 int16_t (*pChebps)(int16_t x, int16_t f[], int16_t n);

/*-------------------------------------------------------------*
 *  find the sum and diff. pol. F1(z) and F2(z)                *
 *    F1(z) <--- F1(z)/(1+z**-1) & F2(z) <--- F2(z)/(1-z**-1)  *
 *                                                             *
 * f1[0] = 1.0;                                                *
 * f2[0] = 1.0;                                                *
 *                                                             *
 * for (i = 0; i< NC; i++)                                     *
 * {                                                           *
 *   f1[i+1] = a[i+1] + a[M-i] - f1[i] ;                       *
 *   f2[i+1] = a[i+1] - a[M-i] + f2[i] ;                       *
 * }                                                           *
 *-------------------------------------------------------------*/

 pChebps = Chebps_11;

 f1[0] = 2048;          /* f1[0] = 1.0 is in Q11 */
 f2[0] = 2048;          /* f2[0] = 1.0 is in Q11 */

 for (i = 0; i< NC; i++)
 {
   L_temp1 = (int32_t)a[i+1];
   L_temp2 = (int32_t)a[M-i];

   /* x = (a[i+1] + a[M-i]) >> 1        */
   x = ((L_temp1 + L_temp2) >> 1);
   /* x = (a[i+1] - a[M-i]) >> 1        */
   y = ((L_temp1 - L_temp2) >> 1);

   /* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
   L_temp1 = (int32_t)x - (int32_t)f1[i];
   if (L_temp1 > 0x00007fffL || L_temp1 < (int32_t)0xffff8000L)
     break;
   f1[i+1] = (int16_t)L_temp1;

   /* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
   L_temp2 = (int32_t)y + (int32_t)f2[i];
   if (L_temp2 > 0x00007fffL || (L_temp2 < (int32_t)0xffff8000L))
     break;
   f2[i+1] = (int16_t)L_temp2;
 }

 if (i != NC) {
   //printf("===== OVF ovf_coef =====\n");

   pChebps = Chebps_10;

   f1[0] = 1024;          /* f1[0] = 1.0 is in Q10 */
   f2[0] = 1024;          /* f2[0] = 1.0 is in Q10 */

   for (i = 0; i< NC; i++)
   {
     L_temp1 = (int32_t)a[i+1];
     L_temp2 = (int32_t)a[M-i];
     /* x = (a[i+1] + a[M-i]) >> 2  */
     x = (int16_t)((L_temp1 + L_temp2) >> 2);
     /* y = (a[i+1] - a[M-i]) >> 2 */
     y = (int16_t)((L_temp1 - L_temp2) >> 2);

     f1[i+1] = x - f1[i];            /* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
     f2[i+1] = y + f2[i];            /* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
   }
 }

/*-------------------------------------------------------------*
 * find the LSPs using the Chebichev pol. evaluation           *
 *-------------------------------------------------------------*/

 nf=0;          /* number of found frequencies */
 ip=0;          /* indicator for f1 or f2      */

 coef = f1;

 xlow = WebRtcG729fix_grid[0];
 ylow = (*pChebps)(xlow, coef, NC);

 j = 0;
 while ( (nf < M) && (j < GRID_POINTS) )
 {
   j++;
   xhigh = xlow;
   yhigh = ylow;
   xlow  = WebRtcG729fix_grid[j];
   ylow  = (*pChebps)(xlow,coef,NC);

   if (L_mult(ylow ,yhigh) <= 0)
   {
     /* divide 2 times the interval */
     for (i = 0; i < 2; i++)
     {
       xmid = WebRtcSpl_AddSatW16( shr(xlow, 1) , shr(xhigh, 1)); /* xmid = (xlow + xhigh)/2 */

       ymid = (*pChebps)(xmid,coef,NC);

       if (L_mult(ylow,ymid) <= (int32_t)0)
       {
         yhigh = ymid;
         xhigh = xmid;
       }
       else
       {
         ylow = ymid;
         xlow = xmid;
       }
     }

    /*-------------------------------------------------------------*
     * Linear interpolation                                        *
     *    xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);            *
     *-------------------------------------------------------------*/

     x   = WebRtcSpl_SubSatW16(xhigh, xlow);
     y   = WebRtcSpl_SubSatW16(yhigh, ylow);

     if(y == 0)
     {
       xint = xlow;
     }
     else
     {
       sign= y;
       y   = abs_s(y);
       exp = WebRtcSpl_NormW16(y);
       y   = shl(y, exp);
       y   = div_s( (int16_t)16383, y);
       t0  = L_mult(x, y);
       t0  = L_shr(t0, WebRtcSpl_SubSatW16(20, exp) );
       y   = extract_l(t0);            /* y= (xhigh-xlow)/(yhigh-ylow) in Q11 */

       if(sign < 0) y = negate(y);

       t0   = L_mult(ylow, y);                  /* result in Q26 */
       t0   = L_shr(t0, 11);                    /* result in Q15 */
       xint = WebRtcSpl_SubSatW16(xlow, extract_l(t0));         /* xint = xlow - ylow*y */
     }

     lsp[nf] = xint;
     xlow    = xint;
     nf++;

     if(ip == 0)
     {
       ip = 1;
       coef = f2;
     }
     else
     {
       ip = 0;
       coef = f1;
     }
     ylow = (*pChebps)(xlow,coef,NC);

   }
 }

 /* Check if M roots found */

 if (nf < M)
 {
   WEBRTC_SPL_MEMCPY_W16(lsp, old_lsp, M);
 /* printf("\n !!Not 10 roots found in Az_lsp()!!!\n"); */
 }
}

/*--------------------------------------------------------------*
 * function  Chebps_11, Chebps_10:                              *
 *           ~~~~~~~~~~~~~~~~~~~~                               *
 *    Evaluates the Chebichev polynomial series                 *
 *--------------------------------------------------------------*
 *                                                              *
 *  The polynomial order is                                     *
 *     n = M/2   (M is the prediction order)                    *
 *  The polynomial is given by                                  *
 *    C(x) = T_n(x) + f(1)T_n-1(x) + ... +f(n-1)T_1(x) + f(n)/2 *
 * Arguments:                                                   *
 *  x:     input value of evaluation; x = cos(frequency) in Q15 *
 *  f[]:   coefficients of the pol.                             *
 *                         in Q11(Chebps_11), in Q10(Chebps_10) *
 *  n:     order of the pol.                                    *
 *                                                              *
 * The value of C(x) is returned. (Saturated to +-1.99 in Q14)  *
 *                                                              *
 *--------------------------------------------------------------*/
static int16_t Chebps_11(int16_t x, int16_t f[], int16_t n)
{
  int16_t i, cheb;
  int16_t b0_h, b0_l, b1_h, b1_l, b2_h, b2_l;
  int32_t t0;

 /* Note: All computation are done in Q24. */

  b2_h = 256;                           /* b2 = 1.0 in Q24 DPF */
  b2_l = 0;

  t0 = L_mult(x, 512);                  /* 2*x in Q24          */
  t0 = L_mac(t0, f[1], 4096);           /* + f[1] in Q24       */
  WebRtcG729fix_L_Extract(t0, &b1_h, &b1_l);          /* b1 = 2*x + f[1]     */

  for (i = 2; i<n; i++)
  {
    t0 = WebRtcG729fix_Mpy_32_16(b1_h, b1_l, x);      /* t0 = 2.0*x*b1              */
    t0 = L_shl(t0, 1);
    t0 = L_mac(t0,b2_h,(int16_t)-32768L);/* t0 = 2.0*x*b1 - b2         */
    t0 = L_msu(t0, b2_l, 1);
    t0 = L_mac(t0, f[i], 4096);         /* t0 = 2.0*x*b1 - b2 + f[i]; */

    WebRtcG729fix_L_Extract(t0, &b0_h, &b0_l);        /* b0 = 2.0*x*b1 - b2 + f[i]; */

    b2_l = b1_l;                        /* b2 = b1; */
    b2_h = b1_h;
    b1_l = b0_l;                        /* b1 = b0; */
    b1_h = b0_h;
  }

  t0 = WebRtcG729fix_Mpy_32_16(b1_h, b1_l, x);        /* t0 = x*b1;              */
  t0 = L_mac(t0, b2_h,(int16_t)-32768L); /* t0 = x*b1 - b2          */
  t0 = L_msu(t0, b2_l, 1);
  t0 = L_mac(t0, f[i], 2048);           /* t0 = x*b1 - b2 + f[i]/2 */

  t0 = L_shl(t0, 6);                    /* Q24 to Q30 with saturation */
  cheb = extract_h(t0);                 /* Result in Q14              */


  return(cheb);
}


static int16_t Chebps_10(int16_t x, int16_t f[], int16_t n)
{
  int16_t i, cheb;
  int16_t b0_h, b0_l, b1_h, b1_l, b2_h, b2_l;
  int32_t t0;

 /* Note: All computation are done in Q23. */

  b2_h = 128;                           /* b2 = 1.0 in Q23 DPF */
  b2_l = 0;

  t0 = L_mult(x, 256);                  /* 2*x in Q23          */
  t0 = L_mac(t0, f[1], 4096);           /* + f[1] in Q23       */
  WebRtcG729fix_L_Extract(t0, &b1_h, &b1_l);          /* b1 = 2*x + f[1]     */

  for (i = 2; i<n; i++)
  {
    t0 = WebRtcG729fix_Mpy_32_16(b1_h, b1_l, x);      /* t0 = 2.0*x*b1              */
    t0 = L_shl(t0, 1);
    t0 = L_mac(t0,b2_h,(int16_t)-32768L);/* t0 = 2.0*x*b1 - b2         */
    t0 = L_msu(t0, b2_l, 1);
    t0 = L_mac(t0, f[i], 4096);         /* t0 = 2.0*x*b1 - b2 + f[i]; */

    WebRtcG729fix_L_Extract(t0, &b0_h, &b0_l);        /* b0 = 2.0*x*b1 - b2 + f[i]; */

    b2_l = b1_l;                        /* b2 = b1; */
    b2_h = b1_h;
    b1_l = b0_l;                        /* b1 = b0; */
    b1_h = b0_h;
  }

  t0 = WebRtcG729fix_Mpy_32_16(b1_h, b1_l, x);        /* t0 = x*b1;              */
  t0 = L_mac(t0, b2_h,(int16_t)-32768L); /* t0 = x*b1 - b2          */
  t0 = L_msu(t0, b2_l, 1);
  t0 = L_mac(t0, f[i], 2048);           /* t0 = x*b1 - b2 + f[i]/2 */

  t0 = L_shl(t0, 7);                    /* Q23 to Q30 with saturation */
  cheb = extract_h(t0);                 /* Result in Q14              */


  return(cheb);
}
