/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.5    Last modified: October 2006

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#ifndef __BASIC_OP_H__
#define __BASIC_OP_H__

#include "signal_processing_library.h"

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/

static __inline int16_t abs_s(int16_t var1);                /* Short abs,           1 */
static __inline int16_t shl(int16_t var1, int16_t var2);    /* Short shift left,    1 */
static __inline int16_t shr(int16_t var1, int16_t var2);    /* Short shift right,   1 */
static __inline int16_t mult(int16_t var1, int16_t var2);   /* Short mult,          1 */
static __inline int32_t L_mult(int16_t var1, int16_t var2); /* Long mult,           1 */
static __inline int16_t negate(int16_t var1);               /* Short negate,        1 */
static __inline int16_t extract_h(int32_t L_var1);          /* Extract high,        1 */
static __inline int16_t extract_l(int32_t L_var1);          /* Extract low,         1 */
static __inline int16_t L_round(int32_t L_var1);            /* Round,               1 */
static __inline int32_t L_mac(int32_t L_var3, int16_t var1, int16_t var2); /* Mac,  1 */
static __inline int32_t L_msu(int32_t L_var3, int16_t var1, int16_t var2); /* Msu,  1 */

static __inline int32_t L_negate(int32_t L_var1);               /* Long negate,     2 */
static __inline int16_t mult_r(int16_t var1, int16_t var2);  /* Mult with round,    2 */
static __inline int32_t L_shl(int32_t L_var1, int16_t var2); /* Long shift left,    2 */
static __inline int32_t L_shr(int32_t L_var1, int16_t var2); /* Long shift right,   2 */
static __inline int16_t shr_r(int16_t var1, int16_t var2); /* Shift right w/ round, 2 */
static __inline int16_t msu_r(int32_t L_var3, int16_t var1, int16_t var2);/* Msu w/ rounding, 2*/
static __inline int32_t L_deposit_h(int16_t var1);       /* 16 bit var1 -> MSB,     2 */
static __inline int32_t L_deposit_l(int16_t var1);       /* 16 bit var1 -> LSB,     2 */

static __inline int32_t L_shr_r(int32_t L_var1, int16_t var2);/* Long shift right w/ round,  3*/
static __inline int32_t L_abs(int32_t L_var1);            /* Long abs,              3 */

