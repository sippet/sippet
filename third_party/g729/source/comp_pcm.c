// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"


int main(int argc, char *argv[] )
{
  FILE *f_speech;

  int16_t frame[2*L_FRAME];
  int32_t autocorr[M];
  int scale, i, count = 0;

  if ( argc != 2 ){
    printf("Usage :%s speech_file\n", argv[0]);
    printf("\n");
    printf("Format for speech_file:\n");
    printf("  Speech is read from a binary file of 16 bits PCM data.\n");
    printf("\n");
    exit(1);
  }

  if ( (f_speech = fopen(argv[1], "rb")) == NULL) {
     printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
     exit(0);
   }
  printf(" Input speech file:  %s\n", argv[1]);

  WebRtcSpl_Init();

  while (fread(frame, sizeof(int16_t), 2*L_FRAME, f_speech) == 2*L_FRAME)
  {
    WebRtcSpl_AutoCorrelation(frame, 2 * L_FRAME, M, autocorr, &scale);
    printf("%d = [%d", count++, scale);
    for (i = 0; i < M; i++)
      printf(", %ld", autocorr[i]);
    printf("]\n");
  }

  return 0;
}




