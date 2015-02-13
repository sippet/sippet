/*
 * copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file libavcodec/put_bits.h
 * bitstream writer API
 */

#ifndef AVCODEC_PUT_BITS_H
#define AVCODEC_PUT_BITS_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
//#include "libavutil/bswap.h"
static uint32_t bswap_32(uint32_t x)
{
#if defined(ARCH_ARM)
  __asm__("rev %0, %0" : "+r"(x));
#else
  x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
  x= (x>>16) | (x<<16);
#endif
  return x;
}
#ifdef WORDS_BIGENDIAN
#define be2me_32(x) (x)
#define le2me_32(x) bswap_32(x)
#else
#define be2me_32(x) bswap_32(x)
#define le2me_32(x) (x)
#endif
//#include "libavutil/common.h"
//#include "libavutil/intreadwrite.h"
#define AV_WB32(p, d) do { \
  ((uint8_t*)(p))[3] = (d); \
  ((uint8_t*)(p))[2] = (d)>>8; \
  ((uint8_t*)(p))[1] = (d)>>16; \
  ((uint8_t*)(p))[0] = (d)>>24; } while(0)
//#include "libavutil/log.h"
//#include "mathops.h"

//#define ALT_BITSTREAM_WRITER
//#define ALIGNED_BITSTREAM_WRITER
//#define BITSTREAM_WRITER_LE

/* buf and buf_end must be present and used by every alternative writer. */
typedef struct PutBitContext {
#ifdef ALT_BITSTREAM_WRITER
  uint8_t *buf, *buf_end;
  int index;
#else
  uint32_t bit_buf;
  int bit_left;
  uint8_t *buf, *buf_ptr, *buf_end;
#endif
  int size_in_bits;
} PutBitContext;

/**
 * Initializes the PutBitContext s.
 *
 * @param buffer the buffer where to put bits
 * @param buffer_size the size in bytes of buffer
 */
static void init_put_bits(PutBitContext *s, uint8_t *buffer, int buffer_size)
{
  if(buffer_size < 0) {
    buffer_size = 0;
    buffer = NULL;
  }
  
  s->size_in_bits= 8*buffer_size;
  s->buf = buffer;
  s->buf_end = s->buf + buffer_size;
#ifdef ALT_BITSTREAM_WRITER
  s->index=0;
  ((uint32_t*)(s->buf))[0]=0;
  //    memset(buffer, 0, buffer_size);
#else
  s->buf_ptr = s->buf;
  s->bit_left=32;
  s->bit_buf=0;
#endif
}

/**
 * Returns the total number of bits written to the bitstream.
 */
static int put_bits_count(PutBitContext *s)
{
#ifdef ALT_BITSTREAM_WRITER
  return s->index;
#else
  return (s->buf_ptr - s->buf) * 8 + 32 - s->bit_left;
#endif
}

/**
 * Pads the end of the output stream with zeros.
 */
static void flush_put_bits(PutBitContext *s)
{
#ifdef ALT_BITSTREAM_WRITER
  align_put_bits(s);
#else
#ifndef BITSTREAM_WRITER_LE
  s->bit_buf<<= s->bit_left;
#endif
  while (s->bit_left < 32) {
    /* XXX: should test end of buffer */
#ifdef BITSTREAM_WRITER_LE
    *s->buf_ptr++=s->bit_buf;
    s->bit_buf>>=8;
#else
    *s->buf_ptr++=s->bit_buf >> 24;
    s->bit_buf<<=8;
#endif
    s->bit_left+=8;
  }
  s->bit_left=32;
  s->bit_buf=0;
#endif
}

/**
 * Pads the bitstream with zeros up to the next byte boundary.
 */
//void align_put_bits(PutBitContext *s);

/**
 * Puts the string s in the bitstream.
 *
 * @param terminate_string 0-terminates the written string if value is 1
 */
//void ff_put_string(PutBitContext * pbc, const char *s, int terminate_string);

/**
 * Copies the content of src to the bitstream.
 *
 * @param length the number of bits of src to copy
 */
//void ff_copy_bits(PutBitContext *pb, const uint8_t *src, int length);

