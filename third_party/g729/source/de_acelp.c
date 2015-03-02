// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/****************************************************************************************
Portions of this file are derived from the following ITU standard:
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
****************************************************************************************/

/*-----------------------------------------------------------*
 *  Function  Decod_ACELP()                                  *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"

void Decod_ACELP(
  Word16 sign,      /* (i)     : signs of 4 pulses.                       */
  Word16 index,     /* (i)     : Positions of the 4 pulses.               */
  Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
)
{
  Word16 i, j;
  Word16 pos[4];


  /* Decode the positions */

  i      = index & (Word16)7;
  pos[0] = 5*i;

  index  >>= 3;
  i      = index & (Word16)7;
  pos[1] = 5*i+1;

  index >>= 3;
  i      = index & (Word16)7;
  pos[2] = 5*i+2;

  index >>= 3;
  j      = index & (Word16)1;
  index >>= 1;
  i      = index & (Word16)7;
  pos[3] = 5*i+3+j;

  /* decode the signs  and build the codeword */
  Set_zero(cod, L_SUBFR);

  for (j=0; j<4; j++)
  {

    i = sign & (Word16)1;
    sign >>= 1;

		/* Q13 +1.0 or  Q13 -1.0 */
		cod[pos[j]] = (i != 0 ? 8191 : -8192);
  }
}