static __inline int16_t div_s(int16_t var1, int16_t var2); /* Short division,      18 */

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : abs_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of var1; abs_s(-32768) = 32767.                         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0x0000 0000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t abs_s(int16_t var1)
{
  int16_t var_out;

  if (var1 == WEBRTC_SPL_WORD16_MIN) {
    var_out = WEBRTC_SPL_WORD16_MAX;
  } else {
    var_out = WEBRTC_SPL_ABS_W16(var1);
  }

  return var_out;
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : shl                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
 |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
 |   var1 right by -var2 with sign extension. Saturate the result in case of |
 |   underflows or overflows.                                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t shl(int16_t var1, int16_t var2)
{
  int16_t var_out;

  if (var2 < 0) {
    var_out = shr(var1, -var2);
  } else {
    var_out = var1 << var2;
    if (var_out >> var2 != var1) {
      var_out = (var1 & WEBRTC_SPL_WORD16_MIN)
          ? WEBRTC_SPL_WORD16_MIN : WEBRTC_SPL_WORD16_MAX;
    }
  }

  return var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : shr                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
 |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
 |   -var2 with sign extension. Saturate the result in case of underflows or |
 |   overflows.                                                              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t shr(int16_t var1,int16_t var2)
{
  int16_t var_out;

  if (var2 < 0) {
    var_out = shl(var1,-var2);
  } else if (var2 >= 15) {
    var_out = (var1 < 0) ? (int16_t)(-1) : (int16_t)0;
  } else {
    var_out = (var1 < 0) ? ~((~var1) >> var2) : var1 >> var2;
  }

  return var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : mult                                                    |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the multiplication of var1 by var2 and gives a 16 bit result  |
 |    which is scaled i.e.:                                                  |
 |             mult(var1,var2) = shr((var1 times var2),15) and               |
 |             mult(-32768,-32768) = 32767.                                  |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t mult(int16_t var1, int16_t var2)
{
#if defined(WEBRTC_ARCH_ARM_V7)
  int32_t L_var_out;
  int32_t temp = WEBRTC_SPL_WORD16_MAX;
  int32_t ra = (int32_t)var1;
  int32_t rb = (int32_t)var2;

  __asm __volatile(
          "smulbb %0, %1, %2   \n\t"
          "mov %0, %0, ASR #15 \n\t"
          "cmp %0, %3          \n\t"
#if defined(ANDROID)
          "it ge               \n\t"
#endif
          "movge %0, %3        \n\t"
          : "=r"(L_var_out)
          : "r"(ra), "r"(rb), "r"(temp));

  return (int16_t)L_var_out;
#else
  int32_t L_var_out;

  L_var_out = ((int32_t)var1 * var2) >> 15;

  /* Saturate result (if necessary). */
  /* var1 * var2 > 0x00007fff is the only case */
  /* that saturation occurs. */

  if (L_var_out > (int32_t)0x00007fffL) {
    L_var_out = (int32_t)WEBRTC_SPL_WORD16_MAX;
  }

  return (int16_t)L_var_out;
#endif
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_mult                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   L_mult is the 32 bit result of the multiplication of var1 times var2    |
 |   with one shift left i.e.:                                               |
 |        L_mult(var1,var2) = shl((var1 times var2),1) and                   |
 |        L_mult(-32768,-32768) = 2147483647.                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_mult(int16_t var1,int16_t var2)
{
#if defined(WEBRTC_ARCH_ARM_V7)
  int32_t L_var_out;
  int32_t ra = (int32_t)var1;
  int32_t rb = (int32_t)var2;

  __asm __volatile(
          "smulbb %0, %1, %2 \n\t"
          "qadd %0, %0, %0   \n\t"
          : "=r"(L_var_out)
          : "r"(ra), "r"(rb));

  return L_var_out;
#else
  int32_t L_var_out;

  L_var_out = (int32_t)var1 * var2;

  if (L_var_out != (int32_t)0x40000000L) {
    L_var_out <<= 1; /* Multiply by 2 */
  } else {
    L_var_out = WEBRTC_SPL_WORD32_MAX;
  }

  return L_var_out;
#endif
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : negate                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate var1 with saturation, saturate in the case where input is -32768:|
 |                negate(var1) = WebRtcSpl_SubSatW16(0,var1).                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t negate(int16_t var1)
{
  return (var1 == WEBRTC_SPL_WORD16_MIN) ? WEBRTC_SPL_WORD16_MAX : -var1;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : extract_h                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 MSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |            32 bit long signed integer (int32_t ) whose value falls in the |
 |            range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                  |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t extract_h(int32_t L_var1)
{
   return (int16_t)(L_var1 >> 16);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : extract_l                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 LSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |            32 bit long signed integer (int32_t ) whose value falls in the |
 |            range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                  |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t extract_l(int32_t L_var1)
{
   return (int16_t) L_var1;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_mac                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation, return a 32 bit result:               |
 |        L_mac(L_var3,var1,var2) = L_add(L_var3,(L_mult(var1,var2)).        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_mac(int32_t L_var3, int16_t var1, int16_t var2)
{
#if defined(WEBRTC_ARCH_ARM_V7)
  int32_t L_var_out;
  int32_t ra = (int32_t)var1;
  int32_t rb = (int32_t)var2;
  int32_t rc = L_var3;

  __asm __volatile(
          "smulbb %0, %1, %2\n"
          "qdadd %0, %3, %0"
          : "=&r*i"(L_var_out)
          : "r"(ra),
            "r"(rb),
            "r"(rc));

  return L_var_out;
#else
  int32_t L_var_out;

  L_var_out = (int32_t) var1 * var2;
  if (L_var_out != (int32_t)0x40000000L) {
    L_var_out = (L_var_out << 1) + L_var3;

    /* Check if L_var_out and L_var_3 share the same sign */
    if ((L_var3 ^ L_var_out) > 0) {
      if ((L_var_out ^ L_var3) < 0) {
        L_var_out = (L_var3 < 0)
            ? WEBRTC_SPL_WORD32_MIN : WEBRTC_SPL_WORD32_MAX;
      }
    }
  } else {
    L_var_out = WEBRTC_SPL_WORD32_MAX;
  }

  return L_var_out;
#endif
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_msu                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation, return a 32 bit result:           |
 |        L_msu(L_var3,var1,var2) = L_sub(L_var3,(L_mult(var1,var2)).        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_msu(int32_t L_var3, int16_t var1, int16_t var2)
{
#if defined(WEBRTC_ARCH_ARM_V7)
  int32_t L_var_out;
  int32_t ra = (int32_t)var1;
  int32_t rb = (int32_t)var2;
  int32_t rc = L_var3;

  __asm __volatile(
          "smulbb %0, %1, %2\n"
          "qdsub %0, %3, %0"
          : "=&r*i"(L_var_out)
          : "r"(ra),
            "r"(rb),
            "r"(rc));

  return L_var_out;
#else
  return WebRtcSpl_SubSatW32(L_var3, L_mult(var1, var2));
#endif
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_negate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
 |   where input is -2147483648 (0x8000 0000).                               |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_negate(int32_t L_var1)
{
  return (L_var1 == WEBRTC_SPL_WORD32_MIN) ? WEBRTC_SPL_WORD32_MAX : -L_var1;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : mult_r                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as mult with rounding, i.e.:                                       |
 |     mult_r(var1,var2) = shr(((var1*var2) + 16384),15) and                 |
 |     mult_r(-32768,-32768) = 32767.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t mult_r(int16_t var1, int16_t var2)
{
  int16_t var_out;
  int32_t L_produit_arr;

  L_produit_arr = (int32_t)var1 * var2;        /* product */
  L_produit_arr += (int32_t)0x00004000L;       /* round */
  L_produit_arr &= (int32_t)0xffff8000L;
  L_produit_arr >>= 15;                        /* shift */

  if (L_produit_arr & (int32_t)0x00010000L) { /* sign extend when necessary */
    L_produit_arr |= (int32_t)0xffff0000L;
  }

  var_out = WebRtcSpl_SatW32ToW16(L_produit_arr);
  return var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_shl                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
 |   fill the var2 LSB of the result. If var2 is negative, L_var1 right by   |
 |   -var2 arithmetically shift with sign extension. Saturate the result in  |
 |   case of underflows or overflows.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_shl(int32_t L_var1, int16_t var2)
{
  int32_t tmp;

  if (var2 < 0) {
    tmp = (L_var1 < 0 ? ~(( ~L_var1) >> (-var2)) : L_var1 >> (-var2));
  } else {
    tmp = L_var1 << var2;
    if (tmp >> var2 != L_var1) {
      tmp = (L_var1 & WEBRTC_SPL_WORD32_MIN)
          ? WEBRTC_SPL_WORD32_MIN : WEBRTC_SPL_WORD32_MAX;
    }
  }

  return tmp;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_shr                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
 |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
 |   by -var2 and zero fill the var2 LSB of the result. Saturate the result  |
 |   in case of underflows or overflows.                                     |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.               |
 |___________________________________________________________________________|
*/
static __inline int32_t L_shr(int32_t L_var1, int16_t var2)
{
  int32_t tmp;

  if (var2 < 0) {
    tmp = L_shl(L_var1, -var2);
  } else {
    tmp = (L_var1 < 0) ? ~(( ~L_var1) >> var2) : L_var1 >> var2;
  }

  return tmp;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : shr_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as shr(var1,var2) but with rounding. Saturate the result in case of|
 |   underflows or overflows :                                               |
 |    If var2 is greater than zero :                                         |
 |       shr_r(var1,var2) = shr(add(var1,2**(var2-1)),var2)                  |
 |    If var2 is less than zero :                                            |
 |       shr_r(var1,var2) = shr(var1,var2).                                  |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t shr_r(int16_t var1, int16_t var2)
{
  int16_t var_out;

  if (var2 > 15) {
    var_out = 0;
  } else {
    var_out = shr(var1, var2);
    if (var2 > 0) {
      if ((var1 & ((int16_t)1 << (var2-1))) != 0) {
        var_out++;
      }
    }
  }

  return var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_abs                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of L_var1; Saturate in case where the input is          |
 |                                                               -214783648  |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= var1 <= 0x7fff ffff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x0000 0000 <= var_out <= 0x7fff ffff.                 |
 |___________________________________________________________________________|
*/
static __inline int32_t L_abs(int32_t L_var1)
{
 int32_t L_var_out;

  if (L_var1 == WEBRTC_SPL_WORD32_MIN) {
    L_var_out = WEBRTC_SPL_WORD32_MAX;
  } else {
    L_var_out = WEBRTC_SPL_ABS_W32(L_var1);
  }

  return L_var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : msu_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation. Round the LS 16 bits of the res-  |
 |   ult into the MS 16 bits with saturation and shift the result right by   |
 |   16. Return a 16 bit result.                                             |
 |            msu_r(L_var3,var1,var2) = round(L_msu(Lvar3,var1,var2))        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3  32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                  |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.               |
 |___________________________________________________________________________|
*/
static __inline int16_t msu_r(int32_t L_var3, int16_t var1, int16_t var2)
{
  int16_t var_out;

  L_var3 = L_msu(L_var3,var1,var2);
  L_var3 = WebRtcSpl_AddSatW32(L_var3, (int32_t)0x00008000L);
  var_out = extract_h(L_var3);

  return var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_deposit_h                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
 |   16 LS bits of the output are zeroed.                                    |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= var_out <= 0x7fff 0000.                 |
 |___________________________________________________________________________|
*/
static __inline int32_t L_deposit_h(int16_t var1)
{
  return (int32_t)var1 << 16;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_deposit_l                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
 |   16 MS bits of the output are sign extended.                             |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int32_t L_deposit_l(int16_t var1)
{
  return (int32_t)var1;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_shr_r                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as L_shr(L_var1,var2)but with rounding. Saturate the result in case|
 |   of underflows or overflows :                                            |
 |    If var2 is greater than zero :                                         |
 |       L_shr_r(var1,var2) = L_shr(L_add(L_var1,2**(var2-1)),var2)          |
 |    If var2 is less than zero :                                            |
 |       L_shr_r(var1,var2) = L_shr(L_var1,var2).                            |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= var1 <= 0x7fff ffff.                    |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var1 <= 0x0000 7fff.                    |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |            32 bit long signed integer (int32_t) whose value falls in the  |
 |            range : 0x8000 0000 <= var_out <= 0x7fff ffff.                 |
 |___________________________________________________________________________|
*/
static __inline int32_t L_shr_r(int32_t L_var1,int16_t var2)
{
  int32_t L_var_out;

  if (var2 > 31) {
    L_var_out = 0;
  } else {
    L_var_out = L_shr(L_var1,var2);
    if (var2 > 0) {
      if ((L_var1 & ((int32_t)1 << (var2-1))) != 0) {
        L_var_out++;
      }
    }
  }

  return L_var_out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : div_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces a result which is the fractional  integer division of var1 by  |
 |   var2; var1 and var2 must be positive and var2 must be greater or equal  |
 |   to var1; the result is positive (leading bit equal to 0) and truncated  |
 |   to 16 bits.                                                             |
 |   If var1 = var2 then div(var1,var2) = 32767.                             |
 |                                                                           |
 |   Complexity weight : 18                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0x0000 0000 <= var1 <= var2 and var2 != 0.             |
 |                                                                           |
 |    var2                                                                   |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : var1 <= var2 <= 0x0000 7fff and var2 != 0.             |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0x0000 0000 <= var_out <= 0x0000 7fff.                 |
 |            It's a Q15 value (point between b15 and b14).                  |
 |___________________________________________________________________________|
*/
static __inline int16_t div_s(int16_t var1, int16_t var2)
{
  int16_t var_out = 0, iteration;
  int32_t L_num, L_denom;
  int32_t L_denom_by_2, L_denom_by_4;

  if ((var1 > var2) || (var1 < 0)) {
    return 0; /* used to exit(0); */
  }

  if (var1) {
    if (var1 == var2)
      var_out = WEBRTC_SPL_WORD16_MAX;
    else {
      L_num = (int32_t) var1;
      L_denom = (int32_t) var2;
      L_denom_by_2 = (L_denom << 1);
      L_denom_by_4 = (L_denom << 2);

      for (iteration = 5; iteration > 0; iteration--) {
        var_out <<= 3;
        L_num <<= 3;
        if (L_num >= L_denom_by_4) {
          L_num -= L_denom_by_4;
          var_out |= 4;
        }
        if (L_num >= L_denom_by_2) {
          L_num -= L_denom_by_2;
          var_out |=  2;
        }
        if (L_num >= (L_denom)) {
          L_num -= (L_denom);
          var_out |=  1;
        }
      }
    }
  }

  return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_round                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into its MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |               L_round(L_var1) = extract_h(L_add(L_var1,32768))            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |            32 bit long signed integer (int32_t ) whose value falls in the |
 |            range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                  |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |            16 bit short signed integer (int16_t) whose value falls in the |
 |            range : 0xffff 8000 <= var_out <= 0x0000 7fff.                 |
 |___________________________________________________________________________|
*/
static __inline int16_t L_round(int32_t ra)
{
#if defined(WEBRTC_ARCH_ARM_V7)
   int32_t var_out;
   int32_t rb = 0x00008000L;

   __asm __volatile(
            "qadd %0, %1, %2  \n\t"
            "mov %0, %0, ASR #16 \n\t"
            : "=r"(var_out)
            : "r"(ra), "r"(rb));

  return var_out;
#else
  return (int16_t)(WebRtcSpl_AddSatW32(ra, 0x00008000L) >> 16);
#endif
}

#endif /* __BASIC_OP_H__ */

