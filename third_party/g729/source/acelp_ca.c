/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* G.729A  Version 1.1    Last modified: September 1996 */

/*
   ITU-T G.729A Speech Coder     ANSI-C Source Code
   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*---------------------------------------------------------------------------*
 *  Function  ACELP_Code_A()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
 *   Find Algebraic codebook for G.729A                                      *
 *---------------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"

/* Constants defined in ld8a.h */
/*  L_SUBFR   -> Lenght of subframe.                                        */
/*  NB_POS    -> Number of positions for each pulse.                        */
/*  STEP      -> Step betweem position of the same pulse.                   */
/*  MSIZE     -> Size of vectors for cross-correlation between two pulses.  */


/* local routines definition */

static void Cor_h(
     int16_t *H,         /* (i) Q12 :Impulse response of filters */
     int16_t *rr         /* (o)     :Correlations of H[]         */
);
static int16_t D4i40_17_fast(/*(o) : Index of pulses positions.             */
  int16_t dn[],          /* (i)    : Correlations between h[] and Xn[].     */
  int16_t *rr,           /* (i)    : Correlations of impulse response h[].  */
  int16_t h[],           /* (i) Q12: Impulse response of filters.           */
  int16_t cod[],         /* (o) Q13: Selected algebraic codeword.           */
  int16_t y[],           /* (o) Q12: Filtered algebraic codeword.           */
  int16_t *sign          /* (o)    : Signs of 4 pulses.                     */
);

 /*-----------------------------------------------------------------*
  * Main ACELP function.                                            *
  *-----------------------------------------------------------------*/

int16_t  WebRtcG729fix_ACELP_Code_A(/* (o):index of pulses positions        */
  int16_t x[],           /* (i)     :Target vector                          */
  int16_t h[],           /* (i) Q12 :Inpulse response of filters            */
  int16_t T0,            /* (i)     :Pitch lag                              */
  int16_t pitch_sharp,   /* (i) Q14 :Last quantized pitch gain              */
  int16_t code[],        /* (o) Q13 :Innovative codebook                    */
  int16_t y[],           /* (o) Q12 :Filtered innovative codebook           */
  int16_t *sign          /* (o)     :Signs of 4 pulses                      */
)
{
  int16_t i, index, sharp;
  int16_t Dn[L_SUBFR];
  int16_t rr[DIM_RR];

 /*-----------------------------------------------------------------*
  * Include fixed-gain pitch contribution into impulse resp. h[]    *
  * Find correlations of h[] needed for the codebook search.        *
  *-----------------------------------------------------------------*/

  sharp = shl(pitch_sharp, 1);          /* From Q14 to Q15 */
  if (T0 < L_SUBFR)
     for (i = T0; i < L_SUBFR; i++)     /* h[i] += pitch_sharp*h[i-T0] */
       h[i] = WebRtcSpl_AddSatW16(h[i], mult(h[i-T0], sharp));

  Cor_h(h, rr);

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  WebRtcG729fix_Cor_h_X(h, x, Dn);

 /*-----------------------------------------------------------------*
  * Find innovative codebook.                                       *
  *-----------------------------------------------------------------*/

  index = D4i40_17_fast(Dn, rr, h, code, y, sign);

 /*-----------------------------------------------------------------*
  * Compute innovation vector gain.                                 *
  * Include fixed-gain pitch contribution into code[].              *
  *-----------------------------------------------------------------*/

  if(T0 < L_SUBFR)
     for (i = T0; i < L_SUBFR; i++)    /* code[i] += pitch_sharp*code[i-T0] */
       code[i] = WebRtcSpl_AddSatW16(code[i], mult(code[i-T0], sharp));

  return index;
}


/*--------------------------------------------------------------------------*
 *  Function  Cor_h()                                                       *
 *  ~~~~~~~~~~~~~~~~~                                                       *
 * Compute  correlations of h[]  needed for the codebook search.            *
 *--------------------------------------------------------------------------*/