static void put_bits(PutBitContext *s, int n, unsigned int value)
#ifndef ALT_BITSTREAM_WRITER
{
  unsigned int bit_buf;
  int bit_left;
  
  //    printf("put_bits=%d %x\n", n, value);
  assert(n == 32 || value < (1U << n));
  
  bit_buf = s->bit_buf;
  bit_left = s->bit_left;
  
  //    printf("n=%d value=%x cnt=%d buf=%x\n", n, value, bit_cnt, bit_buf);
  /* XXX: optimize */
#ifdef BITSTREAM_WRITER_LE
  bit_buf |= value << (32 - bit_left);
  if (n >= bit_left) {
#if !HAVE_FAST_UNALIGNED
    if (3 & (intptr_t) s->buf_ptr) {
      AV_WL32(s->buf_ptr, bit_buf);
    } else
#endif
      *(uint32_t *)s->buf_ptr = le2me_32(bit_buf);
    s->buf_ptr+=4;
    bit_buf = (bit_left==32)?0:value >> bit_left;
    bit_left+=32;
  }
  bit_left-=n;
#else
  if (n < bit_left) {
    bit_buf = (bit_buf<<n) | value;
    bit_left-=n;
  } else {
    bit_buf<<=bit_left;
    bit_buf |= value >> (n - bit_left);
#if !HAVE_FAST_UNALIGNED
    if (3 & (intptr_t) s->buf_ptr) {
      AV_WB32(s->buf_ptr, bit_buf);
    } else
#endif
      *(uint32_t *)s->buf_ptr = be2me_32(bit_buf);
    //printf("bitbuf = %08x\n", bit_buf);
    s->buf_ptr+=4;
    bit_left+=32 - n;
    bit_buf = value;
  }
#endif
  
  s->bit_buf = bit_buf;
  s->bit_left = bit_left;
}
#else  /* ALT_BITSTREAM_WRITER defined */
{
#    ifdef ALIGNED_BITSTREAM_WRITER
#        if defined(ARCH_X86)
  __asm__ volatile(
                   "movl %0, %%ecx                 \n\t"
                   "xorl %%eax, %%eax              \n\t"
                   "shrdl %%cl, %1, %%eax          \n\t"
                   "shrl %%cl, %1                  \n\t"
                   "movl %0, %%ecx                 \n\t"
                   "shrl $3, %%ecx                 \n\t"
                   "andl $0xFFFFFFFC, %%ecx        \n\t"
                   "bswapl %1                      \n\t"
                   "orl %1, (%2, %%ecx)            \n\t"
                   "bswapl %%eax                   \n\t"
                   "addl %3, %0                    \n\t"
                   "movl %%eax, 4(%2, %%ecx)       \n\t"
                   : "=&r" (s->index), "=&r" (value)
                   : "r" (s->buf), "r" (n), "0" (s->index), "1" (value<<(-n))
                   : "%eax", "%ecx"
                   );
#        else
  int index= s->index;
  uint32_t *ptr= ((uint32_t *)s->buf)+(index>>5);
  
  value<<= 32-n;
  
  ptr[0] |= be2me_32(value>>(index&31));
  ptr[1]  = be2me_32(value<<(32-(index&31)));
  //if(n>24) printf("%d %d\n", n, value);
  index+= n;
  s->index= index;
#        endif
#    else //ALIGNED_BITSTREAM_WRITER
#        if defined(ARCH_X86)
  __asm__ volatile(
                   "movl $7, %%ecx                 \n\t"
                   "andl %0, %%ecx                 \n\t"
                   "addl %3, %%ecx                 \n\t"
                   "negl %%ecx                     \n\t"
                   "shll %%cl, %1                  \n\t"
                   "bswapl %1                      \n\t"
                   "movl %0, %%ecx                 \n\t"
                   "shrl $3, %%ecx                 \n\t"
                   "orl %1, (%%ecx, %2)            \n\t"
                   "addl %3, %0                    \n\t"
                   "movl $0, 4(%%ecx, %2)          \n\t"
                   : "=&r" (s->index), "=&r" (value)
                   : "r" (s->buf), "r" (n), "0" (s->index), "1" (value)
                   : "%ecx"
                   );
#        else
  int index= s->index;
  uint32_t *ptr= (uint32_t*)(((uint8_t *)s->buf)+(index>>3));
  
  ptr[0] |= be2me_32(value<<(32-n-(index&7) ));
  ptr[1] = 0;
  //if(n>24) printf("%d %d\n", n, value);
  index+= n;
  s->index= index;
#        endif
#    endif //!ALIGNED_BITSTREAM_WRITER
  }
#endif
  
  static void put_sbits(PutBitContext *pb, int bits, int32_t val)
  {
    assert(bits >= 0 && bits <= 31);
    
    put_bits(pb, bits, val & ((1<<bits)-1));
  }
  
  /**
   * Returns the pointer to the byte where the bitstream writer will put
   * the next bit.
   */
  static uint8_t* put_bits_ptr(PutBitContext *s)
  {
#ifdef ALT_BITSTREAM_WRITER
    return s->buf + (s->index>>3);
#else
    return s->buf_ptr;
#endif
  }
  
  /**
   * Skips the given number of bytes.
   * PutBitContext must be flushed & aligned to a byte boundary before calling this.
   */
  static void skip_put_bytes(PutBitContext *s, int n){
    assert((put_bits_count(s)&7)==0);
#ifdef ALT_BITSTREAM_WRITER
    FIXME may need some cleaning of the buffer
    s->index += n<<3;
#else
    assert(s->bit_left==32);
    s->buf_ptr += n;
#endif
  }
  
  /**
   * Skips the given number of bits.
   * Must only be used if the actual values in the bitstream do not matter.
   * If n is 0 the behavior is undefined.
   */
  static void skip_put_bits(PutBitContext *s, int n){
#ifdef ALT_BITSTREAM_WRITER
    s->index += n;
#else
    s->bit_left -= n;
    s->buf_ptr-= 4*(s->bit_left>>5);
    s->bit_left &= 31;
#endif
  }
  
  /**
   * Changes the end of the buffer.
   *
   * @param size the new size in bytes of the buffer where to put bits
   */
  static void set_put_bits_buffer_size(PutBitContext *s, int size){
    s->buf_end= s->buf + size;
  }
  
#endif /* AVCODEC_PUT_BITS_H */
