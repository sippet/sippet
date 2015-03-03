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

/*-------------------------------------------------------------------*
 * Main program of the ITU-T G.729A  8 kbit/s encoder.               *
 *                                                                   *
 *    Usage : g729a_coder speech_file  bitstream_file                *
 *-------------------------------------------------------------------*/

#include <iostream>
#include <fstream>

extern "C" {
#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "g729a.h"
}

using namespace std;

int main(int argc, char *argv[])
{
  FILE *f_speech;        // File of speech data
  FILE *f_serial;        // File of serial bits for transmission

  Word16 speech[L_FRAME];
  UWord8 serial[M];      // Serial stream
  Word16 frame;          // Frame counter
  Word32 size;
  void *encoder_state;

  cout << endl
       << "*********    ITU G.729A 8 KBIT/S SPEECH CODER    *********" << endl
       << endl
       << "----------------- Fixed point C simulation ---------------" << endl
       << endl
       << "------------ Version 1.1 (Release 2, November 2006) ------" << endl
       << endl;

  // Open speech file and result file (output serial bit stream)
  if (argc != 3) {
    cout << "Usage : g729a_coder speech_file  bitstream_file" << endl
         << endl
         << "Format for speech_file:" << endl
         << "  Speech is read from a binary file of 16 bits PCM data." << endl
         << endl
         << "Format for bitstream_file:" << endl
         << "  10 bytes - g729a parameters" << endl
         << endl;
    exit(1);
  }

  if ((f_speech = fopen(argv[1], "rb")) == NULL) {
    cerr << "-- Error opening file " << argv[1] << " !!" << endl;
    exit(0);
  }
  cout << " Input speech file    :  " << argv[1] << endl;

  if ((f_serial = fopen(argv[2], "wb")) == NULL) {
    cerr << "-- Error opening file " << argv[2] << " !!" << endl;
    exit(0);
  }
  cout << " Output bitstream file:  " << argv[2] << endl;

  // Initialization of the coder.
  size = g729a_enc_mem_size();
  encoder_state = new UWord8[size];
  if (encoder_state == NULL) {
    cerr << "-- Not enough memory" << endl;
    fclose(f_speech);
    fclose(f_serial);
    exit(0);
  }
  g729a_enc_init(encoder_state);

  // Loop for each "L_FRAME" speech data.
  frame = 0;
  while (fread(speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME) {
    cout << "Frame = " << frame++ << "\r";
    g729a_enc_process(encoder_state, speech, serial);
    fwrite(serial, sizeof(UWord8), M, f_serial);
  }
  cout << "Done!                " << endl;

  g729a_enc_deinit(encoder_state);
  delete [] reinterpret_cast<UWord8*>(encoder_state);
  fclose(f_serial);
  fclose(f_speech);

  return (0);
}
