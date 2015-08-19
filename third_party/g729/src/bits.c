/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ld8a.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "octet.h"

/* prototypes for local functions */
static void  int2bin(int16_t value, int16_t no_of_bits, int16_t *bitstream);
static int16_t   bin2int(int16_t no_of_bits, int16_t *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters are:
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *----------------------------------------------------------------------------
 */
void WebRtcG729fix_prm2bits_ld8k(
 int16_t   prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  int16_t bits[]           /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = 80 */
)
{
  int16_t i;
  *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */

  switch(prm[0]){

    /* not transmitted */
  case 0 : {
    *bits = RATE_0;
    break;
  }

  case 1 : {
    *bits++ = RATE_8000;
    for (i = 0; i < PRM_SIZE; i++) {
      int2bin(prm[i+1], WebRtcG729fix_bitsno[i], bits);
      bits += WebRtcG729fix_bitsno[i];
    }
    break;
  }

  case 2 : {

#ifndef OCTET_TX_MODE
    *bits++ = RATE_SID;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], WebRtcG729fix_bitsno2[i], bits);
      bits += WebRtcG729fix_bitsno2[i];
    }
#else
    *bits++ = RATE_SID_OCTET;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], WebRtcG729fix_bitsno2[i], bits);
      bits += WebRtcG729fix_bitsno2[i];
    }
    *bits++ = BIT_0;
#endif

    break;
  }

  default : {
    printf("Unrecognized frame type\n");
    exit(-1);
  }

  }

  return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 int16_t value,             /* input : decimal value */
 int16_t no_of_bits,        /* input : number of bits to use */
 int16_t *bitstream       /* output: bitstream  */
)
{
   int16_t *pt_bitstream;
   int16_t   i, bit;

   pt_bitstream = bitstream + no_of_bits;

   for (i = 0; i < no_of_bits; i++)
   {
     bit = value & (int16_t)0x0001;      /* get lsb */
     if (bit == 0)
         *--pt_bitstream = BIT_0;
     else
         *--pt_bitstream = BIT_1;
     value >>= 1;
   }
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void WebRtcG729fix_bits2prm_ld8k(
 int16_t bits[],          /* input : serial bits (80)                       */
 int16_t   prm[]          /* output: decoded parameters (11 parameters)     */
)
{
  int16_t i;
  int16_t nb_bits;

  nb_bits = *bits++;        /* Number of bits in this frame       */

  if(nb_bits == RATE_8000) {
    prm[1] = 1;
    for (i = 0; i < PRM_SIZE; i++) {
      prm[i+2] = bin2int(WebRtcG729fix_bitsno[i], bits);
      bits  += WebRtcG729fix_bitsno[i];
    }
  }
  else
#ifndef OCTET_TX_MODE
    if(nb_bits == RATE_SID) {
      prm[1] = 2;
      for (i = 0; i < 4; i++) {
        prm[i+2] = bin2int(WebRtcG729fix_bitsno2[i], bits);
        bits += WebRtcG729fix_bitsno2[i];
      }
    }
#else
  /* the last bit of the SID bit stream under octet mode will be discarded */
  if(nb_bits == RATE_SID_OCTET) {
    prm[1] = 2;
    for (i = 0; i < 4; i++) {
      prm[i+2] = bin2int(WebRtcG729fix_bitsno2[i], bits);
      bits += WebRtcG729fix_bitsno2[i];
    }
  }
#endif

  else {
    prm[1] = 0;
  }
  return;

}

/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static int16_t bin2int(            /* output: decimal value of bit pattern */
 int16_t no_of_bits,        /* input : number of bits to read */
 int16_t *bitstream       /* input : array containing bits */
)
{
   int16_t   value, i;
   int16_t bit;

   value = 0;
   for (i = 0; i < no_of_bits; i++)
   {
     value <<= 1;
     bit = *bitstream++;
     if (bit == BIT_1)  value += 1;
   }
   return(value);
}

int16_t WebRtcG729fix_read_frame(FILE *f_serial, int16_t *parm)
{
  int16_t  i;
  int16_t  serial[SERIAL_SIZE];          /* Serial stream               */

  if(fread(serial, sizeof(short), 2, f_serial) != 2) {
    return(0);
  }

  if(fread(&serial[2], sizeof(int16_t), (size_t)serial[1], f_serial)
     != (size_t)serial[1]) {
    return(0);
  }

  WebRtcG729fix_bits2prm_ld8k(&serial[1], parm);

  /* This part was modified for version V1.3 */
  /* for speech and SID frames, the hardware detects frame erasures
     by checking if all bits are set to zero */
  /* for untransmitted frames, the hardware detects frame erasures
     by testing serial[0] */

  parm[0] = 0;           /* No frame erasure */
  if(serial[1] != 0) {
   for (i=0; i < serial[1]; i++)
     if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
  }
  else if(serial[0] != SYNC_WORD) parm[0] = 1;

  if(parm[1] == 1) {
    /* check parity and put 1 in parm[5] if parity error */
    parm[5] = WebRtcG729fix_Check_Parity_Pitch(parm[4], parm[5]);
  }

  return(1);
}
