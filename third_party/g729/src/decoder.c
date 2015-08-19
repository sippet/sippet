/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.5    Last modified: October 2006

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 * Main program of the G.729A 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include "basic_op.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"


/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
  int16_t  synth_buf[L_FRAME+M], *synth; /* Synthesis                   */
  int16_t  parm[PRM_SIZE+2];             /* Synthesis parameters        */
  int16_t  Az_dec[MP1*2];                /* Decoded Az for post-filter  */
  int16_t  T2[2];                        /* Pitch lag for 2 subframes   */


  int16_t  i, Vad;
  int32_t  count_frame;
  FILE   *f_syn, *f_serial;

  int32_t stop_after = WEBRTC_SPL_WORD32_MAX;

  Decod_ld8a_state state;
  Post_Filter_state post_filter_state;
  Post_Process_state post_proc_state;

  printf("\n");
  printf("************  ITU G.729A 8.0 KBIT/S SPEECH DECODER  ************\n");
  printf("                       (WITH ANNEX B)                           \n");
  printf("\n");
  printf("------------------ Fixed point C simulation --------------------\n");
  printf("\n");
  printf("------------ Version 1.5 (Release 2, November 2006) ------------\n");
  printf("\n");

   /* Passed arguments */

  if (argc != 3 && argc != 4) {
    printf("Usage :%s bitstream_file  outputspeech_file\n",argv[0]);
    printf("\n");
    printf("Format for bitstream_file:\n");
    printf("  One (2-byte) synchronization word \n");
    printf("  One (2-byte) size word,\n");
    printf("  80 words (2-byte) containing 80 bits.\n");
    printf("\n");
    printf("Format for outputspeech_file:\n");
    printf("  Synthesis is written to a binary file of 16 bits data.\n");
    exit( 1 );
  }

  /* Open file for synthesis and packed serial stream */

  if( (f_serial = fopen(argv[1],"rb") ) == NULL )
    {
      printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
      exit(0);
    }

  if( (f_syn = fopen(argv[2], "wb") ) == NULL )
    {
      printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
      exit(0);
    }

  printf("Input bitstream file  :   %s\n",argv[1]);
  printf("Synthesis speech file :   %s\n",argv[2]);

#ifndef OCTET_TX_MODE
  printf("OCTET TRANSMISSION MODE is disabled\n");
#endif

  if (argc == 4) {
    stop_after = atoi(argv[3]);
  }

  WebRtcSpl_Init();

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/

  WebRtcSpl_ZerosArrayW16(synth_buf, M);
  synth = synth_buf + M;

  WebRtcG729fix_Init_Decod_ld8a(&state);
  WebRtcG729fix_Init_Post_Filter(&post_filter_state);
  WebRtcG729fix_Init_Post_Process(&post_proc_state);

  /* for G.729b */
  WebRtcG729fix_Init_Dec_cng(&state);

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

  count_frame = 0L;
  while (WebRtcG729fix_read_frame(f_serial, parm) != 0)
  {
    if (count_frame < stop_after) {
      printf("Frame = %d\r", count_frame++);
      WebRtcG729fix_Decod_ld8a(&state, parm, synth, Az_dec, T2, &Vad, 0);
    }
    else
    {
      printf("Erased frame = %d\r", count_frame++);
      WebRtcSpl_ZerosArrayW16(parm, PRM_SIZE+2);
      parm[0] = 1; /* frame erasure */
      WebRtcG729fix_Decod_ld8a(&state, parm, synth, Az_dec, T2, &Vad, 0);
    }

    WebRtcG729fix_Post_Filter(&post_filter_state, synth, Az_dec, T2, Vad); /* Post-filter */
    WebRtcG729fix_Post_Process(&post_proc_state, synth, synth, L_FRAME);

    fwrite(synth, sizeof(short), L_FRAME, f_syn);

  }

  printf("%d frames decoded\n", count_frame);
  return(0);
}




