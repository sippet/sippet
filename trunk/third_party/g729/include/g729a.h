// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __G729A_H__
#define __G729A_H__

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * g729a_dec_mem_size:
   *
   * parameters:
   *    None
   *
   * return:
   *    Memory block size in bytes for decoder.
   */
Word32 g729a_dec_mem_size ();

  /**
   * g729a_dec_init:
   * Initializes resources needed for a new instance of the decoder.
   *
   * parameters:
   *    decState : Pre-allocated memory block of size defined by a call to
   *                <ref>g729a_dec_mem_size</ref>
   *
   * return:
   *    0, if an error occurs
   *    1, otherwise.
   */
Flag   g729a_dec_init     (void *decState);

  /**
   * g729a_dec_process
   * Decodes one frame of g729a encoded bitstream data.
   *
   * parameters:
   *    decState  :
   *    bitstream : Buffer containing one frame of g729a encoded bitstream data.
   *                (10 bytes)
   *    pcm       : Buffer containing one frame of pcm data. (80 bytes)
   *    badFrame  : set to 1 to indicate a bad frame to the decoder, 0 otherwise.
   *
   * return:
   *
   */
void   g729a_dec_process  (void *decState, UWord8 *bitstream, Word16 *pcm,
                           Flag badFrame);

  /**
   * g729a_dec_deinit
   *
   * parameters:
   *    decState :
   *
   * return:
   *    None
   */
void   g729a_dec_deinit   (void *decState);

  /**
   * g729a_enc_mem_size:
   *
   * parameters:
   *    None
   *
   * return:
   *    Memory block size in bytes for decoder.
   */
Word32 g729a_enc_mem_size ();

  /**
   * g729a_enc_init:
   * Initializes resources needed for a new instance of the encoder.
   *
   * parameters:
   *    encState : Pre-allocated memory block of size defined by a call to
   *               <ref>g729a_enc_mem_size</ref>
   *
   * return:
   *    0, if an error occurs
   *    1, otherwise.
   */
Flag   g729a_enc_init     (void *encState);

  /**
   * g729a_enc_process
   * Encode one frame of 16-bit linear PCM data.
   *
   * parameters:
   *    encState  :
   *    pcm       : Buffer containing one frame of pcm data. (80 bytes)
   *    bitstream : Buffer containing one frame of g729a encoded bitstream data.
   *                (10 bytes)
   *
   * return:
   *    None
   */
void   g729a_enc_process  (void *encState, Word16 *pcm, UWord8 *bitstream);

  /**
   * g729a_enc_deinit
   *
   * parameters:
   *    encState :
   *
   * return:
   *    None
   */
void   g729a_enc_deinit   (void *encState);


#ifdef __cplusplus
  }
#endif


#endif /* __G729A_H__ */
