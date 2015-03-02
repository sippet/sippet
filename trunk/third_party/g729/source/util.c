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

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"

/* Random generator  */

Word16 Random()
{
  static Word16 seed = 21845;

  //seed = (Word32)seed * 31821LL + 13849LL;
  seed = extract_l(L_add(L_shr(L_mult(seed, 31821), 1), 13849L));

  return(seed);
}