static void Cor_h(
  int16_t *H,     /* (i) Q12 :Impulse response of filters */
  int16_t *rr     /* (o)     :Correlations of H[]         */
)
{
  int16_t *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  int16_t *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  int16_t *rri1i2, *rri1i3, *rri1i4;
  int16_t *rri2i3, *rri2i4;

  int16_t *p0, *p1, *p2, *p3, *p4;

  int16_t *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
  int32_t cor;
  int16_t i, k, ldec, l_fin_sup, l_fin_inf;
  int16_t h[L_SUBFR];

 /* Scaling h[] for maximum precision */

  cor = 0;
  for(i=0; i<L_SUBFR; i++)
    cor = L_mac(cor, H[i], H[i]);

  if(extract_h(cor) > 32000)
  {
    for(i=0; i<L_SUBFR; i++) {
      h[i] = shr(H[i], 1);
    }
  }
  else
  {
    k = WebRtcSpl_NormW32(cor);
    k = shr(k, 1);

    for(i=0; i<L_SUBFR; i++) {
      h[i] = shl(H[i], k);
    }
  }


 /*------------------------------------------------------------*
  * Compute rri0i0[], rri1i1[], rri2i2[], rri3i3 and rri4i4[]  *
  *------------------------------------------------------------*/
  /* Init pointers */
  rri0i0 = rr;
  rri1i1 = rri0i0 + NB_POS;
  rri2i2 = rri1i1 + NB_POS;
  rri3i3 = rri2i2 + NB_POS;
  rri4i4 = rri3i3 + NB_POS;
  rri0i1 = rri4i4 + NB_POS;
  rri0i2 = rri0i1 + MSIZE;
  rri0i3 = rri0i2 + MSIZE;
  rri0i4 = rri0i3 + MSIZE;
  rri1i2 = rri0i4 + MSIZE;
  rri1i3 = rri1i2 + MSIZE;
  rri1i4 = rri1i3 + MSIZE;
  rri2i3 = rri1i4 + MSIZE;
  rri2i4 = rri2i3 + MSIZE;

  p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
  p1 = rri1i1 + NB_POS-1;
  p2 = rri2i2 + NB_POS-1;
  p3 = rri3i3 + NB_POS-1;
  p4 = rri4i4 + NB_POS-1;

  ptr_h1 = h;
  cor    = 0;
  for(i=0;  i<NB_POS; i++)
  {
    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p4-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p3-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p2-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p1-- = extract_h(cor);

    cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
    *p0-- = extract_h(cor);
  }

 /*-----------------------------------------------------------------*
  * Compute elements of: rri2i3[], rri1i2[], rri0i1[] and rri0i4[]  *
  *-----------------------------------------------------------------*/

  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-(int16_t)1;
  ldec = NB_POS+1;

  ptr_hd = h;
  ptr_hf = ptr_hd + 1;

  for(k=0; k<NB_POS; k++) {

          p3 = rri2i3 + l_fin_sup;
          p2 = rri1i2 + l_fin_sup;
          p1 = rri0i1 + l_fin_sup;
          p0 = rri0i4 + l_fin_inf;

          cor = 0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;

          for(i=k+(int16_t)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p2 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p1 = extract_h(cor);

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*---------------------------------------------------------------------*
  * Compute elements of: rri2i4[], rri1i3[], rri0i2[], rri1i4[], rri0i3 *
  *---------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 2;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-(int16_t)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri2i4 + l_fin_sup;
          p3 = rri1i3 + l_fin_sup;
          p2 = rri0i2 + l_fin_sup;
          p1 = rri1i4 + l_fin_inf;
          p0 = rri0i3 + l_fin_inf;

          cor = 0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          for(i=k+(int16_t)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p4 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p4 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p2 = extract_h(cor);


          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*----------------------------------------------------------------------*
  * Compute elements of: rri1i4[], rri0i3[], rri2i4[], rri1i3[], rri0i2  *
  *----------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 3;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-(int16_t)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri1i4 + l_fin_sup;
          p3 = rri0i3 + l_fin_sup;
          p2 = rri2i4 + l_fin_inf;
          p1 = rri1i3 + l_fin_inf;
          p0 = rri0i2 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(int16_t)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p4 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p4 -= ldec;
                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p4 = extract_h(cor);

          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }

 /*----------------------------------------------------------------------*
  * Compute elements of: rri0i4[], rri2i3[], rri1i2[], rri0i1[]          *
  *----------------------------------------------------------------------*/

  ptr_hd = h;
  ptr_hf = ptr_hd + 4;
  l_fin_sup = MSIZE-1;
  l_fin_inf = l_fin_sup-(int16_t)1;
  for(k=0; k<NB_POS; k++) {

          p3 = rri0i4 + l_fin_sup;
          p2 = rri2i3 + l_fin_inf;
          p1 = rri1i2 + l_fin_inf;
          p0 = rri0i1 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(int16_t)1; i<NB_POS; i++ ) {

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p3 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p2 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p1 = extract_h(cor);

                  cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
                  *p0 = extract_h(cor);

                  p3 -= ldec;
                  p2 -= ldec;
                  p1 -= ldec;
                  p0 -= ldec;
          }
          cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
          *p3 = extract_h(cor);

          l_fin_sup -= NB_POS;
          l_fin_inf--;
          ptr_hf += STEP;
  }
  return;
}


/*------------------------------------------------------------------------*
 * Function  D4i40_17_fast()                                              *
 *           ~~~~~~~~~                                                    *
 * Algebraic codebook for ITU 8kb/s.                                      *
 *  -> 17 bits; 4 pulses in a frame of 40 samples                         *
 *                                                                        *
 *------------------------------------------------------------------------*
 * The code length is 40, containing 4 nonzero pulses i0, i1, i2, i3.     *
 * Each pulses can have 8 possible positions (positive or negative)       *
 * except i3 that have 16 possible positions.                             *
 *                                                                        *
 * i0 (+-1) : 0, 5, 10, 15, 20, 25, 30, 35                                *
 * i1 (+-1) : 1, 6, 11, 16, 21, 26, 31, 36                                *
 * i2 (+-1) : 2, 7, 12, 17, 22, 27, 32, 37                                *
 * i3 (+-1) : 3, 8, 13, 18, 23, 28, 33, 38                                *
 *            4, 9, 14, 19, 24, 29, 34, 39                                *
 *------------------------------------------------------------------------*/

static int16_t D4i40_17_fast(/*(o) : Index of pulses positions.               */
  int16_t dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  int16_t rr[],          /* (i)    : Correlations of impulse response h[].    */
  int16_t h[],           /* (i) Q12: Impulse response of filters.             */
  int16_t cod[],         /* (o) Q13: Selected algebraic codeword.             */
  int16_t y[],           /* (o) Q12: Filtered algebraic codeword.             */
  int16_t *sign          /* (o)    : Signs of 4 pulses.                       */
)
{
  int16_t i0, i1, i2, i3, ip0, ip1, ip2, ip3;
  int16_t i, j, ix, iy, track, trk, max;
  int16_t prev_i0, i1_offset;
  int16_t psk, ps, ps0, ps1, ps2, sq, sq2;
  int16_t alpk, alp, alp_16;
  int32_t s, alp0, alp1, alp2;
  int16_t *p0, *p1, *p2, *p3, *p4;
  int16_t sign_dn[L_SUBFR], sign_dn_inv[L_SUBFR], *psign;
  int16_t tmp_vect[NB_POS];
  int16_t *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  int16_t *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  int16_t *rri1i2, *rri1i3, *rri1i4;
  int16_t *rri2i3, *rri2i4;

  int16_t  *ptr_rri0i3_i4;
  int16_t  *ptr_rri1i3_i4;
  int16_t  *ptr_rri2i3_i4;
  int16_t  *ptr_rri3i3_i4;

     /* Init pointers */
   rri0i0 = rr;
   rri1i1 = rri0i0 + NB_POS;
   rri2i2 = rri1i1 + NB_POS;
   rri3i3 = rri2i2 + NB_POS;
   rri4i4 = rri3i3 + NB_POS;
   rri0i1 = rri4i4 + NB_POS;
   rri0i2 = rri0i1 + MSIZE;
   rri0i3 = rri0i2 + MSIZE;
   rri0i4 = rri0i3 + MSIZE;
   rri1i2 = rri0i4 + MSIZE;
   rri1i3 = rri1i2 + MSIZE;
   rri1i4 = rri1i3 + MSIZE;
   rri2i3 = rri1i4 + MSIZE;
   rri2i4 = rri2i3 + MSIZE;

 /*-----------------------------------------------------------------------*
  * Chose the sign of the impulse.                                        *
  *-----------------------------------------------------------------------*/

   for (i=0; i<L_SUBFR; i++)
   {
     if (dn[i] >= 0)
     {
       sign_dn[i] = WEBRTC_SPL_WORD16_MAX;
       sign_dn_inv[i] = WEBRTC_SPL_WORD16_MIN;
     }
     else
     {
       sign_dn[i] = WEBRTC_SPL_WORD16_MIN;
       sign_dn_inv[i] = WEBRTC_SPL_WORD16_MAX;
       dn[i] = negate(dn[i]);
     }
   }

 /*-------------------------------------------------------------------*
  * Modification of rrixiy[] to take signs into account.              *
  *-------------------------------------------------------------------*/

  p0 = rri0i1;
  p1 = rri0i2;
  p2 = rri0i3;
  p3 = rri0i4;

  for(i0=0; i0<L_SUBFR; i0+=STEP)
  {
    psign = sign_dn;
    if (psign[i0] < 0) psign = sign_dn_inv;

    for(i=0, i1=1; i1<L_SUBFR; i1+=STEP,i++)
    {
      p0[i] = mult(p0[i], psign[i1]);
      p1[i] = mult(p1[i], psign[i1+1]);
      p2[i] = mult(p2[i], psign[i1+2]);
      p3[i] = mult(p3[i], psign[i1+3]);
    }
  }

  p0 = rri1i2;
  p1 = rri1i3;
  p2 = rri1i4;

  for(i1=1; i1<L_SUBFR; i1+=STEP)
  {
    psign = sign_dn;
    if (psign[i1] < 0) psign = sign_dn_inv;

    for(i=0,i2=2; i2<L_SUBFR; i2+=STEP,i++)
    {
      p0[i] = mult(p0[i], psign[i2]);
      p1[i] = mult(p1[i], psign[i2+1]);
      p2[i] = mult(p2[i], psign[i2+2]);
    }
  }

  p0 = rri2i3;
  p1 = rri2i4;

  for(i2=2; i2<L_SUBFR; i2+=STEP)
  {
    psign = sign_dn;
    if (psign[i2] < 0) psign = sign_dn_inv;

    for(i=0,i3=3; i3<L_SUBFR; i3+=STEP,i++)
    {
      p0[i] = mult(p0[i], psign[i3]);
      p1[i] = mult(p1[i], psign[i3+1]);
    }
  }


 /*-------------------------------------------------------------------*
  * Search the optimum positions of the four pulses which maximize    *
  *     square(correlation) / energy                                  *
  *-------------------------------------------------------------------*/

  psk = -1;
  alpk = 1;

  ptr_rri0i3_i4 = rri0i3;
  ptr_rri1i3_i4 = rri1i3;
  ptr_rri2i3_i4 = rri2i3;
  ptr_rri3i3_i4 = rri3i3;

  /* Initializations only to remove warning from some compilers */

  ip0=0; ip1=1; ip2=2; ip3=3; ix=0; iy=0; ps=0;

  /* search 2 times: track 3 and 4 */
  for (track=3, trk=0; track<5; track++, trk++)
  {
   /*------------------------------------------------------------------*
    * depth first search 3, phase A: track 2 and 3/4.                  *
    *------------------------------------------------------------------*/

    sq = -1;
    alp = 1;

    /* i0 loop: 2 positions in track 2 */

    prev_i0  = -1;

    for (i=0; i<2; i++)
    {
      max = -1;
      /* search "dn[]" maximum position in track 2 */
      for (j=2; j<L_SUBFR; j+=STEP)
      {
        if ((dn[j] > max) && (prev_i0 !=j))
        {
          max = dn[j];
          i0 = j;
        }
      }
      prev_i0 = i0;

      j = mult(i0, 6554);        /* j = i0/5 */
      p0 = rri2i2 + j;

      ps1 = dn[i0];
      alp1 = L_mult(*p0, _1_4);

      /* i1 loop: 8 positions in track 2 */

      p0 = ptr_rri2i3_i4 + shl(j, 3);
      p1 = ptr_rri3i3_i4;

      for (i1=track; i1<L_SUBFR; i1+=STEP)
      {
        ps2 = WebRtcSpl_AddSatW16(ps1, dn[i1]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
        alp2 = L_mac(alp1, *p0++, _1_2);
        alp2 = L_mac(alp2, *p1++, _1_4);

        sq2 = mult(ps2, ps2);
        alp_16 = L_round(alp2);

        s = L_msu(L_mult(alp,sq2),sq,alp_16);
        if (s > 0)
        {
          sq = sq2;
          ps = ps2;
          alp = alp_16;
          ix = i0;
          iy = i1;
        }
      }
    }

    i0 = ix;
    i1 = iy;
    i1_offset = shl(mult(i1, 6554), 3);       /* j = 8*(i1/5) */

   /*------------------------------------------------------------------*
    * depth first search 3, phase B: track 0 and 1.                    *
    *------------------------------------------------------------------*/

    ps0 = ps;
    alp0 = L_mult(alp, _1_4);

    sq = -1;
    alp = 1;

    /* build vector for next loop to decrease complexity */

    p0 = rri1i2 + mult(i0, 6554);
    p1 = ptr_rri1i3_i4 + mult(i1, 6554);
    p2 = rri1i1;
    p3 = tmp_vect;

    for (i3=1; i3<L_SUBFR; i3+=STEP)
    {
      /* rrv[i3] = rr[i3][i3] + rr[i0][i3] + rr[i1][i3]; */
      s = L_mult(*p0, _1_4);        p0 += NB_POS;
      s = L_mac(s, *p1, _1_4);      p1 += NB_POS;
      s = L_mac(s, *p2++, _1_8);
      *p3++ = L_round(s);
    }

    /* i2 loop: 8 positions in track 0 */

    p0 = rri0i2 + mult(i0, 6554);
    p1 = ptr_rri0i3_i4 + mult(i1, 6554);
    p2 = rri0i0;
    p3 = rri0i1;

    for (i2=0; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = WebRtcSpl_AddSatW16(ps0, dn[i2]);         /* index increment = STEP */

      /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */
      alp1 = L_mac(alp0, *p0, _1_8);       p0 += NB_POS;
      alp1 = L_mac(alp1, *p1, _1_8);       p1 += NB_POS;
      alp1 = L_mac(alp1, *p2++, _1_16);

      /* i3 loop: 8 positions in track 1 */

      p4 = tmp_vect;

      for (i3=1; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = WebRtcSpl_AddSatW16(ps1, dn[i3]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i3] + rr[i1][i3] + rr[i2][i3] + 1/2*rr[i3][i3]; */
        alp2 = L_mac(alp1, *p3++, _1_8);
        alp2 = L_mac(alp2, *p4++, _1_2);

        sq2 = mult(ps2, ps2);
        alp_16 = L_round(alp2);

        s = L_msu(L_mult(alp,sq2),sq,alp_16);
        if (s > 0)
        {
          sq = sq2;
          alp = alp_16;
          ix = i2;
          iy = i3;
        }
      }
    }

   /*----------------------------------------------------------------*
    * depth first search 3: compare codevector with the best case.   *
    *----------------------------------------------------------------*/

    s = L_msu(L_mult(alpk,sq),psk,alp);
    if (s > 0)
    {
      psk = sq;
      alpk = alp;
      ip2 = i0;
      ip3 = i1;
      ip0 = ix;
      ip1 = iy;
    }

   /*------------------------------------------------------------------*
    * depth first search 4, phase A: track 3 and 0.                    *
    *------------------------------------------------------------------*/

    sq = -1;
    alp = 1;

    /* i0 loop: 2 positions in track 3/4 */

    prev_i0  = -1;

    for (i=0; i<2; i++)
    {
      max = -1;
      /* search "dn[]" maximum position in track 3/4 */
      for (j=track; j<L_SUBFR; j+=STEP)
      {
        if ((dn[j] > max) && (prev_i0 != j))
        {
          max = dn[j];
          i0 = j;
        }
      }
      prev_i0 = i0;

      j = mult(i0, 6554);        /* j = i0/5 */
      p0 = ptr_rri3i3_i4 + j;

      ps1 = dn[i0];
      alp1 = L_mult(*p0, _1_4);

      /* i1 loop: 8 positions in track 0 */

      p0 = ptr_rri0i3_i4 + j;
      p1 = rri0i0;

      for (i1=0; i1<L_SUBFR; i1+=STEP)
      {
        ps2 = WebRtcSpl_AddSatW16(ps1, dn[i1]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
        alp2 = L_mac(alp1, *p0, _1_2);       p0 += NB_POS;
        alp2 = L_mac(alp2, *p1++, _1_4);

        sq2 = mult(ps2, ps2);
        alp_16 = L_round(alp2);

        s = L_msu(L_mult(alp,sq2),sq,alp_16);
        if (s > 0)
        {
          sq = sq2;
          ps = ps2;
          alp = alp_16;
          ix = i0;
          iy = i1;
        }
      }
    }

    i0 = ix;
    i1 = iy;
    i1_offset = shl(mult(i1, 6554), 3);       /* j = 8*(i1/5) */

   /*------------------------------------------------------------------*
    * depth first search 4, phase B: track 1 and 2.                    *
    *------------------------------------------------------------------*/

    ps0 = ps;
    alp0 = L_mult(alp, _1_4);

    sq = -1;
    alp = 1;

    /* build vector for next loop to decrease complexity */

    p0 = ptr_rri2i3_i4 + mult(i0, 6554);
    p1 = rri0i2 + i1_offset;
    p2 = rri2i2;
    p3 = tmp_vect;

    for (i3=2; i3<L_SUBFR; i3+=STEP)
    {
      /* rrv[i3] = rr[i3][i3] + rr[i0][i3] + rr[i1][i3]; */
      s = L_mult(*p0, _1_4);         p0 += NB_POS;
      s = L_mac(s, *p1++, _1_4);
      s = L_mac(s, *p2++, _1_8);
      *p3++ = L_round(s);
    }

    /* i2 loop: 8 positions in track 1 */

    p0 = ptr_rri1i3_i4 + mult(i0, 6554);
    p1 = rri0i1 + i1_offset;
    p2 = rri1i1;
    p3 = rri1i2;

    for (i2=1; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = WebRtcSpl_AddSatW16(ps0, dn[i2]);         /* index increment = STEP */

      /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */
      alp1 = L_mac(alp0, *p0, _1_8);       p0 += NB_POS;
      alp1 = L_mac(alp1, *p1++, _1_8);
      alp1 = L_mac(alp1, *p2++, _1_16);

      /* i3 loop: 8 positions in track 2 */

      p4 = tmp_vect;

      for (i3=2; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = WebRtcSpl_AddSatW16(ps1, dn[i3]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i3] + rr[i1][i3] + rr[i2][i3] + 1/2*rr[i3][i3]; */
        alp2 = L_mac(alp1, *p3++, _1_8);
        alp2 = L_mac(alp2, *p4++, _1_2);

        sq2 = mult(ps2, ps2);
        alp_16 = L_round(alp2);

        s = L_msu(L_mult(alp,sq2),sq,alp_16);
        if (s > 0)
        {
          sq = sq2;
          alp = alp_16;
          ix = i2;
          iy = i3;
        }
      }
    }

   /*----------------------------------------------------------------*
    * depth first search 1: compare codevector with the best case.   *
    *----------------------------------------------------------------*/

    s = L_msu(L_mult(alpk,sq),psk,alp);
    if (s > 0)
    {
      psk = sq;
      alpk = alp;
      ip3 = i0;
      ip0 = i1;
      ip1 = ix;
      ip2 = iy;
    }

  ptr_rri0i3_i4 = rri0i4;
  ptr_rri1i3_i4 = rri1i4;
  ptr_rri2i3_i4 = rri2i4;
  ptr_rri3i3_i4 = rri4i4;

  }


 /* Set the sign of impulses */

 i0 = sign_dn[ip0];
 i1 = sign_dn[ip1];
 i2 = sign_dn[ip2];
 i3 = sign_dn[ip3];

 /* Find the codeword corresponding to the selected positions */

 WebRtcSpl_ZerosArrayW16(cod, L_SUBFR);

 cod[ip0] = shr(i0, 2);         /* From Q15 to Q13 */
 cod[ip1] = shr(i1, 2);
 cod[ip2] = shr(i2, 2);
 cod[ip3] = shr(i3, 2);

 /* find the filtered codeword */

 WebRtcSpl_ZerosArrayW16(y, ip0);

 if (i0 > 0)
   WEBRTC_SPL_MEMCPY_W16(&y[ip0], h, L_SUBFR - ip0);
 else
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = negate(h[j]);

 if (i1 > 0)
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_AddSatW16(y[i], h[j]);
 else
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_SubSatW16(y[i], h[j]);

 if (i2 > 0)
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_AddSatW16(y[i], h[j]);
 else
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_SubSatW16(y[i], h[j]);

 if (i3 > 0)
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_AddSatW16(y[i], h[j]);
 else
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = WebRtcSpl_SubSatW16(y[i], h[j]);

 /* find codebook index;  17-bit address */

 i = 0;
 if (i0 > 0) i = WebRtcSpl_AddSatW16(i, 1);
 if (i1 > 0) i = WebRtcSpl_AddSatW16(i, 2);
 if (i2 > 0) i = WebRtcSpl_AddSatW16(i, 4);
 if (i3 > 0) i = WebRtcSpl_AddSatW16(i, 8);
 *sign = i;

 ip0 = mult(ip0, 6554);         /* ip0/5 */
 ip1 = mult(ip1, 6554);         /* ip1/5 */
 ip2 = mult(ip2, 6554);         /* ip2/5 */
 i   = mult(ip3, 6554);         /* ip3/5 */
 j   = WebRtcSpl_AddSatW16(i, shl(i, 2));       /* j = i*5 */
 j   = WebRtcSpl_SubSatW16(ip3, WebRtcSpl_AddSatW16(j, 3));     /* j= ip3%5 -3 */
 ip3 = WebRtcSpl_AddSatW16(shl(i, 1), j);

 i = WebRtcSpl_AddSatW16(ip0, shl(ip1, 3));
 i = WebRtcSpl_AddSatW16(i  , shl(ip2, 6));
 i = WebRtcSpl_AddSatW16(i  , shl(ip3, 9));

 return i;
}
