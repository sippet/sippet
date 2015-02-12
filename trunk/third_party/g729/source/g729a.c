/****************************************************************************
** This file is an amalgamation of many separate C source files from G.729
** source code.  By combining all the individual C code files into this 
** single large file, the entire code can be compiled as a single translation
** unit.  This allows many compilers to do optimizations that would not be
** possible if the files were compiled separately.
**
** This file is all you need to compile G.729.
*/
/****************************************************************************
Portions of this file are derived from the following ITU standard:
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
*****************************************************************************/

/************** Begin file typedef.h ****************************************/
#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

#include <stdint.h>

typedef int16_t  Word16;
typedef int32_t  Word32;
typedef int      Flag;
typedef uint8_t  UWord8;
typedef uint16_t UWord16;
typedef uint32_t UWord32;
typedef int64_t  Word64;

#endif
/************** End of file typedef.h ***************************************/

/************** Begin file basic_op.h ***************************************/
/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000


/*___________________________________________________________________________
 |                                                                           |
 |   Operators prototypes                                                    |
 |___________________________________________________________________________|
*/

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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 <= var1 <= var2.                               |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : var1 <= var2 <= 0x7fff.                               |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 <= var_out <= 0x7fff.                          |
 |             It's a Q15 value (point between b15 and b14).                 |
 |___________________________________________________________________________|
*/
static Word16 div_s(Word16 var1, Word16 var2)
{
  Word16 var_out = 0, iteration;
  Word32 L_num, L_denom;
  Word32 L_denom_by_2, L_denom_by_4;

  if ((var1 > var2) || (var1 < 0)) {
    return 0; /* used to exit(0); */
  }

  if (var1) {
    if (var1 == var2)
      var_out = MAX_16;
    else {
      L_num = (Word32) var1;
      L_denom = (Word32) var2;
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
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
#define extract_h(L_var1) ((Word16)((L_var1) >> 16))

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
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
#define extract_l(L_var1) ((Word16)(L_var1))

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : negate                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate var1 with saturation, saturate in the case where input is -32768:|
 |                negate(var1) = sub(0,var1).                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
#define negate(var1) (((var1) == MIN_16) ? MAX_16 : -(var1))

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
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
#define L_negate(L_var1) (((L_var1) == MIN_32) ? MAX_32 : -(L_var1))

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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff 0000.                |
 |___________________________________________________________________________|
*/
#define L_deposit_h(var1) ((Word32)(var1) << 16)

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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
#define L_deposit_l(var1) ((Word32)(var1))

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : norm_s                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = shl(var1,norm_s(var1)).                    |
 |                                                                           |
 |   Complexity weight : 15                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
 |___________________________________________________________________________|
*/
#define norm_s(var1) norm_l(((Word32)(var1)) << 16)

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : norm_l                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 32 bit varia- |
 |   ble l_var1 for positive values on the interval with minimum of          |
 |   1073741824 and maximum of 2147483647, and for negative values on the in-|
 |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
 |   to normalize the result, the following operation must be done :         |
 |                   norm_L_var1 = L_shl(L_var1,norm_l(L_var1)).             |
 |                                                                           |
 |   Complexity weight : 30                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 001f.                |
 |___________________________________________________________________________|
*/
static Word16 norm_l(register Word32 ra)
{
  Word32 out = 0;
#if defined(ARCH_ARM)
  if (ra)
  {
    ra ^= (ra << 1);
    __asm__("clz %0, %1  \n\t"
            : "=r"(out)
            : "r"(ra));
  }
#else
  if (ra)
  {
    if (ra == (Word32)0xffffffffL)
    {
      out = 31;
    }
    else
    {
      if (ra < 0)
      {
        ra = ~ra;
      }
      for(out = 0;ra < (Word32)0x40000000L;out++)
      {
        ra <<= 1;
      }
    }
  }
#endif
  return (out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_sub                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with   |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_sub(register Word32 ra, register Word32 rb)
{
  Word32 out;
#if defined(ARCH_ARM)
  __asm__("qsub %0, %1, %2"
          : "=r"(out)
          : "r"(ra), "r"(rb));
#else
  out = ra - rb;
  if ((ra ^ rb) < 0)
  {
    if ((out ^ ra) & MIN_32)
    {
      out = (ra < 0L) ? MIN_32 : MAX_32;
    }
  }
#endif
  return (out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_add                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits addition of the two 32 bits variables (L_var1+L_var2) with      |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_add(register Word32 ra, register Word32 rb)
{
  Word32 out;
#if defined(ARCH_ARM)
  __asm__("qadd %0, %1, %2"
          : "=r"(out)
          : "r"(ra), "r"(rb));
#else
  out = ra + rb;
  if ((ra ^ rb) >= 0)
  {
      if ((out ^ ra) < 0)
      {
          out = (ra < 0) ? MIN_32 : MAX_32;
      }
  }
#endif
  return (out);
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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 mult(/*register Word32 ra */Word16 var1,
                          /*register Word32 rb */Word16 var2)
{
  Word32 product;
#if defined(ARCH_ARM)
  Word32 temp = MAX_16;
  register Word32 ra = var1;
  register Word32 rb = var2;

  __asm__("smulbb %0, %1, %2   \n\t"
          "mov %0, %0, ASR #15 \n\t"
          "cmp %0, %3          \n\t"
          "movge %0, %3        \n\t"
          : "=r"(product)
          : "r"(ra), "r"(rb), "r"(temp));

#else
  product = ((Word32) var1 * var2) >> 15;

  /* Saturate result (if necessary). */
  /* var1 * var2 >0x00007fff is the only case */
  /* that saturation occurs. */

  if (product > 0x00007fffL)
  {
    product = (Word32) MAX_16;
  }
#endif
  return ((Word16) product);
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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_mult(/*register Word32 ra /*/Word16 var1,
                            /*register Word32 rb /*/Word16 var2)
{
#if defined(ARCH_ARM)
  register Word32 ra = var1;
  register Word32 rb = var2;
  Word32 out;

  __asm__("smulbb %0, %1, %2 \n\t"
          "qadd %0, %0, %0   \n\t"
          : "=r"(out)
          : "r"(ra), "r"(rb));

  return(out);
#else
  register Word32 L_product;
  L_product = (Word32) var1 * var2;
  if (L_product != (Word32) 0x40000000L)
  {
    L_product <<= 1;          /* Multiply by 2 */
  }
  else
  {
    L_product = MAX_32;
  }
  return (L_product);
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
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_msu(register Word32 ra,
                           register Word32 rb /*Word16 var1*/,
                           register Word32 rc /*Word16 var2*/)
{
  Word32 out;
#if defined(ARCH_ARM)
  Word32 tmp = 0;
  __asm__("smulbb %0, %2, %3 \n\t"
          "qdsub %1, %4, %5  \n\t"
          : "=r"(tmp), "=r"(out)
          : "r"(rb), "r"(rc), "r"(ra), "0"(tmp));
#else
  out = L_mult(rb, rb);
  out = L_sub(ra, out);
#endif
  return (out);
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
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_mac(register Word32 ra,
                           register Word32 rb /*Word16 var1*/,
                           register Word32 rc /*Word16 var2*/)
{
  Word32 out;
  Word32 tmp = 0;
#if defined(ARCH_ARM)
  __asm__("smulbb %0, %3, %4   \n\t"
          "qdadd %1, %2, %5    \n\t"
          : "=r"(tmp), "=r"(out)
          : "r"(ra), "r"(rb), "r"(rc), "0"(tmp));
#else
  tmp = (Word32) rb * rc;
  if (tmp != (Word32) 0x40000000L)
  {
    out = (tmp << 1) + ra;
    /* Check if out and L_var_3 share the same sign */
    if ((ra ^ tmp) > 0)
    {
      if ((out ^ ra) < 0)
      {
        out = (ra < 0) ? MIN_32 : MAX_32;
      }
    }
  }
  else
  {
    out = MAX_32;
  }
#endif
  return (out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : sature                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Limit the 32 bit input to the range of a 16 bit word.                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 sature(Word32 L_var1)
{
  Word16 var_out;
  if (L_var1 > 0x00007fffL)
    var_out = MAX_16;
  else if (L_var1 < (Word32)0xffff8000L)
    var_out = MIN_16;
  else
    var_out = extract_l(L_var1);
  return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : add                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the addition (var1+var2) with overflow control and saturation;|
 |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
 |    when underflow occurs.                                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 add(Word16 var1, Word16 var2)
{
  Word32 out;
#if defined(ARCH_ARM)
  register Word32 ra = var1;
  register Word32 rb = var2;
  __asm__("qadd16 %0, %1, %2"
          : "=r"(out)
          : "r"(ra), "r"(rb));
#else
   out = (Word32) var1 + var2;
   out = sature(out);
#endif
  return (Word16)out;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : sub                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the subtraction (var1+var2) with overflow control and satu-   |
 |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
 |    -32768 when underflow occurs.                                          |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 sub(Word16 var1, Word16 var2)
{
  Word32 out;
#if defined(ARCH_ARM)
  register Word32 ra = var1;
  register Word32 rb = var2;
  __asm__("qsub16 %0, %1, %2"
          : "=r"(out)
          : "r"(ra), "r"(rb));
#else
   out = (Word32) var1 - var2;
   out = sature(out);
#endif
  return (Word16)out;
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
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x0000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/
static Word32 L_abs(Word32 L_var1)
{
  if (L_var1 == MIN_32)
    return MAX_32;
  else
    return (L_var1 < 0 ? -L_var1 : L_var1);
}

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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 abs_s(Word16 var1)
{
 if (var1 == MIN_16 )
   return MAX_16;
 else
   return (var1 < 0 ? -var1 : var1);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g_round                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into its MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |               g_round(L_var1) = extract_h(L_add(L_var1,32768))              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 g_round(register Word32 ra)
{
   Word32 out;
#if defined(ARCH_ARM)
   register Word32 rb = 0x00008000;
   __asm__ ("qadd %0, %1, %2  \n\t"
            "mov %0, %0, ASR #16 \n\t"
            : "=r"(out)
            : "r"(ra), "r"(rb));
#else
  out = L_add(ra, 0x00008000L);
  out = (Word16)(out >> 16);
#endif
   return (Word16)out;
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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 shl(Word16 var1,Word16 var2)
{
  Word16 tmp;
  if (var2 < 0)
    //return shr(var1, -var2);
    return (var1 < 0 ? ~(( ~var1) >> (-var2)) : var1 >> (-var2));

  tmp = var1 << var2;
  if (tmp >> var2 != var1)
    return (var1 & MIN_16 ? MIN_16 : MAX_16);

  return (tmp);
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
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
static Word16 shr(Word16 var1,Word16 var2)
{
  if (var2 < 0)
    return shl(var1, -var2);

  return (var1 < 0 ? ~(( ~var1) >> var2) : var1 >> var2);
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
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_shl(Word32 var1,Word32 var2)
{
  Word32 tmp;
  if (var2 < 0)
    //return L-shr(var1, -var2);
    return (var1 < 0 ? ~(( ~var1) >> (-var2)) : var1 >> (-var2));

  tmp = var1 << var2;
  if (tmp >> var2 != var1)
    return (var1 & MIN_32 ? MIN_32 : MAX_32);

  return (tmp);
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
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
static Word32 L_shr(Word32 var1,Word32 var2)
{
  if (var2 < 0)
    return L_shl(var1, -var2);

  return (var1 < 0 ? ~(( ~var1) >> var2) : var1 >> var2);
}

/************** End of file basic_op.h **************************************/

/************** Begin of file oper_32b.h ************************************/
/*___________________________________________________________________________
 |                                                                           |
 |  Function L_Comp()                                                        |
 |                                                                           |
 |  Compose from two 16 bit DPF a 32 bit integer.                            |
 |                                                                           |
 |     L_32 = hi<<16 + lo<<1                                                 |
 |                                                                           |
 |  Arguments:                                                               |
 |                                                                           |
 |   hi        msb                                                           |
 |   lo        lsf (with sign)                                               |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_32 <= 0x7fff fff0.                   |
 |                                                                           |
 |___________________________________________________________________________|
*/
#define L_Comp(hi, lo)  (((Word32)(hi) << 16) + ((Word32)(lo) << 1))

/*___________________________________________________________________________
 |                                                                           |
 |  Function L_Extract()                                                     |
 |                                                                           |
 |  Extract from a 32 bit integer two 16 bit DPF.                            |
 |                                                                           |
 |  Arguments:                                                               |
 |                                                                           |
 |   L_32      : 32 bit integer.                                             |
 |               0x8000 0000 <= L_32 <= 0x7fff ffff.                         |
 |   hi        : b16 to b31 of L_32                                          |
 |   lo        : (L_32 - hi<<16)>>1                                          |
 |___________________________________________________________________________|
*/
static void L_Extract(Word32 L_32, Word16 *hi, Word16 *lo)
{
  *hi = (Word16) (L_32 >> 16);
  *lo = (Word16)((L_32 >> 1) - (*hi << 15));
}

/*___________________________________________________________________________
 | Function Mpy_32_16()                                                      |
 |                                                                           |
 |   Multiply a 16 bit integer by a 32 bit (DPF). The result is divided      |
 |   by 2**15                                                                |
 |                                                                           |
 |   This operation can also be viewed as the multiplication of a Q31        |
 |   number by a Q15 number, the result is in Q31.                           |
 |                                                                           |
 |   L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1                                |
 |                                                                           |
 | Arguments:                                                                |
 |                                                                           |
 |  hi          hi part of 32 bit number.                                    |
 |  lo          lo part of 32 bit number.                                    |
 |  n           16 bit number.                                               |
 |                                                                           |
 |___________________________________________________________________________|
*/
static Word32 Mpy_32_16(Word16 shi, Word16 slo, Word16 n)
{
#if defined(ARCH_ARM)
  register Word32 a = L_Comp(shi, slo);
  register Word32 b = n;
  int lo, hi;
  __asm__("smull %0, %1, %2, %3     \n\t"
          "mov   %0, %0,     LSR #15 \n\t"
          "add   %1, %0, %1, LSL #17 \n\t"
          : "=&r"(lo), "=&r"(hi)
          : "r"(b), "r"(a));
  return hi;
#else
  Word32 product;
  Word32 out;
  Word32 result;
  product = (Word32) shi * n;
  if (product != (Word32) 0x40000000L)
  {
    product <<= 1;
  }
  else
  {
    product = MAX_32;
  }
  result = ((Word32)slo * n) >> 15;
  out  =  product + (result << 1);
  if ((product ^ result) > 0)
  {
    if ((out ^ product) < 0)
    {
      out = (product < 0) ? MIN_32 : MAX_32;
    }
  }
  return (out);
#endif
}

/*___________________________________________________________________________
 | Function Mpy_32()                                                         |
 |                                                                           |
 |   Multiply two 32 bit integers (DPF). The result is divided by 2**31      |
 |                                                                           |
 |   L_32 = (hi1*hi2)<<1 + ( (hi1*lo2)>>15 + (lo1*hi2)>>15 )<<1              |
 |                                                                           |
 |   This operation can also be viewed as the multiplication of two Q31      |
 |   number and the result is also in Q31.                                   |
 |                                                                           |
 | Arguments:                                                                |
 |                                                                           |
 |  hi1         hi part of first number                                      |
 |  lo1         lo part of first number                                      |
 |  hi2         hi part of second number                                     |
 |  lo2         lo part of second number                                     |
 |                                                                           |
 |___________________________________________________________________________|
*/
static Word32 Mpy_32(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2)
{
#if defined(ARCH_ARM)
  register Word32 ra = hi1 << 16 | lo1;
  register Word32 rb = hi2 << 16 | lo2;
  Word32 out, outl;

  __asm__("smultt %0, %1, %2 \n\t"
          "qadd %0, %0, %0   \n\t"
          : "=r"(out)
          : "r"(ra), "r"(rb));
  __asm__("smultb %0, %2, %3   \n\t"
          "mov %0, %0, ASR #15 \n\t"
          "qdadd %1, %4, %0"
          : "=r"(outl), "=r"(out)
          : "r"(ra), "r"(rb), "1"(out));
  __asm__("smulbt %0, %2, %3   \n\t"
          "mov %0, %0, ASR #15 \n\t"
          "qdadd %1, %4, %0"
          : "=r"(outl), "=r"(out)
          : "r"(ra), "r"(rb), "1"(out));

  return out;
#else
  Word32 product;
  Word32 out;
  Word32 product32;
  product = (Word32) hi1 * hi2;
  if (product != (Word32) 0x40000000L)
  {
    product <<= 1;
  }
  else
  {
    product = MAX_32;
  }
  product32 = ((Word32) hi1 * lo2) >> 15;
  out = product + (product32 << 1);
  if ((product ^ product32) > 0)
  {
    if ((out ^ product) < 0)
    {
      out = (product < 0) ? MIN_32 : MAX_32;
    }
  }
  product = out;
  product32 = ((Word32) lo1 * hi2) >> 15;
  out = product + (product32 << 1);
  if ((product ^ product32) > 0)
  {
    if ((out ^ product) < 0)
    {
      out = (product < 0) ? MIN_32 : MAX_32;
    }
  }
  return (out);
#endif
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : Div_32                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |             Fractional integer division of two 32 bit numbers.            |
 |             L_num / L_denom.                                              |
 |             L_num and L_denom must be positive and L_num < L_denom.       |
 |             L_denom = denom_hi<<16 + denom_lo<<1                          |
 |             denom_hi is a normalize number.                               |
 |             The result is in Q30.                                         |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_num                                                                  |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x0000 0000 < L_num < L_denom                         |
 |                                                                           |
 |    L_denom = denom_hi<<16 + denom_lo<<1      (DPF)                        |
 |                                                                           |
 |       denom_hi                                                            |
 |             16 bit positive normalized integer whose value falls in the   |
 |             range : 0x4000 < hi < 0x7fff                                  |
 |       denom_lo                                                            |
 |             16 bit positive integer whose value falls in the              |
 |             range : 0 < lo < 0x7fff                                       |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_div                                                                  |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x0000 0000 <= L_div <= 0x7fff ffff.                  |
 |             It's a Q31 value                                              |
 |                                                                           |
 |  Algorithm:                                                               |
 |                                                                           |
 |  - find = 1/L_denom.                                                      |
 |      First approximation: approx = 1 / denom_hi                           |
 |      1/L_denom = approx * (2.0 - L_denom * approx )                       |
 |                                                                           |
 |  -  result = L_num * (1/L_denom)                                          |
 |___________________________________________________________________________|
*/
static Word32 Div_32(Word32 L_num, Word16 denom_hi, Word16 denom_lo)
{
  Word16 approx, hi, lo, n_hi, n_lo;
  Word32 L_32;


  /* First approximation: 1 / L_denom = 1/denom_hi */

  approx = div_s( (Word16)0x3fff, denom_hi);    /* result in Q14 */
                                                /* Note: 3fff = 0.5 in Q15 */

  /* 1/L_denom = approx * (2.0 - L_denom * approx) */

  L_32 = Mpy_32_16(denom_hi, denom_lo, approx); /* result in Q30 */

  L_32 = L_sub( MAX_32, L_32);      /* result in Q30 */

  hi = (Word16)(L_32 >> 16);
  lo = (L_32 >> 1) - (hi << 15);

  L_32 = Mpy_32_16(hi, lo, approx);             /* = 1/L_denom in Q29 */

  /* L_num * (1/L_denom) */
  hi = (Word16)(L_32 >> 16);
  lo = (L_32 >> 1) - (hi << 15);
  n_hi = (Word16)(L_num >> 16);
  n_lo = (L_num >> 1) - (n_hi << 15);
  L_32 = Mpy_32(n_hi, n_lo, hi, lo);            /* result in Q29   */
  L_32 = L_shl(L_32, 2);                        /* From Q29 to Q31 */

  return( L_32 );
}
/************** End of file oper_32b.h **************************************/

/************** Begin of file ld8a.h ****************************************/
#ifndef __LD8A_H__
#define __LD8A_H__

#include <stddef.h>
#include <string.h>

/* Uncomment the next line, to compile codec in verification mode */
//#define CONTROL_OPT 1


/*---------------------------------------------------------------*
 * LD8A.H                                                        *
 * ~~~~~~                                                        *
 * Function prototypes and constants use for G.729A 8kb/s coder. *
 *                                                               *
 *---------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 *       Codec constant parameters (coder, decoder, and postfilter)         *
 *--------------------------------------------------------------------------*/

#define  L_TOTAL      240     /* Total size of speech buffer.               */
#define  L_WINDOW     240     /* Window size in LP analysis.                */
#define  L_NEXT       40      /* Lookahead in LP analysis.                  */
#define  L_FRAME      80      /* Frame size.                                */
#define  L_SUBFR      40      /* Subframe size.                             */
#define  M            10      /* Order of LP filter.                        */
#define  MP1          (M+1)   /* Order of LP filter + 1                     */
#define  PIT_MIN      20      /* Minimum pitch lag.                         */
#define  PIT_MAX      143     /* Maximum pitch lag.                         */
#define  L_INTERPOL   (10+1)  /* Length of filter for interpolation.        */
#define  GAMMA1       24576   /* Bandwitdh factor = 0.75   in Q15           */

#define  PRM_SIZE     11      /* Size of vector of analysis parameters.     */
#define  SERIAL_SIZE  (80+2)  /* bfi+ number of speech bits                 */

#define SHARPMAX  13017   /* Maximum value of pitch sharpening     0.8  Q14 */
#define SHARPMIN  3277    /* Minimum value of pitch sharpening     0.2  Q14 */


/*-------------------------------*
 * Mathematic functions.         *
 *-------------------------------*/

static Word32 Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

static void Log2(
  Word32 L_x,       /* (i) Q0 : input value                                 */
  Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  Word16 *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

static Word32 Pow2(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  Word16 fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);

/*-------------------------------*
 * LPC analysis and filtering.   *
 *-------------------------------*/

static void Autocorr(
  Word16 x[],      /* (i)    : Input signal                      */
  Word16 m,        /* (i)    : LPC order                         */
  Word16 r_h[],    /* (o)    : Autocorrelations  (msb)           */
  Word16 r_l[]     /* (o)    : Autocorrelations  (lsb)           */
);

static void Lag_window(
  Word16 m,         /* (i)     : LPC order                        */
  Word16 r_h[],     /* (i/o)   : Autocorrelations  (msb)          */
  Word16 r_l[]      /* (i/o)   : Autocorrelations  (lsb)          */
);

static void Levinson(
  Word16 Rh[],      /* (i)     : Rh[m+1] Vector of autocorrelations (msb) */
  Word16 Rl[],      /* (i)     : Rl[m+1] Vector of autocorrelations (lsb) */
  Word16 A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)       */
  Word16 rc[]       /* (o) Q15 : rc[M]   Relection coefficients.          */
);

static void Az_lsp(
  Word16 a[],        /* (i) Q12 : predictor coefficients              */
  Word16 lsp[],      /* (o) Q15 : line spectral pairs                 */
  Word16 old_lsp[]   /* (i)     : old lsp[] (in case not found 10 roots) */
);

static void Lsp_Az(
  Word16 lsp[],    /* (i) Q15 : line spectral frequencies            */
  Word16 a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
);

static void Int_qlpc(
 Word16 lsp_old[], /* input : LSP vector of past frame              */
 Word16 lsp_new[], /* input : LSP vector of present frame           */
 Word16 Az[]       /* output: interpolated Az() for the 2 subframes */
);

static void Weight_Az(
  Word16 a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
  Word16 gamma,    /* (i) Q15 : Spectral expansion factor.           */
  Word16 m,        /* (i)     : LPC order.                           */
  Word16 ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
);

static void Residu(
  Word16 a[],    /* (i) Q12 : prediction coefficients                     */
  Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed (m=10)  */
  Word16 y[],    /* (o)     : residual signal                             */
  Word16 lg      /* (i)     : size of filtering                           */
);

static void Syn_filt(
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
  Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
);

static Flag Syn_filt_overflow(
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[]    /* (i)     : memory associated with this filtering.   */
);

/*--------------------------------------------------------------------------*
 *       LTP constant parameters                                            *
 *--------------------------------------------------------------------------*/

#define UP_SAMP         3
#define L_INTER10       10
#define FIR_SIZE_SYN    (UP_SAMP*L_INTER10+1)

/*-----------------------*
 * Pitch functions.      *
 *-----------------------*/

static Word16 Pitch_ol_fast(  /* output: open loop pitch lag                        */
   Word16 signal[],    /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
   Word16   pit_max,   /* input : maximum pitch lag                          */
   Word16   L_frame    /* input : length of frame to compute pitch           */
);

static Word16 Pitch_fr3_fast(/* (o)     : pitch period.                          */
  Word16 exc[],       /* (i)     : excitation buffer                      */
  Word16 xn[],        /* (i)     : target vector                          */
  Word16 h[],         /* (i) Q12 : impulse response of filters.           */
  Word16 L_subfr,     /* (i)     : Length of subframe                     */
  Word16 t0_min,      /* (i)     : minimum value in the searched range.   */
  Word16 t0_max,      /* (i)     : maximum value in the searched range.   */
  Word16 i_subfr,     /* (i)     : indicator for first subframe.          */
  Word16 *pit_frac    /* (o)     : chosen fraction.                       */
);

static Word16 G_pitch(      /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
  Word16 xn[],       /* (i)     : Pitch target.                            */
  Word16 y1[],       /* (i)     : Filtered adaptive codebook.              */
  Word16 g_coeff[],  /* (i)     : Correlations need for gain quantization. */
  Word16 L_subfr     /* (i)     : Length of subframe.                      */
);

static Word16 Enc_lag3(     /* output: Return index of encoding */
  Word16 T0,         /* input : Pitch delay              */
  Word16 T0_frac,    /* input : Fractional pitch delay   */
  Word16 *T0_min,    /* in/out: Minimum search delay     */
  Word16 *T0_max,    /* in/out: Maximum search delay     */
  Word16 pit_min,    /* input : Minimum pitch delay      */
  Word16 pit_max,    /* input : Maximum pitch delay      */
  Word16 pit_flag    /* input : Flag for 1st subframe    */
);

static void Dec_lag3(        /* output: return integer pitch lag       */
  Word16 index,       /* input : received pitch index           */
  Word16 pit_min,     /* input : minimum pitch lag              */
  Word16 pit_max,     /* input : maximum pitch lag              */
  Word16 i_subfr,     /* input : subframe flag                  */
  Word16 *T0,         /* output: integer part of pitch lag      */
  Word16 *T0_frac     /* output: fractional part of pitch lag   */
);

static Word16 Interpol_3(      /* (o)  : interpolated value  */
  Word16 *x,            /* (i)  : input vector        */
  Word16 frac           /* (i)  : fraction            */
);

static void Pred_lt_3(
  Word16   exc[],       /* in/out: excitation buffer */
  Word16   T0,          /* input : integer pitch lag */
  Word16   frac,        /* input : fraction of lag   */
  Word16   L_subfr      /* input : subframe size     */
);

static Word16 Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
   Word16 pitch_index   /* input : index for which parity to compute */
);

static Word16  Check_Parity_Pitch( /* output: 0 = no error, 1= error */
  Word16 pitch_index,       /* input : index of parameter     */
  Word16 parity             /* input : parity bit             */
);

static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i)     :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
);

/*-----------------------*
 * Innovative codebook.  *
 *-----------------------*/

#define DIM_RR  616 /* size of correlation matrix                            */
#define NB_POS  8   /* Number of positions for each pulse                    */
#define STEP    5   /* Step betweem position of the same pulse.              */
#define MSIZE   64  /* Size of vectors for cross-correlation between 2 pulses*/

/* The following constants are Q15 fractions.
   These fractions is used to keep maximum precision on "alp" sum */

#define _1_2    (Word16)(16384)
#define _1_4    (Word16)( 8192)
#define _1_8    (Word16)( 4096)
#define _1_16   (Word16)( 2048)

static Word16  ACELP_Code_A(    /* (o)     :index of pulses positions    */
  Word16 x[],            /* (i)     :Target vector                */
  Word16 h[],            /* (i) Q12 :Inpulse response of filters  */
  Word16 T0,             /* (i)     :Pitch lag                    */
  Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
  Word16 code[],         /* (o) Q13 :Innovative codebook          */
  Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
  Word16 *sign           /* (o)     :Signs of 4 pulses            */
);

static void Decod_ACELP(
  Word16 sign,      /* (i)     : signs of 4 pulses.                       */
  Word16 index,     /* (i)     : Positions of the 4 pulses.               */
  Word16 cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
);
/*--------------------------------------------------------------------------*
 *       LSP constant parameters                                            *
 *--------------------------------------------------------------------------*/

#define   NC            5      /*  NC = M/2 */
#define   MA_NP         4      /* MA prediction order for LSP */
#define   MODE          2      /* number of modes for MA prediction */
#define   NC0_B         7      /* number of first stage bits */
#define   NC1_B         5      /* number of second stage bits */
#define   NC0           (1<<NC0_B)
#define   NC1           (1<<NC1_B)

#define   L_LIMIT          40   /* Q13:0.005 */
#define   M_LIMIT       25681   /* Q13:3.135 */

#define   GAP1          10     /* Q13 */
#define   GAP2          5      /* Q13 */
#define   GAP3          321    /* Q13 */
#define GRID_POINTS     50

#define PI04      ((Word16)1029)        /* Q13  pi*0.04 */
#define PI92      ((Word16)23677)       /* Q13  pi*0.92 */
#define CONST10   ((Word16)10*(1<<11))  /* Q11  10.0 */
#define CONST12   ((Word16)19661)       /* Q14  1.2 */

/*-------------------------------*
 * LSP VQ functions.             *
 *-------------------------------*/

static void Lsf_lsp2(
  Word16 lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
  Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
  Word16 m         /* (i)     : LPC order                   */
);

static void Lsp_lsf2(
  Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
  Word16 lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
  Word16 m         /* (i)     : LPC order                   */
);

static void Lsp_get_quant(
  Word16 lspcb1[][M],      /* Q13 */
  Word16 lspcb2[][M],      /* Q13 */
  Word16 code0,
  Word16 code1,
  Word16 code2,
  Word16 fg[][M],            /* Q15 */
  Word16 freq_prev[][M],     /* Q13 */
  Word16 lspq[],                /* Q13 */
  Word16 fg_sum[]               /* Q15 */
);

static void Lsp_last_select(
  Word32 L_tdist[],     /* Q27 */
  Word16 *mode_index
);

static void Lsp_stability(
  Word16 buf[]     /* Q13 */
);

static void Lsp_prev_extract(
  Word16 lsp[M],                 /* Q13 */
  Word16 lsp_ele[M],             /* Q13 */
  Word16 fg[MA_NP][M],           /* Q15 */
  Word16 freq_prev[MA_NP][M],    /* Q13 */
  Word16 fg_sum_inv[M]           /* Q12 */
);

static void Lsp_prev_update(
  Word16 lsp_ele[M],             /* Q13 */
  Word16 freq_prev[MA_NP][M]     /* Q13 */
);

/*-------------------------------*
 * gain VQ constants.            *
 *-------------------------------*/

#define NCODE1_B  3                /* number of Codebook-bit */
#define NCODE2_B  4                /* number of Codebook-bit */
#define NCODE1    (1<<NCODE1_B)    /* Codebook 1 size */
#define NCODE2    (1<<NCODE2_B)    /* Codebook 2 size */
#define NCAN1     4                /* Pre-selecting order for #1 */
#define NCAN2     8                /* Pre-selecting order for #2 */
#define INV_COEF  -17103           /* Q19 */

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/

static Word16 Qua_gain(
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 g_coeff[], /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                    /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
  Word16 exp_coeff[],/* (i)    : Q-Format g_coeff[]                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod, /* (o) Q1  : Code gain.                                 */
  Word16 tameflag   /* (i)     : flag set to 1 if taming is needed          */
);

static void Dec_gain(
  Word16 index,     /* (i)     : Index of quantization.                     */
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 bfi,       /* (i)     : Bad frame indicator                        */
  Word16 *gain_pit, /* (o) Q14 : Pitch gain.                                */
  Word16 *gain_cod  /* (o) Q1  : Code gain.                                 */
);

static void Gain_predict(
  Word16 past_qua_en[],/* (i) Q10 :Past quantized energies                  */
  Word16 code[],    /* (i) Q13 : Innovative vector.                         */
  Word16 L_subfr,   /* (i)     : Subframe length.                           */
  Word16 *gcode0,   /* (o) Qxx : Predicted codebook gain                    */
  Word16 *exp_gcode0 /* (o)    : Q-Format(gcode0)                           */
);

static void Gain_update(
  Word16 past_qua_en[],/* (i) Q10 :Past quantized energies                  */
  Word32 L_gbk12    /* (i) Q13 : gbk1[indice1][1]+gbk2[indice2][1]          */
);

static void Gain_update_erasure(
  Word16 past_qua_en[]/* (i) Q10 :Past quantized energies                   */
);

static void Corr_xy2(
      Word16 xn[],           /* (i) Q0  :Target vector.                  */
      Word16 y1[],           /* (i) Q0  :Adaptive codebook.              */
      Word16 y2[],           /* (i) Q12 :Filtered innovative vector.     */
      Word16 g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2 */
      Word16 exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]         */
);

/*-----------------------*
 * Bitstream function    *
 *-----------------------*/

#if defined(CONTROL_OPT) && (CONTROL_OPT == 1)
static void  prm2bits_ld8k(Word16 prm[], Word16 bits[]);
static void  bits2prm_ld8k(Word16 bits[], Word16 prm[]);

#define BIT_0     (short)0x007f /* definition of zero-bit in bit-stream      */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream       */
#else
static void  prm2bits_ld8k(Word16 prm[], UWord8 *bits);
static void  bits2prm_ld8k(UWord8 *bits, Word16 prm[]);
#endif

#define SYNC_WORD (short)0x6b21 /* definition of frame erasure flag          */
#define SIZE_WORD (short)80     /* number of speech bits                     */


/*-----------------------------------*
 * Post-filter functions.            *
 *-----------------------------------*/

#define L_H 22     /* size of truncated impulse response of A(z/g1)/A(z/g2) */

#define GAMMAP      16384   /* 0.5               (Q15) */
#define INV_GAMMAP  21845   /* 1/(1+GAMMAP)      (Q15) */
#define GAMMAP_2    10923   /* GAMMAP/(1+GAMMAP) (Q15) */

#define  GAMMA2_PST 18022 /* Formant postfilt factor (numerator)   0.55 Q15 */
#define  GAMMA1_PST 22938 /* Formant postfilt factor (denominator) 0.70 Q15 */

#define  MU       26214   /* Factor for tilt compensation filter   0.8  Q15 */
#define  AGC_FAC  29491   /* Factor for automatic gain control     0.9  Q15 */
#define  AGC_FAC1 (Word16)(32767 - AGC_FAC)    /* 1-AGC_FAC in Q15          */

static void pit_pst_filt(
  Word16 *signal,      /* (i)     : input signal                        */
  Word16 *scal_sig,    /* (i)     : input signal (scaled, divided by 4) */
  Word16 t0_min,       /* (i)     : minimum value in the searched range */
  Word16 t0_max,       /* (i)     : maximum value in the searched range */
  Word16 L_subfr,      /* (i)     : size of filtering                   */
  Word16 *signal_pst   /* (o)     : harmonically postfiltered signal    */
);

static void preemphasis(
  Word16 *signal,  /* (i/o)   : input signal overwritten by the output */
  Word16 g,        /* (i) Q15 : preemphasis coefficient                */
  Word16 L         /* (i)     : size of filtering                      */
);

static void agc(
  Word16 *sig_in,   /* (i)     : postfilter input signal  */
  Word16 *sig_out,  /* (i/o)   : postfilter output signal */
  Word16 l_trm      /* (i)     : subframe size            */
);

/*--------------------------------------------------------------------------*
 * Constants and prototypes for taming procedure.                           *
 *--------------------------------------------------------------------------*/

#define GPCLIP      15564      /* Maximum pitch gain if taming is needed Q14*/
#define GPCLIP2     481        /* Maximum pitch gain if taming is needed Q9 */
#define GP0999      16383      /* Maximum pitch gain if taming is needed    */
#define L_THRESH_ERR 983040000L /* Error threshold taming 16384. * 60000.   */

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions.                                      *
 *--------------------------------------------------------------------------*/

#define Copy(x,y,L)    memcpy((y), (x), (L)*sizeof(Word16))
#define Set_zero(x, L) memset((x), 0, (L)*sizeof(Word16))

static Word16 Random(void);

#endif /* __LD8A_H__ */

/************** End of file ld8a.h ******************************************/

/************** Begin of file tab_ld8a.h ************************************/
/* This file contains all the tables used by the G.729A codec */

/* Hamming_cos window for LPC analysis.           */
/*   Create with function ham_cos(window,200,40)  */

static Word16 hamwindow[L_WINDOW] = {
  2621,  2623,  2629,  2638,  2651,  2668,  2689,  2713,  2741,  2772,
  2808,  2847,  2890,  2936,  2986,  3040,  3097,  3158,  3223,  3291,
  3363,  3438,  3517,  3599,  3685,  3774,  3867,  3963,  4063,  4166,
  4272,  4382,  4495,  4611,  4731,  4853,  4979,  5108,  5240,  5376,
  5514,  5655,  5800,  5947,  6097,  6250,  6406,  6565,  6726,  6890,
  7057,  7227,  7399,  7573,  7750,  7930,  8112,  8296,  8483,  8672,
  8863,  9057,  9252,  9450,  9650,  9852, 10055, 10261, 10468, 10677,
 10888, 11101, 11315, 11531, 11748, 11967, 12187, 12409, 12632, 12856,
 13082, 13308, 13536, 13764, 13994, 14225, 14456, 14688, 14921, 15155,
 15389, 15624, 15859, 16095, 16331, 16568, 16805, 17042, 17279, 17516,
 17754, 17991, 18228, 18465, 18702, 18939, 19175, 19411, 19647, 19882,
 20117, 20350, 20584, 20816, 21048, 21279, 21509, 21738, 21967, 22194,
 22420, 22644, 22868, 23090, 23311, 23531, 23749, 23965, 24181, 24394,
 24606, 24816, 25024, 25231, 25435, 25638, 25839, 26037, 26234, 26428,
 26621, 26811, 26999, 27184, 27368, 27548, 27727, 27903, 28076, 28247,
 28415, 28581, 28743, 28903, 29061, 29215, 29367, 29515, 29661, 29804,
 29944, 30081, 30214, 30345, 30472, 30597, 30718, 30836, 30950, 31062,
 31170, 31274, 31376, 31474, 31568, 31659, 31747, 31831, 31911, 31988,
 32062, 32132, 32198, 32261, 32320, 32376, 32428, 32476, 32521, 32561,
 32599, 32632, 32662, 32688, 32711, 32729, 32744, 32755, 32763, 32767,
 32767, 32741, 32665, 32537, 32359, 32129, 31850, 31521, 31143, 30716,
 30242, 29720, 29151, 28538, 27879, 27177, 26433, 25647, 24821, 23957,
 23055, 22117, 21145, 20139, 19102, 18036, 16941, 15820, 14674, 13505,
 12315, 11106,  9879,  8637,  7381,  6114,  4838,  3554,  2264,   971};


/*-----------------------------------------------------*
 | Table of lag_window for autocorrelation.            |
 | noise floor = 1.0001   = (0.9999  on r[1] ..r[10])  |
 | Bandwidth expansion = 60 Hz                         |
 |                                                     |
 | Special double precision format. See "oper_32b.c"   |
 |                                                     |
 | lag_wind[0] =  1.00000000    (not stored)           |
 | lag_wind[1] =  0.99879038                           |
 | lag_wind[2] =  0.99546897                           |
 | lag_wind[3] =  0.98995781                           |
 | lag_wind[4] =  0.98229337                           |
 | lag_wind[5] =  0.97252619                           |
 | lag_wind[6] =  0.96072036                           |
 | lag_wind[7] =  0.94695264                           |
 | lag_wind[8] =  0.93131179                           |
 | lag_wind[9] =  0.91389757                           |
 | lag_wind[10]=  0.89481968                           |
 -----------------------------------------------------*/

static Word16 lag_h[M] = {
    32728,
    32619,
    32438,
    32187,
    31867,
    31480,
    31029,
    30517,
    29946,
    29321};

static Word16 lag_l[M] = {
    11904,
    17280,
    30720,
    25856,
    24192,
    28992,
    24384,
     7360,
    19520,
    14784};


/*-----------------------------------------------------*
 | Tables for function Lsf_lsp() and Lsp_lsf()         |
  -----------------------------------------------------*/

/* table of cos(x) in Q15 */

static Word16 table[65] = {
  32767,  32729,  32610,  32413,  32138,  31786,  31357,  30853,
  30274,  29622,  28899,  28106,  27246,  26320,  25330,  24279,
  23170,  22006,  20788,  19520,  18205,  16846,  15447,  14010,
  12540,  11039,   9512,   7962,   6393,   4808,   3212,   1608,
      0,  -1608,  -3212,  -4808,  -6393,  -7962,  -9512, -11039,
 -12540, -14010, -15447, -16846, -18205, -19520, -20788, -22006,
 -23170, -24279, -25330, -26320, -27246, -28106, -28899, -29622,
 -30274, -30853, -31357, -31786, -32138, -32413, -32610, -32729,
 -32768L };

/* slope in Q12 used to compute y = acos(x) */

static Word16 slope[64] = {
 -26887,  -8812,  -5323,  -3813,  -2979,  -2444,  -2081,  -1811,
  -1608,  -1450,  -1322,  -1219,  -1132,  -1059,   -998,   -946,
   -901,   -861,   -827,   -797,   -772,   -750,   -730,   -713,
   -699,   -687,   -677,   -668,   -662,   -657,   -654,   -652,
   -652,   -654,   -657,   -662,   -668,   -677,   -687,   -699,
   -713,   -730,   -750,   -772,   -797,   -827,   -861,   -901,
   -946,   -998,  -1059,  -1132,  -1219,  -1322,  -1450,  -1608,
  -1811,  -2081,  -2444,  -2979,  -3813,  -5323,  -8812, -26887};


/*-----------------------------------------------------*
 | Tables for function Lsf_lsp() and Lsp_lsf()         |
  -----------------------------------------------------*/

/* table of cos(x) in Q15 */

static Word16 table2[64] = {
  32767,  32729,  32610,  32413,  32138,  31786,  31357,  30853,
  30274,  29622,  28899,  28106,  27246,  26320,  25330,  24279,
  23170,  22006,  20788,  19520,  18205,  16846,  15447,  14010,
  12540,  11039,   9512,   7962,   6393,   4808,   3212,   1608,
      0,  -1608,  -3212,  -4808,  -6393,  -7962,  -9512, -11039,
 -12540, -14010, -15447, -16846, -18205, -19520, -20788, -22006,
 -23170, -24279, -25330, -26320, -27246, -28106, -28899, -29622,
 -30274, -30853, -31357, -31786, -32138, -32413, -32610, -32729 };

/* slope in Q19 used to compute y = cos(x) */

static Word16 slope_cos[64] = {
   -632,  -1893,  -3150,  -4399,  -5638,  -6863,  -8072,  -9261,
 -10428, -11570, -12684, -13767, -14817, -15832, -16808, -17744,
 -18637, -19486, -20287, -21039, -21741, -22390, -22986, -23526,
 -24009, -24435, -24801, -25108, -25354, -25540, -25664, -25726,
 -25726, -25664, -25540, -25354, -25108, -24801, -24435, -24009,
 -23526, -22986, -22390, -21741, -21039, -20287, -19486, -18637,
 -17744, -16808, -15832, -14817, -13767, -12684, -11570, -10428,
  -9261,  -8072,  -6863,  -5638,  -4399,  -3150,  -1893,   -632 };

/* slope in Q12 used to compute y = acos(x) */

static Word16 slope_acos[64] = {
 -26887,  -8812,  -5323,  -3813,  -2979,  -2444,  -2081,  -1811,
  -1608,  -1450,  -1322,  -1219,  -1132,  -1059,   -998,   -946,
   -901,   -861,   -827,   -797,   -772,   -750,   -730,   -713,
   -699,   -687,   -677,   -668,   -662,   -657,   -654,   -652,
   -652,   -654,   -657,   -662,   -668,   -677,   -687,   -699,
   -713,   -730,   -750,   -772,   -797,   -827,   -861,   -901,
   -946,   -998,  -1059,  -1132,  -1219,  -1322,  -1450,  -1608,
  -1811,  -2081,  -2444,  -2979,  -3813,  -5323,  -8812, -26887};
/* lsp    code book   <../f7s55m1.v2> */

static Word16 lspcb1[NC0][M] = {        /* Q13 */
{ 1486,  2168,  3751,  9074, 12134, 13944, 17983, 19173, 21190, 21820},
{ 1730,  2640,  3450,  4870,  6126,  7876, 15644, 17817, 20294, 21902},
{ 1568,  2256,  3088,  4874, 11063, 13393, 18307, 19293, 21109, 21741},
{ 1733,  2512,  3357,  4708,  6977, 10296, 17024, 17956, 19145, 20350},
{ 1744,  2436,  3308,  8731, 10432, 12007, 15614, 16639, 21359, 21913},
{ 1786,  2369,  3372,  4521,  6795, 12963, 17674, 18988, 20855, 21640},
{ 1631,  2433,  3361,  6328, 10709, 12013, 13277, 13904, 19441, 21088},
{ 1489,  2364,  3291,  6250,  9227, 10403, 13843, 15278, 17721, 21451},
{ 1869,  2533,  3475,  4365,  9152, 14513, 15908, 17022, 20611, 21411},
{ 2070,  3025,  4333,  5854,  7805,  9231, 10597, 16047, 20109, 21834},
{ 1910,  2673,  3419,  4261, 11168, 15111, 16577, 17591, 19310, 20265},
{ 1141,  1815,  2624,  4623,  6495,  9588, 13968, 16428, 19351, 21286},
{ 2192,  3171,  4707,  5808, 10904, 12500, 14162, 15664, 21124, 21789},
{ 1286,  1907,  2548,  3453,  9574, 11964, 15978, 17344, 19691, 22495},
{ 1921,  2720,  4604,  6684, 11503, 12992, 14350, 15262, 16997, 20791},
{ 2052,  2759,  3897,  5246,  6638, 10267, 15834, 16814, 18149, 21675},
{ 1798,  2497,  5617, 11449, 13189, 14711, 17050, 18195, 20307, 21182},
{ 1009,  1647,  2889,  5709,  9541, 12354, 15231, 18494, 20966, 22033},
{ 3016,  3794,  5406,  7469, 12488, 13984, 15328, 16334, 19952, 20791},
{ 2203,  3040,  3796,  5442, 11987, 13512, 14931, 16370, 17856, 18803},
{ 2912,  4292,  7988,  9572, 11562, 13244, 14556, 16529, 20004, 21073},
{ 2861,  3607,  5923,  7034,  9234, 12054, 13729, 18056, 20262, 20974},
{ 3069,  4311,  5967,  7367, 11482, 12699, 14309, 16233, 18333, 19172},
{ 2434,  3661,  4866,  5798, 10383, 11722, 13049, 15668, 18862, 19831},
{ 2020,  2605,  3860,  9241, 13275, 14644, 16010, 17099, 19268, 20251},
{ 1877,  2809,  3590,  4707, 11056, 12441, 15622, 17168, 18761, 19907},
{ 2107,  2873,  3673,  5799, 13579, 14687, 15938, 17077, 18890, 19831},
{ 1612,  2284,  2944,  3572,  8219, 13959, 15924, 17239, 18592, 20117},
{ 2420,  3156,  6542, 10215, 12061, 13534, 15305, 16452, 18717, 19880},
{ 1667,  2612,  3534,  5237, 10513, 11696, 12940, 16798, 18058, 19378},
{ 2388,  3017,  4839,  9333, 11413, 12730, 15024, 16248, 17449, 18677},
{ 1875,  2786,  4231,  6320,  8694, 10149, 11785, 17013, 18608, 19960},
{  679,  1411,  4654,  8006, 11446, 13249, 15763, 18127, 20361, 21567},
{ 1838,  2596,  3578,  4608,  5650, 11274, 14355, 15886, 20579, 21754},
{ 1303,  1955,  2395,  3322, 12023, 13764, 15883, 18077, 20180, 21232},
{ 1438,  2102,  2663,  3462,  8328, 10362, 13763, 17248, 19732, 22344},
{  860,  1904,  6098,  7775,  9815, 12007, 14821, 16709, 19787, 21132},
{ 1673,  2723,  3704,  6125,  7668,  9447, 13683, 14443, 20538, 21731},
{ 1246,  1849,  2902,  4508,  7221, 12710, 14835, 16314, 19335, 22720},
{ 1525,  2260,  3862,  5659,  7342, 11748, 13370, 14442, 18044, 21334},
{ 1196,  1846,  3104,  7063, 10972, 12905, 14814, 17037, 19922, 22636},
{ 2147,  3106,  4475,  6511,  8227,  9765, 10984, 12161, 18971, 21300},
{ 1585,  2405,  2994,  4036, 11481, 13177, 14519, 15431, 19967, 21275},
{ 1778,  2688,  3614,  4680,  9465, 11064, 12473, 16320, 19742, 20800},
{ 1862,  2586,  3492,  6719, 11708, 13012, 14364, 16128, 19610, 20425},
{ 1395,  2156,  2669,  3386, 10607, 12125, 13614, 16705, 18976, 21367},
{ 1444,  2117,  3286,  6233,  9423, 12981, 14998, 15853, 17188, 21857},
{ 2004,  2895,  3783,  4897,  6168,  7297, 12609, 16445, 19297, 21465},
{ 1495,  2863,  6360,  8100, 11399, 14271, 15902, 17711, 20479, 22061},
{ 2484,  3114,  5718,  7097,  8400, 12616, 14073, 14847, 20535, 21396},
{ 2424,  3277,  5296,  6284, 11290, 12903, 16022, 17508, 19333, 20283},
{ 2565,  3778,  5360,  6989,  8782, 10428, 14390, 15742, 17770, 21734},
{ 2727,  3384,  6613,  9254, 10542, 12236, 14651, 15687, 20074, 21102},
{ 1916,  2953,  6274,  8088,  9710, 10925, 12392, 16434, 20010, 21183},
{ 3384,  4366,  5349,  7667, 11180, 12605, 13921, 15324, 19901, 20754},
{ 3075,  4283,  5951,  7619,  9604, 11010, 12384, 14006, 20658, 21497},
{ 1751,  2455,  5147,  9966, 11621, 13176, 14739, 16470, 20788, 21756},
{ 1442,  2188,  3330,  6813,  8929, 12135, 14476, 15306, 19635, 20544},
{ 2294,  2895,  4070,  8035, 12233, 13416, 14762, 17367, 18952, 19688},
{ 1937,  2659,  4602,  6697,  9071, 12863, 14197, 15230, 16047, 18877},
{ 2071,  2663,  4216,  9445, 10887, 12292, 13949, 14909, 19236, 20341},
{ 1740,  2491,  3488,  8138,  9656, 11153, 13206, 14688, 20896, 21907},
{ 2199,  2881,  4675,  8527, 10051, 11408, 14435, 15463, 17190, 20597},
{ 1943,  2988,  4177,  6039,  7478,  8536, 14181, 15551, 17622, 21579},
{ 1825,  3175,  7062,  9818, 12824, 15450, 18330, 19856, 21830, 22412},
{ 2464,  3046,  4822,  5977,  7696, 15398, 16730, 17646, 20588, 21320},
{ 2550,  3393,  5305,  6920, 10235, 14083, 18143, 19195, 20681, 21336},
{ 3003,  3799,  5321,  6437,  7919, 11643, 15810, 16846, 18119, 18980},
{ 3455,  4157,  6838,  8199,  9877, 12314, 15905, 16826, 19949, 20892},
{ 3052,  3769,  4891,  5810,  6977, 10126, 14788, 15990, 19773, 20904},
{ 3671,  4356,  5827,  6997,  8460, 12084, 14154, 14939, 19247, 20423},
{ 2716,  3684,  5246,  6686,  8463, 10001, 12394, 14131, 16150, 19776},
{ 1945,  2638,  4130,  7995, 14338, 15576, 17057, 18206, 20225, 20997},
{ 2304,  2928,  4122,  4824,  5640, 13139, 15825, 16938, 20108, 21054},
{ 1800,  2516,  3350,  5219, 13406, 15948, 17618, 18540, 20531, 21252},
{ 1436,  2224,  2753,  4546,  9657, 11245, 15177, 16317, 17489, 19135},
{ 2319,  2899,  4980,  6936,  8404, 13489, 15554, 16281, 20270, 20911},
{ 2187,  2919,  4610,  5875,  7390, 12556, 14033, 16794, 20998, 21769},
{ 2235,  2923,  5121,  6259,  8099, 13589, 15340, 16340, 17927, 20159},
{ 1765,  2638,  3751,  5730,  7883, 10108, 13633, 15419, 16808, 18574},
{ 3460,  5741,  9596, 11742, 14413, 16080, 18173, 19090, 20845, 21601},
{ 3735,  4426,  6199,  7363,  9250, 14489, 16035, 17026, 19873, 20876},
{ 3521,  4778,  6887,  8680, 12717, 14322, 15950, 18050, 20166, 21145},
{ 2141,  2968,  6865,  8051, 10010, 13159, 14813, 15861, 17528, 18655},
{ 4148,  6128,  9028, 10871, 12686, 14005, 15976, 17208, 19587, 20595},
{ 4403,  5367,  6634,  8371, 10163, 11599, 14963, 16331, 17982, 18768},
{ 4091,  5386,  6852,  8770, 11563, 13290, 15728, 16930, 19056, 20102},
{ 2746,  3625,  5299,  7504, 10262, 11432, 13172, 15490, 16875, 17514},
{ 2248,  3556,  8539, 10590, 12665, 14696, 16515, 17824, 20268, 21247},
{ 1279,  1960,  3920,  7793, 10153, 14753, 16646, 18139, 20679, 21466},
{ 2440,  3475,  6737,  8654, 12190, 14588, 17119, 17925, 19110, 19979},
{ 1879,  2514,  4497,  7572, 10017, 14948, 16141, 16897, 18397, 19376},
{ 2804,  3688,  7490, 10086, 11218, 12711, 16307, 17470, 20077, 21126},
{ 2023,  2682,  3873,  8268, 10255, 11645, 15187, 17102, 18965, 19788},
{ 2823,  3605,  5815,  8595, 10085, 11469, 16568, 17462, 18754, 19876},
{ 2851,  3681,  5280,  7648,  9173, 10338, 14961, 16148, 17559, 18474},
{ 1348,  2645,  5826,  8785, 10620, 12831, 16255, 18319, 21133, 22586},
{ 2141,  3036,  4293,  6082,  7593, 10629, 17158, 18033, 21466, 22084},
{ 1608,  2375,  3384,  6878,  9970, 11227, 16928, 17650, 20185, 21120},
{ 2774,  3616,  5014,  6557,  7788,  8959, 17068, 18302, 19537, 20542},
{ 1934,  4813,  6204,  7212,  8979, 11665, 15989, 17811, 20426, 21703},
{ 2288,  3507,  5037,  6841,  8278,  9638, 15066, 16481, 21653, 22214},
{ 2951,  3771,  4878,  7578,  9016, 10298, 14490, 15242, 20223, 20990},
{ 3256,  4791,  6601,  7521,  8644,  9707, 13398, 16078, 19102, 20249},
{ 1827,  2614,  3486,  6039, 12149, 13823, 16191, 17282, 21423, 22041},
{ 1000,  1704,  3002,  6335,  8471, 10500, 14878, 16979, 20026, 22427},
{ 1646,  2286,  3109,  7245, 11493, 12791, 16824, 17667, 18981, 20222},
{ 1708,  2501,  3315,  6737,  8729,  9924, 16089, 17097, 18374, 19917},
{ 2623,  3510,  4478,  5645,  9862, 11115, 15219, 18067, 19583, 20382},
{ 2518,  3434,  4728,  6388,  8082,  9285, 13162, 18383, 19819, 20552},
{ 1726,  2383,  4090,  6303,  7805, 12845, 14612, 17608, 19269, 20181},
{ 2860,  3735,  4838,  6044,  7254,  8402, 14031, 16381, 18037, 19410},
{ 4247,  5993,  7952,  9792, 12342, 14653, 17527, 18774, 20831, 21699},
{ 3502,  4051,  5680,  6805,  8146, 11945, 16649, 17444, 20390, 21564},
{ 3151,  4893,  5899,  7198, 11418, 13073, 15124, 17673, 20520, 21861},
{ 3960,  4848,  5926,  7259,  8811, 10529, 15661, 16560, 18196, 20183},
{ 4499,  6604,  8036,  9251, 10804, 12627, 15880, 17512, 20020, 21046},
{ 4251,  5541,  6654,  8318,  9900, 11686, 15100, 17093, 20572, 21687},
{ 3769,  5327,  7865,  9360, 10684, 11818, 13660, 15366, 18733, 19882},
{ 3083,  3969,  6248,  8121,  9798, 10994, 12393, 13686, 17888, 19105},
{ 2731,  4670,  7063,  9201, 11346, 13735, 16875, 18797, 20787, 22360},
{ 1187,  2227,  4737,  7214,  9622, 12633, 15404, 17968, 20262, 23533},
{ 1911,  2477,  3915, 10098, 11616, 12955, 16223, 17138, 19270, 20729},
{ 1764,  2519,  3887,  6944,  9150, 12590, 16258, 16984, 17924, 18435},
{ 1400,  3674,  7131,  8718, 10688, 12508, 15708, 17711, 19720, 21068},
{ 2322,  3073,  4287,  8108,  9407, 10628, 15862, 16693, 19714, 21474},
{ 2630,  3339,  4758,  8360, 10274, 11333, 12880, 17374, 19221, 19936},
{ 1721,  2577,  5553,  7195,  8651, 10686, 15069, 16953, 18703, 19929}
};


static Word16 lspcb2[NC1][M] = {        /* Q13 */
{ -435,  -815,  -742,  1033,  -518,   582, -1201,   829,    86,   385},
{ -833,  -891,   463,    -8, -1251,  1450,    72,  -231,   864,   661},
{-1021,   231,  -306,   321,  -220,  -163,  -526,  -754, -1633,   267},
{   57,  -198,  -339,   -33, -1468,   573,   796,  -169,  -631,   816},
{  171,  -350,   294,  1660,   453,   519,   291,   159,  -640, -1296},
{ -701,  -842,   -58,   950,   892,  1549,   715,   527,  -714,  -193},
{  584,    31,  -289,   356,  -333,  -457,   612,  -283, -1381,  -741},
{ -109,  -808,   231,    77,   -87,  -344,  1341,  1087,  -654,  -569},
{ -859,  1236,   550,   854,   714,  -543, -1752,  -195,   -98,  -276},
{ -877,  -954, -1248,  -299,   212,  -235,  -728,   949,  1517,   895},
{  -77,   344,  -620,   763,   413,   502,  -362,  -960,  -483,  1386},
{ -314,  -307,  -256, -1260,  -429,   450,  -466,  -108,  1010,  2223},
{  711,   693,   521,   650,  1305,   -28,  -378,   744, -1005,   240},
{ -112,  -271,  -500,   946,  1733,   271,   -15,   909,  -259,  1688},
{  575,   -10,  -468,  -199,  1101, -1011,   581,   -53,  -747,   878},
{  145,  -285, -1280,  -398,    36,  -498, -1377,    18,  -444,  1483},
{-1133,  -835,  1350,  1284,   -95,  1015,  -222,   443,   372,  -354},
{-1459, -1237,   416,  -213,   466,   669,   659,  1640,   932,   534},
{  -15,    66,   468,  1019,  -748,  1385,  -182,  -907,  -721,  -262},
{ -338,   148,  1445,    75,  -760,   569,  1247,   337,   416,  -121},
{  389,   239,  1568,   981,   113,   369, -1003,  -507,  -587,  -904},
{ -312,   -98,   949,    31,  1104,    72,  -141,  1465,    63,  -785},
{ 1127,   584,   835,   277, -1159,   208,   301,  -882,   117,  -404},
{  539,  -114,   856,  -493,   223,  -912,   623,   -76,   276,  -440},
{ 2197,  2337,  1268,   670,   304,  -267,  -525,   140,   882,  -139},
{-1596,   550,   801,  -456,   -56,  -697,   865,  1060,   413,   446},
{ 1154,   593,   -77,  1237,   -31,   581, -1037,  -895,   669,   297},
{  397,   558,   203,  -797,  -919,     3,   692,  -292,  1050,   782},
{  334,  1475,   632,   -80,    48, -1061,  -484,   362,  -597,  -852},
{ -545,  -330,  -429,  -680,  1133, -1182,  -744,  1340,   262,    63},
{ 1320,   827,  -398,  -576,   341,  -774,  -483, -1247,   -70,    98},
{ -163,   674,   -11,  -886,   531, -1125,  -265,  -242,   724,   934}
};
static Word16 fg[2][MA_NP][M] = {       /* Q15 */
  {
    { 8421,  9109,  9175,  8965,  9034,  9057,  8765,  8775,  9106,  8673},
    { 7018,  7189,  7638,  7307,  7444,  7379,  7038,  6956,  6930,  6868},
    { 5472,  4990,  5134,  5177,  5246,  5141,  5206,  5095,  4830,  5147},
    { 4056,  3031,  2614,  3024,  2916,  2713,  3309,  3237,  2857,  3473}
  },
  {
    { 7733,  7880,  8188,  8175,  8247,  8490,  8637,  8601,  8359,  7569},
    { 4210,  3031,  2552,  3473,  3876,  3853,  4184,  4154,  3909,  3968},
    { 3214,  1930,  1313,  2143,  2493,  2385,  2755,  2706,  2542,  2919},
    { 3024,  1592,   940,  1631,  1723,  1579,  2034,  2084,  1913,  2601}
  }
};
static Word16 fg_sum[2][M] = {      /* Q15 */
{ 7798,  8447,  8205,  8293,  8126,  8477,  8447,  8703,  9043,  8604},
{14585, 18333, 19772, 17344, 16426, 16459, 15155, 15220, 16043, 15708}
};
static Word16 fg_sum_inv[2][M] = {      /* Q12 */
{17210, 15888, 16357, 16183, 16516, 15833, 15888, 15421, 14840, 15597},
{ 9202,  7320,  6788,  7738,  8170,  8154,  8856,  8818,  8366,  8544}
};

/*-------------------------------------------------------------*
 *  Table for az_lsf()                                         *
 *                                                             *
 * Vector grid[] is in Q15                                     *
 *                                                             *
 * grid[0] = 1.0;                                              *
 * grid[grid_points+1] = -1.0;                                 *
 * for (i = 1; i < grid_points; i++)                           *
 *   grid[i] = cos((6.283185307*i)/(2.0*grid_points));         *
 *                                                             *
 *-------------------------------------------------------------*/

/* Version 51 points */
static Word16 grid[GRID_POINTS+1] ={
     32760,     32703,     32509,     32187,     31738,     31164,
     30466,     29649,     28714,     27666,     26509,     25248,
     23886,     22431,     20887,     19260,     17557,     15786,
     13951,     12062,     10125,      8149,      6140,      4106,
      2057,         0,     -2057,     -4106,     -6140,     -8149,
    -10125,    -12062,    -13951,    -15786,    -17557,    -19260,
    -20887,    -22431,    -23886,    -25248,    -26509,    -27666,
    -28714,    -29649,    -30466,    -31164,    -31738,    -32187,
    -32509,    -32703,    -32760};

/*-----------------------------------------------------*
 | Tables for pitch related routines .                 |
 -----------------------------------------------------*/

/* 1/3 resolution interpolation filter  (-3 dB at 3600 Hz)  in Q15 */

static Word16 inter_3l[FIR_SIZE_SYN] = {
   29443,
   25207,   14701,    3143,
   -4402,   -5850,   -2783,
    1211,    3130,    2259,
       0,   -1652,   -1666,
    -464,     756,    1099,
     550,    -245,    -634,
    -451,       0,     308,
     296,      78,    -120,
    -165,     -79,      34,
      91,      70,       0};

   /*Coefficients in floating point

   0.898517,
   0.769271,   0.448635,   0.095915,
  -0.134333,  -0.178528,  -0.084919,
   0.036952,   0.095533,   0.068936,
  -0.000000,  -0.050404,  -0.050835,
  -0.014169,   0.023083,   0.033543,
   0.016774,  -0.007466,  -0.019340,
  -0.013755,   0.000000,   0.009400,
   0.009029,   0.002381,  -0.003658,
  -0.005027,  -0.002405,   0.001050,
   0.002780,   0.002145,   0.000000};
  */

/*-----------------------------------------------------*
 | Tables for gain related routines .                  |
 -----------------------------------------------------*/

/* MA gain prediction coeff ={0.68, 0.58, 0.34, 0.19} in Q13 */
static Word16 pred[4] = { 5571, 4751, 2785, 1556 };


static Word16 gbk1[NCODE1][2] = {
/* Q14      Q13 */
 {    1 ,  1516 },
 { 1551 ,  2425 },
 { 1831 ,  5022 },
 {   57 ,  5404 },
 { 1921 ,  9291 },
 { 3242 ,  9949 },
 {  356 , 14756 },
 { 2678 , 27162 }
};

static Word16 gbk2[NCODE2][2] = {
/* Q14       Q13 */
 {   826 ,  2005 },
 {  1994 ,     0 },
 {  5142 ,   592 },
 {  6160 ,  2395 },
 {  8091 ,  4861 },
 {  9120 ,   525 },
 { 10573 ,  2966 },
 { 11569 ,  1196 },
 { 13260 ,  3256 },
 { 14194 ,  1630 },
 { 15132 ,  4914 },
 { 15161 , 14276 },
 { 15434 ,   237 },
 { 16112 ,  3392 },
 { 17299 ,  1861 },
 { 18973 ,  5935 }
};

static Word16 map1[NCODE1] = {
 5, 1, 4, 7, 3, 0, 6, 2
};

static Word16 map2[NCODE2] = {
 4, 6, 0, 2,12,14, 8,10,15,11, 9,13, 7, 3, 1, 5
};

/*  [0][0]      [0][1]       [1][0]     [1][1]    */
/*  Q10         Q14          Q16        Q19       */
static Word16 coef[2][2] = {
   { 31881 , 26416 },
   { 31548 , 27816 }
};
/*  [0][0]      [0][1]       [1][0]     [1][1]    */
/*  Q26         Q30          Q32        Q35       */
static Word32 L_coef[2][2] = {
   { 2089405952L , 1731217536L },
   { 2067549984L , 1822990272L }
};

static Word16 thr1[NCODE1-NCAN1] = {  /* Q14 */
   10808 ,
   12374 ,
   19778 ,
   32567
};
static Word16 thr2[NCODE2-NCAN2] = {  /* Q15 */
   14087 ,
   16188 ,
   20274 ,
   21321 ,
   23525 ,
   25232 ,
   27873 ,
   30542
};
static Word16 imap1[NCODE1] = {
 5, 1, 7, 4, 2, 0, 6, 3
};

static Word16 imap2[NCODE2] = {
 2,14, 3,13, 0,15, 1,12, 6,10, 7, 9, 4,11, 5, 8
};

/*-----------------------------------------------------*
 | Tables for routine bits().                          |
 -----------------------------------------------------*/


static Word16 bitsno[PRM_SIZE] = {1+NC0_B,        /* MA + 1st stage   */
                                 NC1_B*2,         /* 2nd stage        */
                                 8, 1, 13, 4, 7,  /* first subframe   */
                                 5,    13, 4, 7}; /* second subframe  */


/*-----------------------------------------------------*
 | Table for routine Pow2().                           |
 -----------------------------------------------------*/

static Word16 tabpow[33] = {
 16384, 16743, 17109, 17484, 17867, 18258, 18658, 19066, 19484, 19911,
 20347, 20792, 21247, 21713, 22188, 22674, 23170, 23678, 24196, 24726,
 25268, 25821, 26386, 26964, 27554, 28158, 28774, 29405, 30048, 30706,
 31379, 32066, 32767 };

/*-----------------------------------------------------*
 | Table for routine Log2().                           |
 -----------------------------------------------------*/

static Word16 tablog[33] = {
     0,  1455,  2866,  4236,  5568,  6863,  8124,  9352, 10549, 11716,
 12855, 13967, 15054, 16117, 17156, 18172, 19167, 20142, 21097, 22033,
 22951, 23852, 24735, 25603, 26455, 27291, 28113, 28922, 29716, 30497,
 31266, 32023, 32767 };

/*-----------------------------------------------------*
 | Table for routine Inv_sqrt().                       |
 -----------------------------------------------------*/

static Word16 tabsqr[49] = {

 32767, 31790, 30894, 30070, 29309, 28602, 27945, 27330, 26755, 26214,
 25705, 25225, 24770, 24339, 23930, 23541, 23170, 22817, 22479, 22155,
 21845, 21548, 21263, 20988, 20724, 20470, 20225, 19988, 19760, 19539,
 19326, 19119, 18919, 18725, 18536, 18354, 18176, 18004, 17837, 17674,
 17515, 17361, 17211, 17064, 16921, 16782, 16646, 16514, 16384 };

/*-----------------------------------------------------*
 | Table for taming procedure test_err.                |
 -----------------------------------------------------*/

static Word16 tab_zone[PIT_MAX+L_INTERPOL-1] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

/************** End of file tab_ld8a.h **************************************/

/************** Begin of file libavcodec_put_bits.h *************************/
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
#elif defined(ARCH_X86)
  uint32_t t;
  __asm__ ("eor %1, %0, %0, ror #16 \n\t"
           "bic %1, %1, #0xFF0000   \n\t"
           "mov %0, %0, ror #8      \n\t"
           "eor %0, %0, %1, lsr #8  \n\t"
           : "+r"(x), "=&r"(t));
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
/************** End of file libavcodec_put_bits.h ***************************/

/************** Begin of file libavcodec_get_bits.h *************************/
/**
 * @file libavcodec/get_bits.h
 * bitstream reader API header.
 */

#ifndef AVCODEC_GET_BITS_H
#define AVCODEC_GET_BITS_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
/*#include "libavutil/bswap.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"*/
#define AV_RB32(x)  \
  ((((const uint8_t*)(x))[0] << 24) | \
  (((const uint8_t*)(x))[1] << 16) | \
  (((const uint8_t*)(x))[2] <<  8) | \
  ((const uint8_t*)(x))[3])
/*#include "libavutil/log.h"
#include "mathops.h"*/

#if defined(ALT_BITSTREAM_READER_LE) && !defined(ALT_BITSTREAM_READER)
#   define ALT_BITSTREAM_READER
#endif

#if !defined(LIBMPEG2_BITSTREAM_READER) && !defined(A32_BITSTREAM_READER) && !defined(ALT_BITSTREAM_READER)
#   if defined(ARCH_ARM)
#       define A32_BITSTREAM_READER
#   else
#       define ALT_BITSTREAM_READER
//#define LIBMPEG2_BITSTREAM_READER
//#define A32_BITSTREAM_READER
#   endif
#endif

extern const uint8_t ff_reverse[256];

#if defined(ARCH_X86)
// avoid +32 for shift optimization (gcc should do that ...)
static  int32_t NEG_SSR32( int32_t a, int8_t s){
  __asm__ ("sarl %1, %0\n\t"
           : "+r" (a)
           : "ic" ((uint8_t)(-s))
           );
  return a;
}
static uint32_t NEG_USR32(uint32_t a, int8_t s){
  __asm__ ("shrl %1, %0\n\t"
           : "+r" (a)
           : "ic" ((uint8_t)(-s))
           );
  return a;
}
#else
#    define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#    define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif

/* bit input */
/* buffer, buffer_end and size_in_bits must be present and used by every reader */
typedef struct GetBitContext {
  const uint8_t *buffer, *buffer_end;
#ifdef ALT_BITSTREAM_READER
  int index;
#elif defined LIBMPEG2_BITSTREAM_READER
  uint8_t *buffer_ptr;
  uint32_t cache;
  int bit_count;
#elif defined A32_BITSTREAM_READER
  uint32_t *buffer_ptr;
  uint32_t cache0;
  uint32_t cache1;
  int bit_count;
#endif
  int size_in_bits;
} GetBitContext;

#define VLC_TYPE int16_t

typedef struct VLC {
  int bits;
  VLC_TYPE (*table)[2]; ///< code, bits
  int table_size, table_allocated;
} VLC;

typedef struct RL_VLC_ELEM {
  int16_t level;
  int8_t len;
  uint8_t run;
} RL_VLC_ELEM;

/* Bitstream reader API docs:
 name
 arbitrary name which is used as prefix for the internal variables
 
 gb
 getbitcontext
 
 OPEN_READER(name, gb)
 loads gb into local variables
 
 CLOSE_READER(name, gb)
 stores local vars in gb
 
 UPDATE_CACHE(name, gb)
 refills the internal cache from the bitstream
 after this call at least MIN_CACHE_BITS will be available,
 
 GET_CACHE(name, gb)
 will output the contents of the internal cache, next bit is MSB of 32 or 64 bit (FIXME 64bit)
 
 SHOW_UBITS(name, gb, num)
 will return the next num bits
 
 SHOW_SBITS(name, gb, num)
 will return the next num bits and do sign extension
 
 SKIP_BITS(name, gb, num)
 will skip over the next num bits
 note, this is equivalent to SKIP_CACHE; SKIP_COUNTER
 
 SKIP_CACHE(name, gb, num)
 will remove the next num bits from the cache (note SKIP_COUNTER MUST be called before UPDATE_CACHE / CLOSE_READER)
 
 SKIP_COUNTER(name, gb, num)
 will increment the internal bit counter (see SKIP_CACHE & SKIP_BITS)
 
 LAST_SKIP_CACHE(name, gb, num)
 will remove the next num bits from the cache if it is needed for UPDATE_CACHE otherwise it will do nothing
 
 LAST_SKIP_BITS(name, gb, num)
 is equivalent to SKIP_LAST_CACHE; SKIP_COUNTER
 
 for examples see get_bits, show_bits, skip_bits, get_vlc
 */

#ifdef ALT_BITSTREAM_READER
#   define MIN_CACHE_BITS 25

#   define OPEN_READER(name, gb)\
int name##_index= (gb)->index;\
int name##_cache= 0;\

#   define CLOSE_READER(name, gb)\
(gb)->index= name##_index;\

# ifdef ALT_BITSTREAM_READER_LE
#   define UPDATE_CACHE(name, gb)\
name##_cache= AV_RL32( ((const uint8_t *)(gb)->buffer)+(name##_index>>3) ) >> (name##_index&0x07);\

#   define SKIP_CACHE(name, gb, num)\
name##_cache >>= (num);
# else
#   define UPDATE_CACHE(name, gb)\
name##_cache= AV_RB32( ((const uint8_t *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#   define SKIP_CACHE(name, gb, num)\
name##_cache <<= (num);
# endif

// FIXME name?
#   define SKIP_COUNTER(name, gb, num)\
name##_index += (num);\

#   define SKIP_BITS(name, gb, num)\
{\
SKIP_CACHE(name, gb, num)\
SKIP_COUNTER(name, gb, num)\
}\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) ;

# ifdef ALT_BITSTREAM_READER_LE
#   define SHOW_UBITS(name, gb, num)\
((name##_cache) & (NEG_USR32(0xffffffff,num)))

#   define SHOW_SBITS(name, gb, num)\
NEG_SSR32((name##_cache)<<(32-(num)), num)
# else
#   define SHOW_UBITS(name, gb, num)\
NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
NEG_SSR32(name##_cache, num)
# endif

#   define GET_CACHE(name, gb)\
((uint32_t)name##_cache)

static int get_bits_count(GetBitContext *s){
  return s->index;
}

static void skip_bits_long(GetBitContext *s, int n){
  s->index += n;
}

#elif defined LIBMPEG2_BITSTREAM_READER
//libmpeg2 like reader

#   define MIN_CACHE_BITS 17

#   define OPEN_READER(name, gb)\
int name##_bit_count=(gb)->bit_count;\
int name##_cache= (gb)->cache;\
uint8_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
(gb)->bit_count= name##_bit_count;\
(gb)->cache= name##_cache;\
(gb)->buffer_ptr= name##_buffer_ptr;\

#   define UPDATE_CACHE(name, gb)\
if(name##_bit_count >= 0){\
name##_cache+= AV_RB16(name##_buffer_ptr) << name##_bit_count; \
name##_buffer_ptr+=2;\
name##_bit_count-= 16;\
}\

#   define SKIP_CACHE(name, gb, num)\
name##_cache <<= (num);\

#   define SKIP_COUNTER(name, gb, num)\
name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
{\
SKIP_CACHE(name, gb, num)\
SKIP_COUNTER(name, gb, num)\
}\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
NEG_SSR32(name##_cache, num)

#   define GET_CACHE(name, gb)\
((uint32_t)name##_cache)

static int get_bits_count(GetBitContext *s){
  return (s->buffer_ptr - s->buffer)*8 - 16 + s->bit_count;
}

static void skip_bits_long(GetBitContext *s, int n){
  OPEN_READER(re, s)
  re_bit_count += n;
  re_buffer_ptr += 2*(re_bit_count>>4);
  re_bit_count &= 15;
  re_cache = ((re_buffer_ptr[-2]<<8) + re_buffer_ptr[-1]) << (16+re_bit_count);
  UPDATE_CACHE(re, s)
  CLOSE_READER(re, s)
}

#elif defined A32_BITSTREAM_READER

#   define MIN_CACHE_BITS 32

#   define OPEN_READER(name, gb)\
int name##_bit_count=(gb)->bit_count;\
uint32_t name##_cache0= (gb)->cache0;\
uint32_t name##_cache1= (gb)->cache1;\
uint32_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
(gb)->bit_count= name##_bit_count;\
(gb)->cache0= name##_cache0;\
(gb)->cache1= name##_cache1;\
(gb)->buffer_ptr= name##_buffer_ptr;\

#   define UPDATE_CACHE(name, gb)\
if(name##_bit_count > 0){\
const uint32_t next= be2me_32( *name##_buffer_ptr );\
name##_cache0 |= NEG_USR32(next,name##_bit_count);\
name##_cache1 |= next<<name##_bit_count;\
name##_buffer_ptr++;\
name##_bit_count-= 32;\
}\

#if defined(ARCH_X86)
#   define SKIP_CACHE(name, gb, num)\
__asm__(\
"shldl %2, %1, %0          \n\t"\
"shll %2, %1               \n\t"\
: "+r" (name##_cache0), "+r" (name##_cache1)\
: "Ic" ((uint8_t)(num))\
);
#else
#   define SKIP_CACHE(name, gb, num)\
name##_cache0 <<= (num);\
name##_cache0 |= NEG_USR32(name##_cache1,num);\
name##_cache1 <<= (num);
#endif

#   define SKIP_COUNTER(name, gb, num)\
name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
{\
SKIP_CACHE(name, gb, num)\
SKIP_COUNTER(name, gb, num)\
}\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
NEG_USR32(name##_cache0, num)

#   define SHOW_SBITS(name, gb, num)\
NEG_SSR32(name##_cache0, num)

#   define GET_CACHE(name, gb)\
(name##_cache0)

static int get_bits_count(GetBitContext *s){
  return ((uint8_t*)s->buffer_ptr - s->buffer)*8 - 32 + s->bit_count;
}

static void skip_bits_long(GetBitContext *s, int n){
  OPEN_READER(re, s)
  re_bit_count += n;
  re_buffer_ptr += re_bit_count>>5;
  re_bit_count &= 31;
  re_cache0 = be2me_32( re_buffer_ptr[-1] ) << re_bit_count;
  re_cache1 = 0;
  UPDATE_CACHE(re, s)
  CLOSE_READER(re, s)
}

#endif

/**
 * read mpeg1 dc style vlc (sign bit + mantisse with no MSB).
 * if MSB not set it is negative
 * @param n length in bits
 * @author BERO
 */
static int get_xbits(GetBitContext *s, int n){
  register int sign;
  register int32_t cache;
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  cache = GET_CACHE(re,s);
  sign=(~cache)>>31;
  LAST_SKIP_BITS(re, s, n)
  CLOSE_READER(re, s)
  return (NEG_USR32(sign ^ cache, n) ^ sign) - sign;
}

static int get_sbits(GetBitContext *s, int n){
  register int tmp;
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  tmp= SHOW_SBITS(re, s, n);
  LAST_SKIP_BITS(re, s, n)
  CLOSE_READER(re, s)
  return tmp;
}

/**
 * reads 1-17 bits.
 * Note, the alt bitstream reader can read up to 25 bits, but the libmpeg2 reader can't
 */
static unsigned int get_bits(GetBitContext *s, int n){
  register int tmp;
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  tmp= SHOW_UBITS(re, s, n);
  LAST_SKIP_BITS(re, s, n)
  CLOSE_READER(re, s)
  return tmp;
}

/**
 * shows 1-17 bits.
 * Note, the alt bitstream reader can read up to 25 bits, but the libmpeg2 reader can't
 */
static unsigned int show_bits(GetBitContext *s, int n){
  register int tmp;
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  tmp= SHOW_UBITS(re, s, n);
  //    CLOSE_READER(re, s)
  return tmp;
}

static void skip_bits(GetBitContext *s, int n){
  //Note gcc seems to optimize this to s->index+=n for the ALT_READER :))
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  LAST_SKIP_BITS(re, s, n)
  CLOSE_READER(re, s)
}

static unsigned int get_bits1(GetBitContext *s){
#ifdef ALT_BITSTREAM_READER
  int index= s->index;
  uint8_t result= s->buffer[ index>>3 ];
#ifdef ALT_BITSTREAM_READER_LE
  result>>= (index&0x07);
  result&= 1;
#else
  result<<= (index&0x07);
  result>>= 8 - 1;
#endif
  index++;
  s->index= index;
  
  return result;
#else
  return get_bits(s, 1);
#endif
}

static unsigned int show_bits1(GetBitContext *s){
  return show_bits(s, 1);
}

static void skip_bits1(GetBitContext *s){
  skip_bits(s, 1);
}

/**
 * reads 0-32 bits.
 */
#if 0
static unsigned int get_bits_long(GetBitContext *s, int n){
  if(n<=17) return get_bits(s, n);
  else{
#ifdef ALT_BITSTREAM_READER_LE
    int ret= get_bits(s, 16);
    return ret | (get_bits(s, n-16) << 16);
#else
    int ret= get_bits(s, 16) << (n-16);
    return ret | get_bits(s, n-16);
#endif
  }
}

/**
 * reads 0-32 bits as a signed integer.
 */
static int get_sbits_long(GetBitContext *s, int n) {
  return sign_extend(get_bits_long(s, n), n);
}

/**
 * shows 0-32 bits.
 */
static unsigned int show_bits_long(GetBitContext *s, int n){
  if(n<=17) return show_bits(s, n);
  else{
    GetBitContext gb= *s;
    return get_bits_long(&gb, n);
  }
}
#endif
static int check_marker(GetBitContext *s, const char *msg)
{
  int bit= get_bits1(s);
  //if(!bit)
  //av_log(NULL, AV_LOG_INFO, "Marker bit missing %s\n", msg);
  
  return bit;
}

/**
 * init GetBitContext.
 * @param buffer bitstream buffer, must be FF_INPUT_BUFFER_PADDING_SIZE bytes larger then the actual read bits
 * because some optimized bitstream readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 */
static void init_get_bits(GetBitContext *s,
                                 const uint8_t *buffer, int bit_size)
{
  int buffer_size= (bit_size+7)>>3;
  if(buffer_size < 0 || bit_size < 0) {
    buffer_size = bit_size = 0;
    buffer = NULL;
  }
  
  s->buffer= buffer;
  s->size_in_bits= bit_size;
  s->buffer_end= buffer + buffer_size;
#ifdef ALT_BITSTREAM_READER
  s->index=0;
#elif defined LIBMPEG2_BITSTREAM_READER
  s->buffer_ptr = (uint8_t*)((intptr_t)buffer&(~1));
  s->bit_count = 16 + 8*((intptr_t)buffer&1);
  skip_bits_long(s, 0);
#elif defined A32_BITSTREAM_READER
  s->buffer_ptr = (uint32_t*)((intptr_t)buffer&(~3));
  s->bit_count = 32 + 8*((intptr_t)buffer&3);
  skip_bits_long(s, 0);
#endif
}

static void align_get_bits(GetBitContext *s)
{
  int n= (-get_bits_count(s)) & 7;
  if(n) skip_bits(s, n);
}


#ifdef VLC
#define init_vlc(vlc, nb_bits, nb_codes,\
bits, bits_wrap, bits_size,\
codes, codes_wrap, codes_size,\
flags)\
init_vlc_sparse(vlc, nb_bits, nb_codes,\
bits, bits_wrap, bits_size,\
codes, codes_wrap, codes_size,\
NULL, 0, 0, flags)

int init_vlc_sparse(VLC *vlc, int nb_bits, int nb_codes,
                    const void *bits, int bits_wrap, int bits_size,
                    const void *codes, int codes_wrap, int codes_size,
                    const void *symbols, int symbols_wrap, int symbols_size,
                    int flags);
#define INIT_VLC_USE_STATIC 1 ///< VERY strongly deprecated and forbidden
#define INIT_VLC_LE         2
#define INIT_VLC_USE_NEW_STATIC 4
void free_vlc(VLC *vlc);

#define INIT_VLC_STATIC(vlc, bits, a,b,c,d,e,f,g, static_size)\
{\
static VLC_TYPE table[static_size][2];\
(vlc)->table= table;\
(vlc)->table_allocated= static_size;\
init_vlc(vlc, bits, a,b,c,d,e,f,g, INIT_VLC_USE_NEW_STATIC);\
}


/**
 *
 * if the vlc code is invalid and max_depth=1 than no bits will be removed
 * if the vlc code is invalid and max_depth>1 than the number of bits removed
 * is undefined
 */
#define GET_VLC(code, name, gb, table, bits, max_depth)\
{\
int n, index, nb_bits;\
\
index= SHOW_UBITS(name, gb, bits);\
code = table[index][0];\
n    = table[index][1];\
\
if(max_depth > 1 && n < 0){\
LAST_SKIP_BITS(name, gb, bits)\
UPDATE_CACHE(name, gb)\
\
nb_bits = -n;\
\
index= SHOW_UBITS(name, gb, nb_bits) + code;\
code = table[index][0];\
n    = table[index][1];\
if(max_depth > 2 && n < 0){\
LAST_SKIP_BITS(name, gb, nb_bits)\
UPDATE_CACHE(name, gb)\
\
nb_bits = -n;\
\
index= SHOW_UBITS(name, gb, nb_bits) + code;\
code = table[index][0];\
n    = table[index][1];\
}\
}\
SKIP_BITS(name, gb, n)\
}

#define GET_RL_VLC(level, run, name, gb, table, bits, max_depth, need_update)\
{\
int n, index, nb_bits;\
\
index= SHOW_UBITS(name, gb, bits);\
level = table[index].level;\
n     = table[index].len;\
\
if(max_depth > 1 && n < 0){\
SKIP_BITS(name, gb, bits)\
if(need_update){\
UPDATE_CACHE(name, gb)\
}\
\
nb_bits = -n;\
\
index= SHOW_UBITS(name, gb, nb_bits) + level;\
level = table[index].level;\
n     = table[index].len;\
}\
run= table[index].run;\
SKIP_BITS(name, gb, n)\
}


/**
 * parses a vlc code, faster then get_vlc()
 * @param bits is the number of bits which will be read at once, must be
 *             identical to nb_bits in init_vlc()
 * @param max_depth is the number of times bits bits must be read to completely
 *                  read the longest vlc code
 *                  = (max_vlc_length + bits - 1) / bits
 */
static av_always_inline int get_vlc2(GetBitContext *s, VLC_TYPE (*table)[2],
                                     int bits, int max_depth)
{
  int code;
  
  OPEN_READER(re, s)
  UPDATE_CACHE(re, s)
  
  GET_VLC(code, re, s, table, bits, max_depth)
  
  CLOSE_READER(re, s)
  return code;
}
#endif

//#define TRACE

#ifdef TRACE
static void print_bin(int bits, int n){
  int i;
  
  for(i=n-1; i>=0; i--){
    av_log(NULL, AV_LOG_DEBUG, "%d", (bits>>i)&1);
  }
  for(i=n; i<24; i++)
    av_log(NULL, AV_LOG_DEBUG, " ");
}

static int get_bits_trace(GetBitContext *s, int n, char *file, const char *func, int line){
  int r= get_bits(s, n);
  
  print_bin(r, n);
  av_log(NULL, AV_LOG_DEBUG, "%5d %2d %3d bit @%5d in %s %s:%d\n", r, n, r, get_bits_count(s)-n, file, func, line);
  return r;
}
static int get_vlc_trace(GetBitContext *s, VLC_TYPE (*table)[2], int bits, int max_depth, char *file, const char *func, int line){
  int show= show_bits(s, 24);
  int pos= get_bits_count(s);
  int r= get_vlc2(s, table, bits, max_depth);
  int len= get_bits_count(s) - pos;
  int bits2= show>>(24-len);
  
  print_bin(bits2, len);
  
  av_log(NULL, AV_LOG_DEBUG, "%5d %2d %3d vlc @%5d in %s %s:%d\n", bits2, len, r, pos, file, func, line);
  return r;
}
static int get_xbits_trace(GetBitContext *s, int n, char *file, const char *func, int line){
  int show= show_bits(s, n);
  int r= get_xbits(s, n);
  
  print_bin(show, n);
  av_log(NULL, AV_LOG_DEBUG, "%5d %2d %3d xbt @%5d in %s %s:%d\n", show, n, r, get_bits_count(s)-n, file, func, line);
  return r;
}

#define get_bits(s, n)  get_bits_trace(s, n, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define get_bits1(s)    get_bits_trace(s, 1, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define get_xbits(s, n) get_xbits_trace(s, n, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define get_vlc(s, vlc)            get_vlc_trace(s, (vlc)->table, (vlc)->bits, 3, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define get_vlc2(s, tab, bits, max) get_vlc_trace(s, tab, bits, max, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define tprintf(p, ...) av_log(p, AV_LOG_DEBUG, __VA_ARGS__)

#else //TRACE
#define tprintf(p, ...) {}
#endif

static int decode012(GetBitContext *gb){
  int n;
  n = get_bits1(gb);
  if (n == 0)
    return 0;
  else
    return get_bits1(gb) + 1;
}

static int decode210(GetBitContext *gb){
  if (get_bits1(gb))
    return 0;
  else
    return 2 - get_bits1(gb);
}

#endif /* AVCODEC_GET_BITS_H */
/************** End of file libavcodec_put_bits.h ***************************/

/************** Begin of file g729a_decoder.h *******************************/
#ifndef __G729A_DECODER_H__
#define __G729A_DECODER_H__

typedef struct g729a_decoder_state
{
  /* Excitation vector */
  Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
  Word16 *exc;

  /* Lsp (Line spectral pairs) */
  Word16 lsp_old[M];

  /* Filter's memory */
  Word16 mem_syn[M];

  Word16 sharp;           /* pitch sharpening of previous frame */
  Word16 old_T0;          /* integer delay of previous frame    */
  Word16 gain_code;       /* Code gain                          */
  Word16 gain_pitch;      /* Pitch gain                         */

  /* Sub state */
  //lspdec.c
  Word16 freq_prev[MA_NP][M];   /* Q13 */
  Word16 prev_ma;                  /* previous MA prediction coef.*/
  Word16 prev_lsp[M];              /* previous LSP vector         */
} g729a_decoder_state;

typedef struct g729a_post_filter_state
{
  /* inverse filtered synthesis (with A(z/GAMMA2_PST))   */
  Word16 res2_buf[PIT_MAX+L_SUBFR];
  Word16 *res2;
  Word16 scal_res2_buf[PIT_MAX+L_SUBFR];
  Word16 *scal_res2;

  /* memory of filter 1/A(z/GAMMA1_PST) */
  Word16 mem_syn_pst[M];
} g729a_post_filter_state;

typedef struct g729a_post_process_state
{
  Word16 y1_hi, y1_lo;
  Word16 y2_hi, y2_lo;
  //Word32 y1, y2;
  Word16 x1, x2;
} g729a_post_process_state;

typedef struct g729a_decode_frame_state
{
  Word16 synth_buf[L_FRAME+M];
  Word16 *synth;

  g729a_decoder_state      decoderState;
  g729a_post_filter_state  postFilterState;
  g729a_post_process_state postProcessState;
} g729a_decode_frame_state;

static void Init_Decod_ld8a(g729a_decoder_state *state);

static void Decod_ld8a(
    g729a_decoder_state *state,
    Word16  parm[],      /* (i)   : vector of synthesis parameters
                                    parm[0] = bad frame indicator (bfi)  */
    Word16  synth[],     /* (o)   : synthesis speech                     */
    Word16  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
    Word16  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
    Word16 bad_lsf       /* (i)   : bad LSF indicator                    */
  );

static void Init_Post_Filter(g729a_post_filter_state *state);

static void Post_Filter(
    g729a_post_filter_state *state,
    Word16 *syn,       /* in/out: synthesis speech (postfiltered is output)    */
    Word16 *Az_4,       /* input : interpolated LPC parameters in all subframes */
    Word16 *T            /* input : decoded pitch lags in all subframes          */
  );

static void Init_Post_Process(g729a_post_process_state *state);

static void Post_Process(
 g729a_post_process_state *state,
 Word16 sigin[],    /* input signal */
 Word16 sigout[],   /* output signal */
 Word16 lg          /* Length of signal    */
);


static void D_lsp(
  g729a_decoder_state *state,
  Word16 prm[],          /* (i)     : indexes of the selected LSP */
  Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  Word16 erase           /* (i)     : frame erase information     */
);

static void Lsp_decw_reset(g729a_decoder_state *state);
#endif /* __G729A_DECODER_H__ */
/************** End of file g729a_decoder.h *********************************/

/************** Begin of file g729a_encoder.h *******************************/
#ifndef __G729A_ENCODER_H__
#define __G729A_ENCODER_H__

typedef struct g729a_pre_process_state
{
  Word16 y1_hi, y1_lo;
  Word16 y2_hi, y2_lo;
  Word16 x1, x2;
  //static Word32 y1, y2;
} g729a_pre_process_state;

typedef struct g729a_encoder_state
{
  /* Speech vector */
  Word16 old_speech[L_TOTAL];
  Word16 *speech, *p_window;
  Word16 *new_speech;                    /* Global variable */

  /* Weighted speech vector */
  Word16 old_wsp[L_FRAME+PIT_MAX];
  Word16 *wsp;

  /* Excitation vector */
  Word16 old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
  Word16 *exc;

  /* Lsp (Line spectral pairs) */
  Word16 lsp_old[M]/*={
        30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000}*/;
  Word16 lsp_old_q[M];

  /* Filter's memory */
  Word16  mem_w0[M], mem_w[M], mem_zero[M];
  Word16  sharp;

  /* Sub state */
  // qua_lsp.c
  Word16 freq_prev[MA_NP][M];    /* Q13:previous LSP vector       */
  // taming.c
  Word32 L_exc_err[4];
} g729a_encoder_state;


typedef struct g729a_encode_frame_state
{
  g729a_encoder_state     encoderState;
  g729a_pre_process_state preProcessState;
} g729a_encode_frame_state;

static void Init_Coder_ld8a  (g729a_encoder_state *state);
static void Coder_ld8a       (g729a_encoder_state *state,
                       Word16 ana[]);

static void Init_Pre_Process(g729a_pre_process_state *state);
static void Pre_Process     (g729a_pre_process_state *state,
                      Word16 sigin[],
                      Word16 sigout[],
                      Word16 lg);


static void   Init_exc_err(g729a_encoder_state *state);
static Word16 test_err    (g729a_encoder_state *state,
                    Word16 T0,
                    Word16 T0_frac);
static void update_exc_err(g729a_encoder_state *state,
                    Word16 gain_pit,
                    Word16 T0);

static void Lsp_encw_reset(g729a_encoder_state *state);
static void Qua_lsp       (g729a_encoder_state *state,
                    Word16 lsp[],
                    Word16 lsp_q[],
                    Word16 ana[]);
  
#endif /* __G729A_ENCODER_H__ */
/************** End of file g729a_encoder.h *********************************/

/************** Begin of file acelp_ca.c ************************************/

/*---------------------------------------------------------------------------*
 *  Function  ACELP_Code_A()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
 *   Find Algebraic codebook for G.729A                                      *
 *--------------------------------------------------------------------------*/

/* Constants defined in ld8a.h */
/*  L_SUBFR   -> Lenght of subframe.                                        */
/*  NB_POS    -> Number of positions for each pulse.                        */
/*  STEP      -> Step betweem position of the same pulse.                   */
/*  MSIZE     -> Size of vectors for cross-correlation between two pulses.  */


/* local routines definition */

static void Cor_h(
     Word16 *H,         /* (i) Q12 :Impulse response of filters */
     Word16 *rr         /* (o)     :Correlations of H[]         */
);
static Word16 D4i40_17_fast(/*(o) : Index of pulses positions.               */
  Word16 dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  Word16 *rr,           /* (i)    : Correlations of impulse response h[].    */
  Word16 h[],           /* (i) Q12: Impulse response of filters.             */
  Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
  Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
  Word16 *sign          /* (o)    : Signs of 4 pulses.                       */
);

 /*-----------------------------------------------------------------*
  * Main ACELP function.                                            *
  *-----------------------------------------------------------------*/

static Word16  ACELP_Code_A(    /* (o)     :index of pulses positions    */
  Word16 x[],            /* (i)     :Target vector                */
  Word16 h[],            /* (i) Q12 :Inpulse response of filters  */
  Word16 T0,             /* (i)     :Pitch lag                    */
  Word16 pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
  Word16 code[],         /* (o) Q13 :Innovative codebook          */
  Word16 y[],            /* (o) Q12 :Filtered innovative codebook */
  Word16 *sign           /* (o)     :Signs of 4 pulses            */
)
{
  Word16 i, index, sharp;
  Word16 Dn[L_SUBFR];
  Word16 rr[DIM_RR];

 /*-----------------------------------------------------------------*
  * Include fixed-gain pitch contribution into impulse resp. h[]    *
  * Find correlations of h[] needed for the codebook search.        *
  *-----------------------------------------------------------------*/

  //sharp = shl(pitch_sharp, 1);          /* From Q14 to Q15 */
  sharp = pitch_sharp << 1;          /* From Q14 to Q15 */
  if (T0 < L_SUBFR)
     for (i = T0; i < L_SUBFR; i++)     /* h[i] += pitch_sharp*h[i-T0] */
       h[i] = add(h[i], mult(h[i-T0], sharp));

  Cor_h(h, rr);

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  Cor_h_X(h, x, Dn);

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
       code[i] = add(code[i], mult(code[i-T0], sharp));

  return index;
}



/*--------------------------------------------------------------------------*
 *  Function  Cor_h()                                                       *
 *  ~~~~~~~~~~~~~~~~~                                                       *
 * Compute  correlations of h[]  needed for the codebook search.            *
 *--------------------------------------------------------------------------*/

static void Cor_h(
  Word16 *H,     /* (i) Q12 :Impulse response of filters */
  Word16 *rr     /* (o)     :Correlations of H[]         */
)
{
  Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  Word16 *rri1i2, *rri1i3, *rri1i4;
  Word16 *rri2i3, *rri2i4;

  Word16 *p0, *p1, *p2, *p3, *p4;

  Word16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
  Word32 cor;
  Word16 i, k, ldec, l_fin_sup, l_fin_inf;
  Word16 h[L_SUBFR];

 /* Scaling h[] for maximum precision */

  cor = 0;
  for(i=0; i<L_SUBFR; i++)
    cor += H[i] * H[i];
	cor <<= 1;

  if(extract_h(cor) > 32000)
  {
    for(i=0; i<L_SUBFR; i++)
      h[i] >>= 1;
  }
  else
  {
    k = norm_l(cor);
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
  l_fin_inf = l_fin_sup-(Word16)1;
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

          for(i=k+(Word16)1; i<NB_POS; i++ ) {

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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri2i4 + l_fin_sup;
          p3 = rri1i3 + l_fin_sup;
          p2 = rri0i2 + l_fin_sup;
          p1 = rri1i4 + l_fin_inf;
          p0 = rri0i3 + l_fin_inf;

          cor = 0;
          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p4 = rri1i4 + l_fin_sup;
          p3 = rri0i3 + l_fin_sup;
          p2 = rri2i4 + l_fin_inf;
          p1 = rri1i3 + l_fin_inf;
          p0 = rri0i2 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

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
  l_fin_inf = l_fin_sup-(Word16)1;
  for(k=0; k<NB_POS; k++) {

          p3 = rri0i4 + l_fin_sup;
          p2 = rri2i3 + l_fin_inf;
          p1 = rri1i2 + l_fin_inf;
          p0 = rri0i1 + l_fin_inf;

          ptr_h1 = ptr_hd;
          ptr_h2 =  ptr_hf;
          cor = 0;
          for(i=k+(Word16)1; i<NB_POS; i++ ) {

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

static Word16 D4i40_17_fast(/*(o) : Index of pulses positions.               */
  Word16 dn[],          /* (i)    : Correlations between h[] and Xn[].       */
  Word16 rr[],          /* (i)    : Correlations of impulse response h[].    */
  Word16 h[],           /* (i) Q12: Impulse response of filters.             */
  Word16 cod[],         /* (o) Q13: Selected algebraic codeword.             */
  Word16 y[],           /* (o) Q12: Filtered algebraic codeword.             */
  Word16 *sign          /* (o)    : Signs of 4 pulses.                       */
)
{
  Word16 i0, i1, i2, i3, ip0, ip1, ip2, ip3;
  Word16 i, j, ix, iy, track, trk, max;
  Word16 prev_i0, i1_offset;
  Word16 psk, ps, ps0, ps1, ps2, sq, sq2;
  Word16 alpk, alp, alp_16;
  Word32 s, alp0, alp1, alp2;
  Word16 *p0, *p1, *p2, *p3, *p4;
  Word16 sign_dn[L_SUBFR], sign_dn_inv[L_SUBFR], *psign;
  Word16 tmp_vect[NB_POS];
  Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3, *rri4i4;
  Word16 *rri0i1, *rri0i2, *rri0i3, *rri0i4;
  Word16 *rri1i2, *rri1i3, *rri1i4;
  Word16 *rri2i3, *rri2i4;

  Word16  *ptr_rri0i3_i4;
  Word16  *ptr_rri1i3_i4;
  Word16  *ptr_rri2i3_i4;
  Word16  *ptr_rri3i3_i4;

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
       sign_dn[i] = MAX_16;
       sign_dn_inv[i] = MIN_16;
     }
     else
     {
       sign_dn[i] = MIN_16;
       sign_dn_inv[i] = MAX_16;
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

    for(i1=1; i1<L_SUBFR; i1+=STEP)
    {
      *p0 = mult(*p0, psign[i1]);   p0++;
      *p1 = mult(*p1, psign[i1+1]); p1++;
      *p2 = mult(*p2, psign[i1+2]); p2++;
      *p3 = mult(*p3, psign[i1+3]); p3++;
    }
  }

  p0 = rri1i2;
  p1 = rri1i3;
  p2 = rri1i4;

  for(i1=1; i1<L_SUBFR; i1+=STEP)
  {
    psign = sign_dn;
    if (psign[i1] < 0) psign = sign_dn_inv;

    for(i2=2; i2<L_SUBFR; i2+=STEP)
    {
      *p0 = mult(*p0, psign[i2]);   p0++;
      *p1 = mult(*p1, psign[i2+1]); p1++;
      *p2 = mult(*p2, psign[i2+2]); p2++;
    }
  }

  p0 = rri2i3;
  p1 = rri2i4;

  for(i2=2; i2<L_SUBFR; i2+=STEP)
  {
    psign = sign_dn;
    if (psign[i2] < 0) psign = sign_dn_inv;

    for(i3=3; i3<L_SUBFR; i3+=STEP)
    {
      *p0 = mult(*p0, psign[i3]);   p0++;
      *p1 = mult(*p1, psign[i3+1]); p1++;
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
        ps2 = add(ps1, dn[i1]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
        alp2 = L_mac(alp1, *p0++, _1_2);
        alp2 = L_mac(alp2, *p1++, _1_4);

        sq2 = mult(ps2, ps2);
        alp_16 = g_round(alp2);

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
      *p3++ = g_round(s);
    }

    /* i2 loop: 8 positions in track 0 */

    p0 = rri0i2 + mult(i0, 6554);
    p1 = ptr_rri0i3_i4 + mult(i1, 6554);
    p2 = rri0i0;
    p3 = rri0i1;

    for (i2=0; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = add(ps0, dn[i2]);         /* index increment = STEP */

      /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */
      alp1 = L_mac(alp0, *p0, _1_8);       p0 += NB_POS;
      alp1 = L_mac(alp1, *p1, _1_8);       p1 += NB_POS;
      alp1 = L_mac(alp1, *p2++, _1_16);

      /* i3 loop: 8 positions in track 1 */

      p4 = tmp_vect;

      for (i3=1; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = add(ps1, dn[i3]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i3] + rr[i1][i3] + rr[i2][i3] + 1/2*rr[i3][i3]; */
        alp2 = L_mac(alp1, *p3++, _1_8);
        alp2 = L_mac(alp2, *p4++, _1_2);

        sq2 = mult(ps2, ps2);
        alp_16 = g_round(alp2);

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
        ps2 = add(ps1, dn[i1]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
        alp2 = L_mac(alp1, *p0, _1_2);       p0 += NB_POS;
        alp2 = L_mac(alp2, *p1++, _1_4);

        sq2 = mult(ps2, ps2);
        alp_16 = g_round(alp2);

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
      *p3++ = g_round(s);
    }

    /* i2 loop: 8 positions in track 1 */

    p0 = ptr_rri1i3_i4 + mult(i0, 6554);
    p1 = rri0i1 + i1_offset;
    p2 = rri1i1;
    p3 = rri1i2;

    for (i2=1; i2<L_SUBFR; i2+=STEP)
    {
      ps1 = add(ps0, dn[i2]);         /* index increment = STEP */

      /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */
      alp1 = L_mac(alp0, *p0, _1_8);       p0 += NB_POS;
      alp1 = L_mac(alp1, *p1++, _1_8);
      alp1 = L_mac(alp1, *p2++, _1_16);

      /* i3 loop: 8 positions in track 2 */

      p4 = tmp_vect;

      for (i3=2; i3<L_SUBFR; i3+=STEP)
      {
        ps2 = add(ps1, dn[i3]);       /* index increment = STEP */

        /* alp1 = alp0 + rr[i0][i3] + rr[i1][i3] + rr[i2][i3] + 1/2*rr[i3][i3]; */
        alp2 = L_mac(alp1, *p3++, _1_8);
        alp2 = L_mac(alp2, *p4++, _1_2);

        sq2 = mult(ps2, ps2);
        alp_16 = g_round(alp2);

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
 Set_zero(cod, L_SUBFR);

 cod[ip0] = shr(i0, 2);         /* From Q15 to Q13 */
 cod[ip1] = shr(i1, 2);
 cod[ip2] = shr(i2, 2);
 cod[ip3] = shr(i3, 2);

 /* find the filtered codeword */
 Set_zero(y, ip0);

 if(i0 > 0)
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = h[j];
 else
   for(i=ip0, j=0; i<L_SUBFR; i++, j++) y[i] = negate(h[j]);

 if(i1 > 0)
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], h[j]);
 else
   for(i=ip1, j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], h[j]);

 if(i2 > 0)
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], h[j]);
 else
   for(i=ip2, j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], h[j]);

 if(i3 > 0)
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = add(y[i], h[j]);
 else
   for(i=ip3, j=0; i<L_SUBFR; i++, j++) y[i] = sub(y[i], h[j]);

 /* find codebook index;  17-bit address */

 i = 0;
 if(i0 > 0) i = add(i, 1);
 if(i1 > 0) i = add(i, 2);
 if(i2 > 0) i = add(i, 4);
 if(i3 > 0) i = add(i, 8);
 *sign = i;

 ip0 = mult(ip0, 6554);         /* ip0/5 */
 ip1 = mult(ip1, 6554);         /* ip1/5 */
 ip2 = mult(ip2, 6554);         /* ip2/5 */
 i   = mult(ip3, 6554);         /* ip3/5 */
 j   = add(i, shl(i, 2));       /* j = i*5 */
 j   = sub(ip3, add(j, 3));     /* j= ip3%5 -3 */
 ip3 = add(shl(i, 1), j);

 i = add(ip0, shl(ip1, 3));
 i = add(i  , shl(ip2, 6));
 i = add(i  , shl(ip3, 9));

 return i;
}

/************** End of file acelp_ca.c **************************************/

/************** Begin of file bits.c ****************************************/
/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/

#if defined(CONTROL_OPT) && (CONTROL_OPT == 1)
/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);

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
static void prm2bits_ld8k(
 Word16   prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]            /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = 80 */
)
{
   Word16 i;
   *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
   *bits++ = SIZE_WORD;     /* bit[1], to be compatible with hardware      */

   for (i = 0; i < PRM_SIZE; i++)
     {
        int2bin(prm[i], bitsno[i], bits);
        bits += bitsno[i];
     }

   return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 Word16 value,             /* input : decimal value         */
 Word16 no_of_bits,        /* input : number of bits to use */
 Word16 *bitstream         /* output: bitstream             */
)
{
   Word16 *pt_bitstream;
   Word16   i, bit;

   pt_bitstream = bitstream + no_of_bits;

   for (i = 0; i < no_of_bits; i++)
   {
     bit = value & (Word16)0x0001;      /* get lsb */
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
static void bits2prm_ld8k(
 Word16 bits[],            /* input : serial bits (80)                       */
 Word16   prm[]            /* output: decoded parameters (11 parameters)     */
)
{
   Word16 i;
   for (i = 0; i < PRM_SIZE; i++)
     {
        prm[i] = bin2int(bitsno[i], bits);
        bits  += bitsno[i];
     }

}

/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static Word16 bin2int(       /* output: decimal value of bit pattern */
 Word16 no_of_bits,          /* input : number of bits to read       */
 Word16 *bitstream           /* input : array containing bits        */
)
{
   Word16   value, i;
   Word16 bit;

   value = 0;
   for (i = 0; i < no_of_bits; i++)
   {
     value <<= 1;
     bit = *bitstream++;
     if (bit == BIT_1)  value += 1;
   }
   return(value);
}
#else

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
static void prm2bits_ld8k(
                    Word16 prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
                    UWord8 *bits            /* output: serial bits (SERIAL_SIZE )*/
)
{
  PutBitContext pb;
  int i;

  init_put_bits(&pb, bits, 10);
  for (i = 0; i < PRM_SIZE; ++i)
    put_bits(&pb, bitsno[i], prm[i]);
  flush_put_bits(&pb);
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
static void bits2prm_ld8k(
                   UWord8  *bits,            /* input : serial bits (80)                       */
                   Word16   prm[]            /* output: decoded parameters (11 parameters)     */
)
{
  GetBitContext gb;
  int i;

  init_get_bits(&gb, bits, 10 /*buf_size*/);
  for (i = 0; i < PRM_SIZE; ++i)
    prm[i] = get_bits(&gb, bitsno[i]);
}
#endif
/************** End of file bits.c ******************************************/

/************** Begin of file cod_ld8a.c ************************************/

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a and Init_Coder_ld8a                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  Init_Coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  Coder_ld8a(Word16 ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  Init_Coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

static void Init_Coder_ld8a(g729a_encoder_state *state)
{

  /*----------------------------------------------------------------------*
  *      Initialize pointers to speech vector.                            *
  *                                                                       *
  *                                                                       *
  *   |--------------------|-------------|-------------|------------|     *
  *     previous speech           sf1           sf2         L_NEXT        *
  *                                                                       *
  *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
  *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
  *   |                   <-- present frame (L_FRAME) -->                 *
  * old_speech            |              <-- new speech (L_FRAME) -->     *
  * p_window              |              |                                *
  *                     speech           |                                *
  *                             new_speech                                *
  *-----------------------------------------------------------------------*/

  state->new_speech = state->old_speech + L_TOTAL - L_FRAME;         /* New speech     */
  state->speech     = state->new_speech - L_NEXT;                    /* Present frame  */
  state->p_window   = state->old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */

  /* Initialize static pointers */

  state->wsp    = state->old_wsp + PIT_MAX;
  state->exc    = state->old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */

  Set_zero(state->old_speech, L_TOTAL);
  Set_zero(state->old_exc, PIT_MAX+L_INTERPOL);
  Set_zero(state->old_wsp, PIT_MAX);
  Set_zero(state->mem_w,   M);
  Set_zero(state->mem_w0,  M);
  Set_zero(state->mem_zero, M);
  state->sharp = SHARPMIN;

  /* Initialize lsp_old_q[] */
  state->lsp_old[0] = 30000;
  state->lsp_old[1] = 26000;
  state->lsp_old[2] = 21000;
  state->lsp_old[3] = 15000;
  state->lsp_old[4] = 8000;
  state->lsp_old[5] = 0;
  state->lsp_old[6] = -8000;
  state->lsp_old[7] = -15000;
  state->lsp_old[8] = -21000;
  state->lsp_old[9] = -26000;

  Copy(state->lsp_old, state->lsp_old_q, M);
  Lsp_encw_reset(state);
  Init_exc_err(state);
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a                                          *
 *            ~~~~~~~~~~                                           *
 *  Coder_ld8a(Word16 ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

static void Coder_ld8a(
      g729a_encoder_state *state,
     Word16 ana[]       /* output  : Analysis parameters */
)
{

  /* LPC analysis */

  Word16 Aq_t[(MP1)*2];         /* A(z)   quantized for the 2 subframes */
  Word16 Ap_t[(MP1)*2];         /* A(z/gamma)       for the 2 subframes */
  Word16 *Aq, *Ap;              /* Pointer on Aq_t and Ap_t             */

  /* Other vectors */

  Word16 h1[L_SUBFR];            /* Impulse response h1[]              */
  Word16 xn[L_SUBFR];            /* Target vector for pitch search     */
  Word16 xn2[L_SUBFR];           /* Target vector for codebook search  */
  Word16 code[L_SUBFR];          /* Fixed codebook excitation          */
  Word16 y1[L_SUBFR];            /* Filtered adaptive excitation       */
  Word16 y2[L_SUBFR];            /* Filtered fixed codebook excitation */
  Word16 g_coeff[4];             /* Correlations between xn & y1       */

  Word16 g_coeff_cs[5];
  Word16 exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                     <y1,y1>, -2<xn,y1>,
                                          <y2,y2>, -2<xn,y2>, 2<y1,y2> */

  /* Scalars */

  Word16 i, j, k, i_subfr;
  Word16 T_op, T0, T0_min, T0_max, T0_frac;
  Word16 gain_pit, gain_code, index;
  Word16 temp, taming;
  Word32 L_temp;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
  {
     /* Temporary vectors */
    Word16 r_l[MP1], r_h[MP1];       /* Autocorrelations low and hi          */
    Word16 rc[M];                    /* Reflection coefficients.             */
    Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */

    /* LP analysis */

    Autocorr(state->p_window, M, r_h, r_l);              /* Autocorrelations */
    Lag_window(M, r_h, r_l);                      /* Lag windowing    */
    Levinson(r_h, r_l, Ap_t, rc);                 /* Levinson Durbin  */
    Az_lsp(Ap_t, lsp_new, state->lsp_old);               /* From A(z) to lsp */

    /* LSP quantization */

    Qua_lsp(state, lsp_new, lsp_new_q, ana);
    ana += 2;                         /* Advance analysis parameters pointer */

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all subframes                  *
     * The interpolated parameters are in array Aq_t[].                   *
     *--------------------------------------------------------------------*/

    Int_qlpc(state->lsp_old_q, lsp_new_q, Aq_t);

    /* Compute A(z/gamma) */

    Weight_Az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
    Weight_Az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);

    /* update the LSPs for the next frame */

    Copy(lsp_new,   state->lsp_old,   M);
    Copy(lsp_new_q, state->lsp_old_q, M);
  }

 /*----------------------------------------------------------------------*
  * - Find the weighted input speech w_sp[] for the whole speech frame   *
  * - Find the open-loop pitch delay                                     *
  *----------------------------------------------------------------------*/

  Residu(&Aq_t[0], &(state->speech[0]), &(state->exc[0]), L_SUBFR);
  Residu(&Aq_t[MP1], &(state->speech[L_SUBFR]), &(state->exc[L_SUBFR]), L_SUBFR);

  {
    Word16 Ap1[MP1];

    Ap = Ap_t;
    Ap1[0] = 4096;
    for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
       Ap1[i] = sub(Ap[i], mult(Ap[i-1], 22938));
    Syn_filt(Ap1, &(state->exc[0]), &(state->wsp[0]), L_SUBFR, state->mem_w, 1);

    Ap += MP1;
    for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
       Ap1[i] = sub(Ap[i], mult(Ap[i-1], 22938));
    Syn_filt(Ap1, &(state->exc[L_SUBFR]), &(state->wsp[L_SUBFR]), L_SUBFR, state->mem_w, 1);
  }

  /* Find open loop pitch lag */

  T_op = Pitch_ol_fast(state->wsp, PIT_MAX, L_FRAME);

  /* Range for closed loop pitch search in 1st subframe */

  T0_min = T_op - 3;
  T0_max = T0_min + 6;
  if (T0_min < PIT_MIN)
  {
    T0_min = PIT_MIN;
    T0_max = PIT_MIN + 6;
  }
  else if (T0_max > PIT_MAX)
  {
     T0_max = PIT_MAX;
     T0_min = PIT_MAX - 6;
  }

 /*------------------------------------------------------------------------*
  *          Loop for every subframe in the analysis frame                 *
  *------------------------------------------------------------------------*
  *  To find the pitch and innovation parameters. The subframe size is     *
  *  L_SUBFR and the loop is repeated 2 times.                             *
  *     - find the weighted LPC coefficients                               *
  *     - find the LPC residual signal res[]                               *
  *     - compute the target signal for pitch search                       *
  *     - compute impulse response of weighted synthesis filter (h1[])     *
  *     - find the closed-loop pitch parameters                            *
  *     - encode the pitch delay                                           *
  *     - find target vector for codebook search                           *
  *     - codebook search                                                  *
  *     - VQ of pitch and codebook gains                                   *
  *     - update states of weighting filter                                *
  *------------------------------------------------------------------------*/

  Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
  Ap = Ap_t;    /* pointer to weighted LPC coefficients             */

  for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    /*---------------------------------------------------------------*
     * Compute impulse response, h1[], of weighted synthesis filter  *
     *---------------------------------------------------------------*/

    h1[0] = 4096;
    Set_zero(&h1[1], L_SUBFR-1);
    Syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);

   /*----------------------------------------------------------------------*
    *  Find the target vector for pitch search:                            *
    *----------------------------------------------------------------------*/

    Syn_filt(Ap, &(state->exc[i_subfr]), xn, L_SUBFR, state->mem_w0, 0);

    /*---------------------------------------------------------------------*
     *                 Closed-loop fractional pitch search                 *
     *---------------------------------------------------------------------*/

    T0 = Pitch_fr3_fast(&(state->exc[i_subfr]), xn, h1, L_SUBFR, T0_min, T0_max,
                    i_subfr, &T0_frac);

    index = Enc_lag3(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);

    *ana++ = index;

    if (i_subfr == 0) {
      *ana++ = Parity_Pitch(index);
    }

   /*-----------------------------------------------------------------*
    *   - find filtered pitch exc                                     *
    *   - compute pitch gain and limit between 0 and 1.2              *
    *   - update target vector for codebook search                    *
    *-----------------------------------------------------------------*/

    Syn_filt(Ap, &(state->exc[i_subfr]), y1, L_SUBFR, state->mem_zero, 0);

    gain_pit = G_pitch(xn, y1, g_coeff, L_SUBFR);

    /* clip pitch gain if taming is necessary */

    taming = test_err(state, T0, T0_frac);

    if( taming == 1){
      if (gain_pit > GPCLIP) {
        gain_pit = GPCLIP;
      }
    }

    /* xn2[i]   = xn[i] - y1[i] * gain_pit  */

    for (i = 0; i < L_SUBFR; i++)
    {
      //L_temp = L_mult(y1[i], gain_pit);
      //L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
      L_temp = ((Word32)y1[i] * gain_pit) << 2;
      xn2[i] = sub(xn[i], extract_h(L_temp));
    }


   /*-----------------------------------------------------*
    * - Innovative codebook search.                       *
    *-----------------------------------------------------*/

    index = ACELP_Code_A(xn2, h1, T0, state->sharp, code, y2, &i);

    *ana++ = index;        /* Positions index */
    *ana++ = i;            /* Signs index     */


   /*-----------------------------------------------------*
    * - Quantization of gains.                            *
    *-----------------------------------------------------*/

    g_coeff_cs[0]     = g_coeff[0];            /* <y1,y1> */
    exp_g_coeff_cs[0] = negate(g_coeff[1]);    /* Q-Format:XXX -> JPN */
    g_coeff_cs[1]     = negate(g_coeff[2]);    /* (xn,y1) -> -2<xn,y1> */
    exp_g_coeff_cs[1] = negate(add(g_coeff[3], 1)); /* Q-Format:XXX -> JPN */

    Corr_xy2( xn, y1, y2, g_coeff_cs, exp_g_coeff_cs );  /* Q0 Q0 Q12 ^Qx ^Q0 */
                         /* g_coeff_cs[3]:exp_g_coeff_cs[3] = <y2,y2>   */
                         /* g_coeff_cs[4]:exp_g_coeff_cs[4] = -2<xn,y2> */
                         /* g_coeff_cs[5]:exp_g_coeff_cs[5] = 2<y1,y2>  */

    *ana++ = Qua_gain(code, g_coeff_cs, exp_g_coeff_cs,
                         L_SUBFR, &gain_pit, &gain_code, taming);


   /*------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pit  *
    *------------------------------------------------------------*/

    state->sharp = gain_pit;
    if (state->sharp > SHARPMAX)      { state->sharp = SHARPMAX;         }
    else if (state->sharp < SHARPMIN) { state->sharp = SHARPMIN;         }

   /*------------------------------------------------------*
    * - Find the total excitation                          *
    * - update filters memories for finding the target     *
    *   vector in the next subframe                        *
    *------------------------------------------------------*/

    for (i = 0; i < L_SUBFR;  i++)
    {
      /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
      /* exc[i]  in Q0   gain_pit in Q14               */
      /* code[i] in Q13  gain_cod in Q1                */

      //L_temp = L_mult(exc[i+i_subfr], gain_pit);
      //L_temp = L_mac(L_temp, code[i], gain_code);
      //L_temp = L_shl(L_temp, 1);
      L_temp = (Word32)(state->exc[i+i_subfr]) * (Word32)gain_pit +
               (Word32)code[i] * (Word32)gain_code;
      L_temp <<= 2;
      state->exc[i+i_subfr] = g_round(L_temp);
    }

    update_exc_err(state, gain_pit, T0);

    for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
    {
      temp       = ((Word32)y1[i] * (Word32)gain_pit)  >> 14;
      k          = ((Word32)y2[i] * (Word32)gain_code) >> 13;
      state->mem_w0[j]  = sub(xn[i], add(temp, k));
    }

    Aq += MP1;           /* interpolated LPC parameters for next subframe */
    Ap += MP1;

  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME:                 *
  *     speech[], wsp[] and  exc[]                   *
  *--------------------------------------------------*/

  Copy(&(state->old_speech[L_FRAME]), &(state->old_speech[0]), L_TOTAL-L_FRAME);
  Copy(&(state->old_wsp[L_FRAME]), &(state->old_wsp[0]), PIT_MAX);
  Copy(&(state->old_exc[L_FRAME]), &(state->old_exc[0]), PIT_MAX+L_INTERPOL);
}
/************** End of file cod_ld8a.c **************************************/

/************** Begin of file cor_func.c ************************************/
/* Functions Corr_xy2() and Cor_h_x()   */

/*---------------------------------------------------------------------------*
 * Function corr_xy2()                                                       *
 * ~~~~~~~~~~~~~~~~~~~                                                       *
 * Find the correlations between the target xn[], the filtered adaptive      *
 * codebook excitation y1[], and the filtered 1st codebook innovation y2[].  *
 *   g_coeff[2]:exp_g_coeff[2] = <y2,y2>                                     *
 *   g_coeff[3]:exp_g_coeff[3] = -2<xn,y2>                                   *
 *   g_coeff[4]:exp_g_coeff[4] = 2<y1,y2>                                    *
 *---------------------------------------------------------------------------*/

static void Corr_xy2(
      Word16 xn[],           /* (i) Q0  :Target vector.                  */
      Word16 y1[],           /* (i) Q0  :Adaptive codebook.              */
      Word16 y2[],           /* (i) Q12 :Filtered innovative vector.     */
      Word16 g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2 */
      Word16 exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]         */
)
{
      Word16   i,exp;

      Word32   scaled_y2; /* Q9 */
      Word32   L_accy2y2, L_accxny2, L_accy1y2;

      L_accy2y2 = L_accxny2 = L_accy1y2 = 0;
      for(i=0; i<L_SUBFR; i++)
      {
        // Scale down y2[] from Q12 to Q9 to avoid overflow
        scaled_y2 = y2[i] >> 3;
        // Compute scalar product <y2[],y2[]>
        L_accy2y2 += scaled_y2 * scaled_y2;
        // Compute scalar product <xn[],y2[]>
        L_accxny2 += (Word32)xn[i] * scaled_y2;
        // Compute scalar product <y1[],y2[]>
        L_accy1y2 += (Word32)y1[i] * scaled_y2;
      }
      L_accy2y2 <<= 1; L_accy2y2 +=1; /* Avoid case of all zeros */
      L_accxny2 <<= 1; L_accxny2 +=1;
      L_accy1y2 <<= 1; L_accy1y2 +=1;

      exp            = norm_l(L_accy2y2);
      g_coeff[2]     = g_round( L_accy2y2 << exp );
      exp_g_coeff[2] = exp + 3; //add(exp, 19-16);               /* Q[19+exp-16] */

      exp            = norm_l(L_accxny2);
      g_coeff[3]     = negate(g_round( L_accxny2 << exp ));
      exp_g_coeff[3] = sub(add(exp, 10-16), 1);                  /* Q[10+exp-16] */

      exp            = norm_l(L_accy1y2);
      g_coeff[4]     = g_round( L_accy1y2 << exp );
      exp_g_coeff[4] = sub(add(exp, 10-16), 1);                  /* Q[10+exp-16] */
}


/*--------------------------------------------------------------------------*
 *  Function  Cor_h_X()                                                     *
 *  ~~~~~~~~~~~~~~~~~~~                                                     *
 * Compute correlations of input response h[] with the target vector X[].   *
 *--------------------------------------------------------------------------*/

static void Cor_h_X(
     Word16 h[],        /* (i) Q12 :Impulse response of filters      */
     Word16 X[],        /* (i)     :Target vector                    */
     Word16 D[]         /* (o)     :Correlations between h[] and D[] */
                        /*          Normalized to 13 bits            */
)
{
   Word16 i, j;
   Word32 s, max;
   Word32 y32[L_SUBFR];

   /* first keep the result on 32 bits and find absolute maximum */

   max = 0;

   for (i = 0; i < L_SUBFR; i++)
   {
     s = 0;
     for (j = i; j <  L_SUBFR; j++)
       s += (Word32)X[j] * h[j-i];
     s <<= 1;
     y32[i] = s;

     if (s < 0) s = -s;
     if(s > max) max = s;
   }

   /* Find the number of right shifts to do on y32[]  */
   /* so that maximum is on 13 bits                   */

   j = norm_l(max);
   if( j > 16) {
    j = 16;
   }

   j = 18 - j;

   for(i=0; i<L_SUBFR; i++)
     D[i] = (Word16)(y32[i] >> j);
}
/************** End of file cor_func.c **************************************/

/************** Begin of file de_acelp.c ************************************/
/*-----------------------------------------------------------*
 *  Function  Decod_ACELP()                                  *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

static void Decod_ACELP(
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
/************** End of file de_acelp.c **************************************/

/************** Begin of file dec_gain.c ************************************/
/*---------------------------------------------------------------------------*
 * Function  Dec_gain                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Decode the pitch and codebook gains                                       *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * input arguments:                                                          *
 *                                                                           *
 *   index      :Quantization index                                          *
 *   code[]     :Innovative code vector                                      *
 *   L_subfr    :Subframe size                                               *
 *   bfi        :Bad frame indicator                                         *
 *                                                                           *
 * output arguments:                                                         *
 *                                                                           *
 *   gain_pit   :Quantized pitch gain                                        *
 *   gain_cod   :Quantized codebook gain                                     *
 *                                                                           *
 *---------------------------------------------------------------------------*/
static void Dec_gain(
   Word16 index,        /* (i)     :Index of quantization.         */
   Word16 code[],       /* (i) Q13 :Innovative vector.             */
   Word16 L_subfr,      /* (i)     :Subframe length.               */
   Word16 bfi,          /* (i)     :Bad frame indicator            */
   Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
   Word16 *gain_cod     /* (o) Q1  :Code gain.                     */
)
{
   Word16  index1, index2, tmp;
   Word16  gcode0, exp_gcode0;
   Word32  L_gbk12, L_acc;
   void    Gain_predict( Word16 past_qua_en[], Word16 code[], Word16 L_subfr,
                        Word16 *gcode0, Word16 *exp_gcode0 );
   void    Gain_update( Word16 past_qua_en[], Word32 L_gbk12 );
   void    Gain_update_erasure( Word16 past_qua_en[] );

        /* Gain predictor, Past quantized energies = -14.0 in Q10 */

   static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };


   /*-------------- Case of erasure. ---------------*/

   if(bfi != 0){
      *gain_pit = (Word16)((Word32)*gain_pit * 29491L >> 15);      /* *0.9 in Q15 */
      if (*gain_pit > 29491) *gain_pit = 29491;
      *gain_cod = (Word16)((Word32)*gain_cod * 32111L >> 15);      /* *0.98 in Q15 */

     /*----------------------------------------------*
      * update table of past quantized energies      *
      *                              (frame erasure) *
      *----------------------------------------------*/
      Gain_update_erasure(past_qua_en);

      return;
   }

   /*-------------- Decode pitch gain ---------------*/

   index1 = imap1[ index >> NCODE2_B ] ;
   index2 = imap2[ index & (NCODE2-1) ] ;
   *gain_pit = gbk1[index1][0] + gbk2[index2][0];

   /*-------------- Decode codebook gain ---------------*/

  /*---------------------------------------------------*
   *-  energy due to innovation                       -*
   *-  predicted energy                               -*
   *-  predicted codebook gain => gcode0[exp_gcode0]  -*
   *---------------------------------------------------*/

   Gain_predict( past_qua_en, code, L_subfr, &gcode0, &exp_gcode0 );

  /*-----------------------------------------------------------------*
   * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
   *-----------------------------------------------------------------*/

   L_gbk12 = (Word32)gbk1[index1][1] + (Word32)gbk2[index2][1]; /* Q13 */
   tmp = (Word16)(L_gbk12 >> 1);  /* Q12 */
   L_acc = tmp * gcode0 << 1;             /* Q[exp_gcode0+12+1] */

   L_acc = L_shl(L_acc, add( negate(exp_gcode0),(-12-1+1+16) ));
   *gain_cod = (Word16)(L_acc >> 16);                          /* Q1 */

  /*----------------------------------------------*
   * update table of past quantized energies      *
   *----------------------------------------------*/
   Gain_update( past_qua_en, L_gbk12 );
}
/************** End of file dec_gain.c **************************************/

/************** Begin of file dec_lag3.c ************************************/
/*------------------------------------------------------------------------*
 *    Function Dec_lag3                                                   *
 *             ~~~~~~~~                                                   *
 *   Decoding of fractional pitch lag with 1/3 resolution.                *
 * See "Enc_lag3.c" for more details about the encoding procedure.        *
 *------------------------------------------------------------------------*/
static void Dec_lag3(
  Word16 index,       /* input : received pitch index           */
  Word16 pit_min,     /* input : minimum pitch lag              */
  Word16 pit_max,     /* input : maximum pitch lag              */
  Word16 i_subfr,     /* input : subframe flag                  */
  Word16 *T0,         /* output: integer part of pitch lag      */
  Word16 *T0_frac     /* output: fractional part of pitch lag   */
)
{
  Word16 i;
  Word16 T0_min, T0_max;

  if (i_subfr == 0)                  /* if 1st subframe */
  {
    if (index<  197)
    {
      /* *T0 = (index+2)/3 + 19 */
      *T0 = ((Word32)(index+2) * 10923) >> 15;
      *T0 += 19;

      /* *T0_frac = index - *T0*3 + 58 */
      i = *T0 + (*T0 << 1);
      *T0_frac = index - i + 58;
    }
    else
    {
      *T0 = index - 112;
      *T0_frac = 0;
    }

  }

  else  /* second subframe */
  {
    /* find T0_min and T0_max for 2nd subframe */
    T0_min = *T0 - 5;
    if (T0_min < pit_min)
    {
      T0_min = pit_min;
    }

    T0_max = T0_min + 9;
    if (T0_max > pit_max)
    {
      T0_max = pit_max;
      T0_min = T0_max - 9;
    }

    /* i = (index+2)/3 - 1 */
    /* *T0 = i + t0_min;    */
    i = ((Word32)(index + 2) * 10923LL) >> 15;
    i -= 1;
    *T0 = i + T0_min;

    /* t0_frac = index - 2 - i*3; */
    i = i + (i << 1);
    *T0_frac = index - 2 - i;
  }
}
/************** End of file dec_lag3.c **************************************/

/************** Begin of file dec_ld8a.c ************************************/
/*-----------------------------------------------------------------*
 *   Functions Init_Decod_ld8a  and Decod_ld8a                     *
 *-----------------------------------------------------------------*/

/*---------------------------------------------------------------*
 *   Decoder constant parameters (defined in "ld8a.h")           *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MIN     : Minimum pitch lag.                            *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation            *
 *   PRM_SIZE    : Size of vector containing analysis parameters *
 *---------------------------------------------------------------*/

/*-----------------------------------------------------------------*
 *   Function Init_Decod_ld8a                                      *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *   ->Initialization of variables for the decoder section.        *
 *                                                                 *
 *-----------------------------------------------------------------*/
static void Init_Decod_ld8a(g729a_decoder_state *state)
{

  /* Initialize static pointer */
  state->exc = state->old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */
  Set_zero(state->old_exc, PIT_MAX+L_INTERPOL);
  Set_zero(state->mem_syn, M);

  state->lsp_old[0] = 30000;
  state->lsp_old[1] = 26000;
  state->lsp_old[2] = 21000;
  state->lsp_old[3] = 15000;
  state->lsp_old[4] = 8000;
  state->lsp_old[5] = 0;
  state->lsp_old[6] = -8000;
  state->lsp_old[7] = -15000;
  state->lsp_old[8] = -21000;
  state->lsp_old[9] = -26000;

  state->sharp  = SHARPMIN;
  state->old_T0 = 60;
  state->gain_code = 0;
  state->gain_pitch = 0;

  Lsp_decw_reset(state);
}

/*-----------------------------------------------------------------*
 *   Function Decod_ld8a                                           *
 *           ~~~~~~~~~~                                            *
 *   ->Main decoder routine.                                       *
 *                                                                 *
 *-----------------------------------------------------------------*/

static void Decod_ld8a(
  g729a_decoder_state *state,
  Word16  parm[],      /* (i)   : vector of synthesis parameters
                                  parm[0] = bad frame indicator (bfi)  */
  Word16  synth[],     /* (o)   : synthesis speech                     */
  Word16  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
  Word16  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
  Word16 bad_lsf       /* (i)   : bad LSF indicator                    */
)
{
  Word16  *Az;                  /* Pointer on A_t   */
  Word16  lsp_new[M];           /* LSPs             */
  Word16  code[L_SUBFR];        /* ACELP codevector */

  /* Scalars */

  Word16  i, j, i_subfr;
  Word16  T0, T0_frac, index;
  Word16  bfi;
  Word32  L_temp, L_temp1;

  Word16 bad_pitch;             /* bad pitch indicator */

  /* Test bad frame indicator (bfi) */

  bfi = *parm++;

  /* Decode the LSPs */

  D_lsp(state, parm, lsp_new, add(bfi, bad_lsf));
  parm += 2;

  /*
  Note: "bad_lsf" is introduce in case the standard is used with
         channel protection.
  */

  /* Interpolation of LPC for the 2 subframes */

  Int_qlpc(state->lsp_old, lsp_new, A_t);

  /* update the LSFs for the next frame */

  Copy(lsp_new, state->lsp_old, M);

/*------------------------------------------------------------------------*
 *          Loop for every subframe in the analysis frame                 *
 *------------------------------------------------------------------------*
 * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR  *
 *  times                                                                 *
 *     - decode the pitch delay                                           *
 *     - decode algebraic code                                            *
 *     - decode pitch and codebook gains                                  *
 *     - find the excitation and compute synthesis speech                 *
 *------------------------------------------------------------------------*/

  Az = A_t;            /* pointer to interpolated LPC parameters */

  for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    index = *parm++;            /* pitch index */

    if(i_subfr == 0)
      bad_pitch = bfi + *parm++; /* get parity check result */
    else
      bad_pitch = bfi;
      if( bad_pitch == 0)
      {
        Dec_lag3(index, PIT_MIN, PIT_MAX, i_subfr, &T0, &T0_frac);
        state->old_T0 = T0;
      }
      else        /* Bad frame, or parity error */
      {
        T0  =  state->old_T0++;
          T0_frac = 0;
        if( state->old_T0 > PIT_MAX)
          state->old_T0 = PIT_MAX;
      }
    *T2++ = T0;

   /*-------------------------------------------------*
    * - Find the adaptive codebook vector.            *
    *-------------------------------------------------*/

    Pred_lt_3(&(state->exc[i_subfr]), T0, T0_frac, L_SUBFR);

   /*-------------------------------------------------------*
    * - Decode innovative codebook.                         *
    * - Add the fixed-gain pitch contribution to code[].    *
    *-------------------------------------------------------*/

    if(bfi != 0)        /* Bad frame */
    {

      parm[0] = Random() & (Word16)0x1fff;     /* 13 bits random */
      parm[1] = Random() & (Word16)0x000f;     /*  4 bits random */
    }
    Decod_ACELP(parm[1], parm[0], code);
    parm +=2;

    j = shl(state->sharp, 1);          /* From Q14 to Q15 */
    if(T0 < L_SUBFR ) {
        for (i = T0; i < L_SUBFR; i++) {
          //code[i] = add(code[i], mult(code[i-T0], j));
          code[i] += ((Word32)code[i-T0] * (Word32)j) >> 15;
        }
    }

   /*-------------------------------------------------*
    * - Decode pitch and codebook gains.              *
    *-------------------------------------------------*/

    index = *parm++;      /* index of energy VQ */

    Dec_gain(index, code, L_SUBFR, bfi, &(state->gain_pitch), &(state->gain_code));

   /*-------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pitch *
    *-------------------------------------------------------------*/

    state->sharp = state->gain_pitch;
    if (state->sharp > SHARPMAX) { state->sharp = SHARPMAX;  }
    if (state->sharp < SHARPMIN) { state->sharp = SHARPMIN;  }

   /*-------------------------------------------------------*
    * - Find the total excitation.                          *
    * - Find synthesis speech corresponding to exc[].       *
    *-------------------------------------------------------*/

    for (i = 0; i < L_SUBFR;  i++)
    {
       /* exc[i] = gain_pitch*exc[i] + gain_code*code[i]; */
       /* exc[i]  in Q0   gain_pitch in Q14               */
       /* code[i] in Q13  gain_codeode in Q1              */

       L_temp1 = (state->exc[i+i_subfr] * state->gain_pitch + code[i] * state->gain_code);
       L_temp = L_temp1 << 2;
       if (L_temp >> 2 != L_temp1)
         state->exc[i+i_subfr] = MIN_16; // FIXME
       else
         state->exc[i+i_subfr] = (Word16)((L_temp + 0x8000) >> 16);
    }

//    Syn_filt(Az, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, 0);
    if (Syn_filt_overflow(Az, &(state->exc[i_subfr]), &synth[i_subfr], L_SUBFR,
                          state->mem_syn) != 0)
    {
      /* In case of overflow in the synthesis          */
      /* -> Scale down vector exc[] and redo synthesis */

      for(i=0; i<PIT_MAX+L_INTERPOL+L_FRAME; i++)
        state->old_exc[i] >>= 2;

      Syn_filt(Az, &(state->exc[i_subfr]), &synth[i_subfr], L_SUBFR,
               state->mem_syn, 1);
    }
    else
      Copy(&synth[i_subfr+L_SUBFR-M], state->mem_syn, M);

    Az += MP1;    /* interpolated LPC parameters for next subframe */
  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME  exc[]           *
  *--------------------------------------------------*/

  Copy(&(state->old_exc[L_FRAME]), &(state->old_exc[0]), PIT_MAX+L_INTERPOL);
}
/************** End of file dec_ld8a.c **************************************/

/************** Begin of file dspfunc.c *************************************/
/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : Pow2()                                                  |
 |                                                                           |
 |     L_x = pow(2.0, exponent.fraction)                                     |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The function Pow2(L_x) is approximated by a table and linear            |
 |   interpolation.                                                          |
 |                                                                           |
 |   1- i = bit10-b15 of fraction,   0 <= i <= 31                            |
 |   2- a = bit0-b9   of fraction                                            |
 |   3- L_x = tabpow[i]<<16 - (tabpow[i] - tabpow[i+1]) * a * 2                 |
 |   4- L_x = L_x >> (30-exponent)     (with rounding)                       |
 |___________________________________________________________________________|
*/
static Word32 Pow2(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  Word16 fraction   /* (i) Q15 : Fractional part.   (range: 0.0<=val<1.0) */
)
{
  Word16 exp, i, a, tmp;
  Word32 L_x;

  L_x = fraction<<6;
  /* Extract b0-b16 of fraction */

  i = ((Word16)(L_x >> 16)) & 31;             /* Extract b10-b15 of fraction */
  a = (Word16)((L_x >> 1) & 0x7fff);          /* Extract b0-b9   of fraction */

  L_x = ((Word32) tabpow[i] << 16);             /* tabpow[i] << 16       */

  tmp = tabpow[i] - tabpow[i + 1];
  L_x -= (((Word32) tmp) * a) << 1;  /* L_x -= tmp*a*2        */

  exp = 30 - exponent;
  L_x = L_x + ((Word32) 1 << (exp-1));
  L_x = (Word16)(L_x >> exp);

  return(L_x);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : Log2()                                                  |
 |                                                                           |
 |       Compute log2(L_x).                                                  |
 |       L_x is positive.                                                    |
 |                                                                           |
 |       if L_x is negative or zero, result is 0.                            |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The function Log2(L_x) is approximated by a table and linear            |
 |   interpolation.                                                          |
 |                                                                           |
 |   1- Normalization of L_x.                                                |
 |   2- exponent = 30-exponent                                               |
 |   3- i = bit25-b31 of L_x,    32 <= i <= 63  ->because of normalization.  |
 |   4- a = bit10-b24                                                        |
 |   5- i -=32                                                               |
 |   6- fraction = tablog[i]<<16 - (tablog[i] - tablog[i+1]) * a * 2            |
 |___________________________________________________________________________|
*/
static void Log2(
  Word32 L_x,       /* (i) Q0 : input value                                 */
  Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  Word16 *fraction  /* (o) Q15: Fractional  part of Log2. (range: 0<=val<1) */
)
{
  Word16 exp, i, a, tmp;
  Word32 L_y;

  if( L_x <= (Word32)0 )
  {
    *exponent = 0;
    *fraction = 0;
    return;
  }

  exp = norm_l(L_x);
  L_x <<= exp;               /* L_x is normalized */

  /* Calculate exponent portion of Log2 */
  *exponent = 30 - exp;

  /* At this point, L_x > 0       */
  /* Shift L_x to the right by 10 to extract bits 10-31,  */
  /* which is needed to calculate fractional part of Log2 */
  L_x >>= 10;
  i = (Word16)(L_x >> 15);    /* Extract b25-b31 */
  a = L_x & 0x7fff;           /* Extract b10-b24 of fraction */

  /* Calculate table index -> subtract by 32 is done for           */
  /* proper table indexing, since 32<=i<=63 (due to normalization) */
  i -= 32;

  /* Fraction part of Log2 is approximated by using table[]    */
  /* and linear interpolation, i.e.,                           */
  /* fraction = table[i]<<16 - (table[i] - table[i+1]) * a * 2 */
  L_y = (Word32) tablog[i] << 16;  /* table[i] << 16        */
  tmp = tablog[i] - tablog[i + 1];  /* table[i] - table[i+1] */
  L_y -= (((Word32) tmp) * a) << 1; /* L_y -= tmp*a*2        */

  *fraction = (Word16)(L_y >> 16);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : Inv_sqrt                                                |
 |                                                                           |
 |       Compute 1/sqrt(L_x).                                                |
 |       L_x is positive.                                                    |
 |                                                                           |
 |       if L_x is negative or zero, result is 1 (3fff ffff).                |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The function 1/sqrt(L_x) is approximated by a table and linear          |
 |   interpolation.                                                          |
 |                                                                           |
 |   1- Normalization of L_x.                                                |
 |   2- If (30-exponent) is even then shift right once.                      |
 |   3- exponent = (30-exponent)/2  +1                                       |
 |   4- i = bit25-b31 of L_x,    16 <= i <= 63  ->because of normalization.  |
 |   5- a = bit10-b24                                                        |
 |   6- i -=16                                                               |
 |   7- L_y = tabsqr[i]<<16 - (tabsqr[i] - tabsqr[i+1]) * a * 2                 |
 |   8- L_y >>= exponent                                                     |
 |___________________________________________________________________________|
*/
static Word32 Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
)
{
  Word16 exp, i, a, tmp;
  Word32 L_y;

  if( L_x <= (Word32)0) return ( (Word32)0x3fffffffL);

  exp = norm_l(L_x);
  L_x <<= exp;               /* L_x is normalize */

  exp = 30 - exp;
  //if( (exp & 1) == 0 )                  /* If exponent even -> shift right */
      //L_x >>= 1;

  L_x >>= (10 - (exp & 1));

  exp >>= 1;
  exp += 1;

  //L_x >>= 9;
  i = (Word16)(L_x >> 16);        /* Extract b25-b31 */
  a = (Word16)(L_x >> 1);         /* Extract b10-b24 */
  a &= (Word16) 0x7fff;

  i   -= 16;

  L_y = (Word32)tabsqr[i] << 16;    /* inv_sqrt_tbl[i] << 16    */
  tmp =  tabsqr[i] - tabsqr[i + 1];
  L_y -= ((Word32)tmp * a) << 1;        /* L_y -=  tmp*a*2         */

  L_y >>= exp;                /* denormalization */

  return(L_y);
}
/************** End of file dspfunc.c ***************************************/

/************** Begin of file filter.c **************************************/
/*-----------------------------------------------------*
 * procedure Syn_filt:                                 *
 *           ~~~~~~~~                                  *
 * Do the synthesis filtering 1/A(z).                  *
 *-----------------------------------------------------*/

/* ff_celp_lp_synthesis_filter */
static Flag Syn_filt_overflow(
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[]    /* (i)     : memory associated with this filtering.   */
)
{
  Word16 i, j;
  Word32 s, t;
  Word16 tmp[100];     /* This is usually done by memory allocation (lg+M) */
  Word16 *yy;

  /* Copy mem[] to yy[] */

  yy = tmp;

  Copy(mem, yy, M);
  yy += M;

  /* Do the filtering. */
  for (i = 0; i < lg; i++)
  {
    s = x[i] * a[0];
    for (j = 1; j <= M; j++)
      s -= a[j] * yy[-j];

    t = s << 4;
    if (t >> 4 != s)
    {
      *yy++ = s & MIN_32 ? MIN_16 : MAX_16;
      return 1;
    }
    else
      *yy++ = (t + 0x8000) >> 16;
  }

  Copy(&tmp[M], y, lg);

  return 0;
}


/* ff_celp_lp_synthesis_filter */
static void Syn_filt(
  Word16 a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  Word16 x[],     /* (i)     : input signal                             */
  Word16 y[],     /* (o)     : output signal                            */
  Word16 lg,      /* (i)     : size of filtering                        */
  Word16 mem[],   /* (i/o)   : memory associated with this filtering.   */
  Word16 update   /* (i)     : 0=no update, 1=update of memory.         */
)
{
  Word16 i, j;
  Word32 s, t;
  Word16 tmp[100];     /* This is usually done by memory allocation (lg+M) */
  Word16 *yy;

  /* Copy mem[] to yy[] */
  yy = tmp;
  Copy(mem, yy, M);
  yy += M;

  /* Do the filtering. */
  for (i = 0; i < lg; i++)
  {
    s = x[i] * a[0];
    for (j = 1; j <= M; j++)
      s -= a[j] * yy[-j];

    t = s << 4;
    if (t >> 4 != s)
    	*yy++ = s & MIN_32 ? MIN_16 : MAX_16;
    else
   		*yy++ = (t + 0x8000) >> 16;
  }

  Copy(&tmp[M], y, lg);

  /* Update of memory if update==1 */

  if(update)
     Copy(&y[lg-M], mem, M);
}

/*-----------------------------------------------------------------------*
 * procedure Residu:                                                     *
 *           ~~~~~~                                                      *
 * Compute the LPC residual  by filtering the input speech through A(z)  *
 *-----------------------------------------------------------------------*/

static void Residu(
  Word16 a[],    /* (i) Q12 : prediction coefficients                     */
  Word16 x[],    /* (i)     : speech (values x[-m..-1] are needed         */
  Word16 y[],    /* (o)     : residual signal                             */
  Word16 lg      /* (i)     : size of filtering                           */
)
{
  Word16 i, j;
  Word32 s;

  for (i = 0; i < lg; i++)
  {
    s = x[i] * a[0];
    for (j = 1; j <= M; j++)
      s += a[j] * x[i-j];

    y[i] = (s + 0x800) >> 12;
  }
}
/************** End of file filter.c ****************************************/

/************** Begin of file gainpred.c ************************************/
/*---------------------------------------------------------------------------*
 *  Gain_predict()        : make gcode0(exp_gcode0)                          *
 *  Gain_update()         : update table of past quantized energies.         *
 *  Gain_update_erasure() : update table of past quantized energies.         *
 *                                                        (frame erasure)    *
 *    This function is used both Coder and Decoder.                          *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Function  Gain_predict                                                    *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * MA prediction is performed on the innovation energy (in dB with mean      *
 * removed).                                                                 *
 *---------------------------------------------------------------------------*/
static void Gain_predict(
   Word16 past_qua_en[], /* (i) Q10 :Past quantized energies        */
   Word16 code[],        /* (i) Q13 :Innovative vector.             */
   Word16 L_subfr,       /* (i)     :Subframe length.               */
   Word16 *gcode0,       /* (o) Qxx :Predicted codebook gain        */
   Word16 *exp_gcode0    /* (o)     :Q-Format(gcode0)               */
)
{
   Word16  i, exp, frac;
   Word32  L_tmp;

  /*-------------------------------*
   * Energy coming from code       *
   *-------------------------------*/

   L_tmp = 0;
   for(i=0; i<L_subfr; i++)
     L_tmp += code[i] * code[i];
   L_tmp <<= 1;

  /*-----------------------------------------------------------------*
   *  Compute: means_ener - 10log10(ener_code/ L_sufr)               *
   *  Note: mean_ener change from 36 dB to 30 dB because input/2     *
   *                                                                 *
   * = 30.0 - 10 log10( ener_code / lcode)  + 10log10(2^27)          *
   *                                          !!ener_code in Q27!!   *
   * = 30.0 - 3.0103 * log2(ener_code) + 10log10(40) + 10log10(2^27) *
   * = 30.0 - 3.0103 * log2(ener_code) + 16.02  + 81.278             *
   * = 127.298 - 3.0103 * log2(ener_code)                            *
   *-----------------------------------------------------------------*/

   Log2(L_tmp, &exp, &frac);               /* Q27->Q0 ^Q0 ^Q15       */
   //L_tmp = Mpy_32_16(exp, frac, -24660);
                                           /* Q0 Q15 Q13 -> ^Q14     */
                                           /* hi:Q0+Q13+1            */
                                           /* lo:Q15+Q13-15+1        */
                                           /* -24660[Q13]=-3.0103    */
   L_tmp = (Word32)exp * -24660LL;
   L_tmp += ((Word16)((Word32)frac * -24660LL >> 15));
   L_tmp <<= 1;
   //L_tmp = L_mac(L_tmp, 32588, 32);        /* 32588*32[Q14]=127.298  */
   L_tmp += 2085632; //32588 * 32 *2;

  /*-----------------------------------------------------------------*
   * Compute gcode0.                                                 *
   *  = Sum(i=0,3) pred[i]*past_qua_en[i] - ener_code + mean_ener    *
   *-----------------------------------------------------------------*/

   L_tmp <<= 10;                      /* From Q14 to Q24 */
   for(i=0; i<4; i++)
     L_tmp += pred[i] * past_qua_en[i] << 1; /* Q13*Q10 ->Q24 */

   *gcode0 = L_tmp >> 16;                    /* From Q24 to Q8  */

  /*-----------------------------------------------------------------*
   * gcode0 = pow(10.0, gcode0/20)                                   *
   *        = pow(2, 3.3219*gcode0/20)                               *
   *        = pow(2, 0.166*gcode0)                                   *
   *-----------------------------------------------------------------*/

   //L_tmp = *gcode0 * 5439 << 1;       /* *0.166 in Q15, result in Q24*/
   //L_tmp = L_tmp >> 8;             /* From Q24 to Q16              */
   L_tmp = *gcode0 * 5439 >> 7;
   //L_Extract(L_tmp, &exp, &frac);       /* Extract exponent of gcode0  */
   exp  = (Word16)(L_tmp >> 16);
   frac = (Word16)((L_tmp >> 1) - (exp << 15));

   *gcode0 = (Word16)Pow2(14, frac); /* Put 14 as exponent so that  */
                                        /* output of Pow2() will be:   */
                                        /* 16768 < Pow2() <= 32767     */
   *exp_gcode0 = 14 - exp;
}


/*---------------------------------------------------------------------------*
 * Function  Gain_update                                                     *
 * ~~~~~~~~~~~~~~~~~~~~~~                                                    *
 * update table of past quantized energies                                   *
 *---------------------------------------------------------------------------*/
static void Gain_update(
   Word16 past_qua_en[],   /* (io) Q10 :Past quantized energies           */
   Word32  L_gbk12         /* (i) Q13 : gbk1[indice1][1]+gbk2[indice2][1] */
)
{
   Word16  i, tmp;
   Word16  exp, frac;
   Word32  L_acc;

   for(i=3; i>0; i--){
      past_qua_en[i] = past_qua_en[i-1];         /* Q10 */
   }
  /*----------------------------------------------------------------------*
   * -- past_qua_en[0] = 20*log10(gbk1[index1][1]+gbk2[index2][1]); --    *
   *    2 * 10 log10( gbk1[index1][1]+gbk2[index2][1] )                   *
   *  = 2 * 3.0103 log2( gbk1[index1][1]+gbk2[index2][1] )                *
   *  = 2 * 3.0103 log2( gbk1[index1][1]+gbk2[index2][1] )                *
   *                                                 24660:Q12(6.0205)    *
   *----------------------------------------------------------------------*/

   Log2( L_gbk12, &exp, &frac );               /* L_gbk12:Q13       */
   L_acc = (((Word32)(exp - 13) << 16) + ((Word32)(frac) << 1)); /* L_acc:Q16 */
   tmp = extract_h( L_shl( L_acc,13 ) );       /* tmp:Q13           */
   past_qua_en[0] = mult( tmp, 24660 );        /* past_qua_en[]:Q10 */
}


/*---------------------------------------------------------------------------*
 * Function  Gain_update_erasure                                             *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                             *
 * update table of past quantized energies (frame erasure)                   *
 *---------------------------------------------------------------------------*
 *     av_pred_en = 0.0;                                                     *
 *     for (i = 0; i < 4; i++)                                               *
 *        av_pred_en += past_qua_en[i];                                      *
 *     av_pred_en = av_pred_en*0.25 - 4.0;                                   *
 *     if (av_pred_en < -14.0) av_pred_en = -14.0;                           *
 *---------------------------------------------------------------------------*/
static void Gain_update_erasure(
   Word16 past_qua_en[]     /* (i) Q10 :Past quantized energies        */
)
{
   Word16  i, av_pred_en;
   Word32  L_tmp;

   L_tmp = 0;                                                     /* Q10 */
   for(i=0; i<4; i++)
      L_tmp += (Word32)past_qua_en[i];
   av_pred_en = (Word16)(L_tmp >> 2) - 4096;

   if( av_pred_en < -14336 ){
      av_pred_en = -14336;                              /* 14336:14[Q10] */
   }

   for(i=3; i>0; i--)
      past_qua_en[i] = past_qua_en[i-1];

   past_qua_en[0] = av_pred_en;
}
/************** End of file gainpred.c **************************************/

/************** Begin of file lpc.c *****************************************/
/*-----------------------------------------------------*
 * Function Autocorr()                                 *
 *                                                     *
 *   Compute autocorrelations of signal with windowing *
 *                                                     *
 *-----------------------------------------------------*/

static void Autocorr(
  Word16 x[],      /* (i)    : Input signal                      */
  Word16 m,        /* (i)    : LPC order                         */
  Word16 r_h[],    /* (o)    : Autocorrelations  (msb)           */
  Word16 r_l[]     /* (o)    : Autocorrelations  (lsb)           */
)
{
  Word16 i, j, norm;
  Word16 y[L_WINDOW];
  Word32 sum;

  /* Windowing of signal */
  sum = 0;
  for(i=0; i<L_WINDOW; i++)
  {
    y[i] = (Word16)(((Word32)x[i] * (Word32)hamwindow[i] + 0x4000) >> 15);
    sum += ((Word32)y[i] * (Word32)y[i]) << 1;
    if (sum < 0) // overflow
      break;
  }

  if (i != L_WINDOW) // overflow
  {
    for (; i<L_WINDOW; i++)
      y[i] = (Word16)(((Word32)x[i] * (Word32)hamwindow[i] + 0x4000) >> 15);

    /* Compute r[0] and test for overflow */
    while (1)
    {
      /* If overflow divide y[] by 4 */
      sum = 0;
      for(i=0; i<L_WINDOW; i++)
      {
        y[i] >>= 2;
        sum += ((Word32)y[i] * (Word32)y[i]);
      }
      sum <<= 1;
      sum += 1; /* Avoid case of all zeros */
      if (sum > 0)
        break;
    }
  }
  else
    sum += 1; /* Avoid case of all zeros */

  /* Normalization of r[0] */
  norm = norm_l(sum);
  sum  <<= norm;

  /* Put in DPF format (see oper_32b) */
  r_h[0] = (Word16)(sum >> 16);
  r_l[0] = (Word16)((sum >> 1) - ((Word32)r_h[0] << 15));

  /* r[1] to r[m] */

  for (i = 1; i <= m; i++)
  {
    sum = 0;
    for(j=0; j<L_WINDOW-i; j++)
      sum += (Word32)y[j] * (Word32)y[j+i];

    sum <<= norm + 1;
    r_h[i] = (Word16)(sum >> 16);
    r_l[i] = (Word16)((sum >> 1) - ((Word32)r_h[i] << 15));
  }
}

/*-------------------------------------------------------*
 * Function Lag_window()                                 *
 *                                                       *
 * Lag_window on autocorrelations.                       *
 *                                                       *
 * r[i] *= lag_wind[i]                                   *
 *                                                       *
 *  r[i] and lag_wind[i] are in special double precision.*
 *  See "oper_32b.c" for the format                      *
 *                                                       *
 *-------------------------------------------------------*/

static void Lag_window(
  Word16 m,         /* (i)     : LPC order                        */
  Word16 r_h[],     /* (i/o)   : Autocorrelations  (msb)          */
  Word16 r_l[]      /* (i/o)   : Autocorrelations  (lsb)          */
)
{
  Word16 i;
  Word32 x;

  for(i=1; i<=m; i++)
  {
     //x  = Mpy_32(r_h[i], r_l[i], lag_h[i-1], lag_l[i-1]);
     x = (((Word32)r_h[i]*lag_h[i-1])<<1) +
         (( (((Word32)r_h[i]*lag_l[i-1])>>15) + (((Word32)r_l[i]*lag_h[i-1])>>15) )<<1);
     //L_Extract(x, &r_h[i], &r_l[i]);
     r_h[i] = (Word16) (x >> 16);
     r_l[i] = (Word16)((x >> 1) - (r_h[i] << 15));
  }
}

/*___________________________________________________________________________
 |                                                                           |
 |      LEVINSON-DURBIN algorithm in double precision                        |
 |      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                        |
 |---------------------------------------------------------------------------|
 |                                                                           |
 | Algorithm                                                                 |
 |                                                                           |
 |       R[i]    autocorrelations.                                           |
 |       A[i]    filter coefficients.                                        |
 |       K       reflection coefficients.                                    |
 |       Alpha   prediction gain.                                            |
 |                                                                           |
 |       Initialization:                                                     |
 |               A[0] = 1                                                    |
 |               K    = -R[1]/R[0]                                           |
 |               A[1] = K                                                    |
 |               Alpha = R[0] * (1-K**2]                                     |
 |                                                                           |
 |       Do for  i = 2 to M                                                  |
 |                                                                           |
 |            S =  SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i]                      |
 |                                                                           |
 |            K = -S / Alpha                                                 |
 |                                                                           |
 |            An[j] = A[j] + K*A[i-j]   for j=1 to i-1                       |
 |                                      where   An[i] = new A[i]             |
 |            An[i]=K                                                        |
 |                                                                           |
 |            Alpha=Alpha * (1-K**2)                                         |
 |                                                                           |
 |       END                                                                 |
 |                                                                           |
 | Remarks on the dynamics of the calculations.                              |
 |                                                                           |
 |       The numbers used are in double precision in the following format :  |
 |       A = AH <<16 + AL<<1.  AH and AL are 16 bit signed integers.         |
 |       Since the LSB's also contain a sign bit, this format does not       |
 |       correspond to standard 32 bit integers.  We use this format since   |
 |       it allows fast execution of multiplications and divisions.          |
 |                                                                           |
 |       "DPF" will refer to this special format in the following text.      |
 |       See oper_32b.c                                                      |
 |                                                                           |
 |       The R[i] were normalized in routine AUTO (hence, R[i] < 1.0).       |
 |       The K[i] and Alpha are theoretically < 1.0.                         |
 |       The A[i], for a sampling frequency of 8 kHz, are in practice        |
 |       always inferior to 16.0.                                            |
 |                                                                           |
 |       These characteristics allow straigthforward fixed-point             |
 |       implementation.  We choose to represent the parameters as           |
 |       follows :                                                           |
 |                                                                           |
 |               R[i]    Q31   +- .99..                                      |
 |               K[i]    Q31   +- .99..                                      |
 |               Alpha   Normalized -> mantissa in Q31 plus exponent         |
 |               A[i]    Q27   +- 15.999..                                   |
 |                                                                           |
 |       The additions are performed in 32 bit.  For the summation used      |
 |       to calculate the K[i], we multiply numbers in Q31 by numbers        |
 |       in Q27, with the result of the multiplications in Q27,              |
 |       resulting in a dynamic of +- 16.  This is sufficient to avoid       |
 |       overflow, since the final result of the summation is                |
 |       necessarily < 1.0 as both the K[i] and Alpha are                    |
 |       theoretically < 1.0.                                                |
 |___________________________________________________________________________|
*/


/* Last A(z) for case of unstable filter */

static Word16 old_A[M+1]={4096,0,0,0,0,0,0,0,0,0,0};
static Word16 old_rc[2]={0,0};


static void Levinson(
  Word16 Rh[],      /* (i)     : Rh[M+1] Vector of autocorrelations (msb) */
  Word16 Rl[],      /* (i)     : Rl[M+1] Vector of autocorrelations (lsb) */
  Word16 A[],       /* (o) Q12 : A[M]    LPC coefficients  (m = 10)       */
  Word16 rc[]       /* (o) Q15 : rc[M]   Reflection coefficients.         */
)
{
 Word16 i, j;
 Word16 hi, lo;
 Word16 Kh, Kl;                /* reflection coefficient; hi and lo           */
 Word16 alp_h, alp_l, alp_exp; /* Prediction gain; hi lo and exponent         */
 Word16 Ah[M+1], Al[M+1];      /* LPC coef. in double prec.                   */
 Word16 Anh[M+1], Anl[M+1];    /* LPC coef.for next iteration in double prec. */
 Word32 t0, t1, t2;            /* temporary variable                          */


	/* K = A[1] = -R[1] / R[0] */

  /* R[1] in Q31      */
  t1 = (((Word32) Rh[1]) << 16) + ((Word32)Rl[1] << 1);
  t2  = L_abs(t1);                      /* abs R[1]         */
  t0  = Div_32(t2, Rh[0], Rl[0]);       /* R[1]/R[0] in Q31 */
  if(t1 > 0) t0= -t0;          /* -R[1]/R[0]       */
  /* K in DPF         */
  Kh = (Word16)(t0 >> 16);
  Kl = (Word16)((t0 >> 1) - ((Word32)(Kh) << 15));
  rc[0] = Kh;

  /* A[1] in Q27      */
  /* A[1] in DPF      */
  Ah[1] = (Word16)(t0 >> 20);
  Al[1] = (Word16)((t0 >> 5) - ((Word32)(Ah[1]) << 15));

/*  Alpha = R[0] * (1-K**2) */

  t0  = (((Word32)Kh * Kl) >> 15) << 1;     /* K*K in Q31    */
  t0 += ((Word32)Kh * Kh);
  t0 <<= 1;

  /* Some case <0 !! */
  /* 1 - K*K  in Q31 */
 	t0 = (t0 < 0 ? MAX_32 + t0 : MAX_32 - t0);

  /* DPF format      */
  hi = (Word16)(t0 >> 16);
  lo = (Word16)((t0 >> 1) - ((Word32)(hi) << 15));

  t0  = (((Word32)Rh[0] * lo) >> 15);     /* Alpha in Q31    */
  t0 += (((Word32)Rl[0] * hi) >> 15);
  t0 += ((Word32)Rh[0] * hi);
  t0 <<= 1;
/* Normalize Alpha */

  alp_exp = norm_l(t0);
  //t0 = L_shl(t0, alp_exp);
  t0 = t0 << alp_exp;
  /* DPF format    */
  alp_h = (Word16)(t0 >> 16);
  alp_l = (Word16)((t0 >> 1) - ((Word32)(alp_h) << 15));

/*--------------------------------------*
 * ITERATIONS  I=2 to M                 *
 *--------------------------------------*/

  for(i= 2; i<=M; i++)
  {

    /* t0 = SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i] */

    t0 = 0;
    for(j=1; j<i; j++)
    {
      //t0 = L_add(t0, Mpy_32(Rh[j], Rl[j], Ah[i-j], Al[i-j]));
      t0 += (((Word32)Rh[j] * Al[i-j]) >> 15);
      t0 += (((Word32)Rl[j] * Ah[i-j]) >> 15);
      t0 += ((Word32) Rh[j] * Ah[i-j]);
    }

    t0 <<= 5;                          /* result in Q27 -> convert to Q31 */
                                       /* No overflow possible            */
    t1 = ((Word32) Rh[i] << 16) + ((Word32)(Rl[i]) << 1);
    t0 += t1;                           /* add R[i] in Q31                 */

    /* K = -t0 / Alpha */

    t1 = L_abs(t0);
    t2 = Div_32(t1, alp_h, alp_l);     /* abs(t0)/Alpha                   */
    if(t0 > 0) t2= -t2;                /* K =-t0/Alpha                    */
    t2 = L_shl(t2, alp_exp);           /* denormalize; compare to Alpha   */
    /* K in DPF                        */
    Kh = (Word16)(t2 >> 16);
    Kl = (Word16)((t2 >> 1) - ((Word32)(Kh) << 15));
    rc[i-1] = Kh;

    /* Test for unstable filter. If unstable keep old A(z) */
    if (abs_s(Kh) > 32750)
    {
      Copy(old_A, A, M+1);
      rc[0] = old_rc[0];        /* only two rc coefficients are needed */
      rc[1] = old_rc[1];
      return;
    }

    /*------------------------------------------*
     *  Compute new LPC coeff. -> An[i]         *
     *  An[j]= A[j] + K*A[i-j]     , j=1 to i-1 *
     *  An[i]= K                                *
     *------------------------------------------*/


    for(j=1; j<i; j++)
    {
      t0  = (((Word32)Kh* Al[i-j]) >> 15);
      t0 += (((Word32)Kl* Ah[i-j]) >> 15);
      t0 += ((Word32)Kh* Ah[i-j]);

      t0 += ((Word32)Ah[j] << 15) + (Word32)Al[j];

      Anh[j] = (Word16)(t0 >> 15);
      Anl[j] = (Word16)(t0 - ((Word32)(Anh[j] << 15)));
    }

    /* t2 = K in Q31 ->convert to Q27  */
    Anh[i] = (Word16) (t2 >> 20);
    Anl[i] = (Word16)((t2 >> 5) - ((Word32)(Anh[i]) << 15));

    /*  Alpha = Alpha * (1-K**2) */

    t0  = (((Word32)Kh * Kl) >> 15) << 1;     /* K*K in Q31    */
 		t0 += ((Word32)Kh * Kh);
  	t0 <<= 1;

    /* Some case <0 !! */
    /* 1 - K*K  in Q31 */
    t0 = (t0 < 0 ? MAX_32 + t0 : MAX_32 - t0);

    /* DPF format      */
    hi = (Word16)(t0 >> 16);
    lo = (Word16)((t0 >> 1) - ((Word32)(hi) << 15));

		t0  = (((Word32)alp_h * lo) >> 15);     /* Alpha in Q31    */
    t0 += (((Word32)alp_l * hi) >> 15);
    t0 += ((Word32)alp_h * hi);
    t0 <<= 1;

    /* Normalize Alpha */
    j = norm_l(t0);
    t0 <<= j;

    /* DPF format    */
    alp_h = (Word16)(t0 >> 16);
    alp_l = (Word16)((t0 >> 1) - ((Word32)(alp_h) << 15));
	  alp_exp += j;             /* Add normalization to alp_exp */

    /* A[j] = An[j] */
    Copy(&Anh[1], &Ah[1], i);
    Copy(&Anl[1], &Al[1], i);
  }

  /* Truncate A[i] in Q27 to Q12 with rounding */

  A[0] = 4096;
  for(i=1; i<=M; i++)
  {
    t0 = ((Word32) Ah[i] << 15) + Al[i];
    old_A[i] = A[i] = (Word16)((t0 + 0x00002000) >> 14);
  }
  old_rc[0] = rc[0];
  old_rc[1] = rc[1];
}



/*-------------------------------------------------------------*
 *  procedure Az_lsp:                                          *
 *            ~~~~~~                                           *
 *   Compute the LSPs from  the LPC coefficients  (order=10)   *
 *-------------------------------------------------------------*/

/* local function */

static Word16 Chebps_11(Word16 x, Word16 f[], Word16 n);
static Word16 Chebps_10(Word16 x, Word16 f[], Word16 n);

static void Az_lsp(
  Word16 a[],        /* (i) Q12 : predictor coefficients              */
  Word16 lsp[],      /* (o) Q15 : line spectral pairs                 */
  Word16 old_lsp[]   /* (i)     : old lsp[] (in case not found 10 roots) */
)
{
 Word16 i, j, nf, ip;
 Word16 xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
 Word16 x, y, sign, exp;
 Word16 *coef;
 Word16 f1[M/2+1], f2[M/2+1];
 Word32 L_temp1, L_temp2;
 Word16 (*pChebps)(Word16 x, Word16 f[], Word16 n);

/*-------------------------------------------------------------*
 *  find the sum and diff. pol. F1(z) and F2(z)                *
 *    F1(z) <--- F1(z)/(1+z**-1) & F2(z) <--- F2(z)/(1-z**-1)  *
 *                                                             *
 * f1[0] = 1.0;                                                *
 * f2[0] = 1.0;                                                *
 *                                                             *
 * for (i = 0; i< NC; i++)                                     *
 * {                                                           *
 *   f1[i+1] = a[i+1] + a[M-i] - f1[i] ;                       *
 *   f2[i+1] = a[i+1] - a[M-i] + f2[i] ;                       *
 * }                                                           *
 *-------------------------------------------------------------*/

 pChebps = Chebps_11;

 f1[0] = 2048;          /* f1[0] = 1.0 is in Q11 */
 f2[0] = 2048;          /* f2[0] = 1.0 is in Q11 */

 for (i = 0; i< NC; i++)
 {
	 L_temp1 = (Word32)a[i+1];
   L_temp2 = (Word32)a[M-i];

   /* x = (a[i+1] + a[M-i]) >> 1        */
   x = ((L_temp1 + L_temp2) >> 1);
   /* x = (a[i+1] - a[M-i]) >> 1        */
   y = ((L_temp1 - L_temp2) >> 1);

   /* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
   L_temp1 = (Word32)x - (Word32)f1[i];
   if (L_temp1 > 0x00007fffL || L_temp1 < (Word32)0xffff8000L)
     break;
   f1[i+1] = (Word16)L_temp1;

   /* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
   L_temp2 = (Word32)y + (Word32)f2[i];
   if (L_temp2 > 0x00007fffL || (L_temp2 < (Word32)0xffff8000L))
     break;
   f2[i+1] = (Word16)L_temp2;
 }

 if (i != NC) {
   //printf("===== OVF ovf_coef =====\n");

   pChebps = Chebps_10;

   f1[0] = 1024;          /* f1[0] = 1.0 is in Q10 */
   f2[0] = 1024;          /* f2[0] = 1.0 is in Q10 */

   for (i = 0; i< NC; i++)
   {
     L_temp1 = (Word32)a[i+1];
     L_temp2 = (Word32)a[M-i];
     /* x = (a[i+1] + a[M-i]) >> 2  */
     x = (Word16)((L_temp1 + L_temp2) >> 2);
     /* y = (a[i+1] - a[M-i]) >> 2 */
     y = (Word16)((L_temp1 - L_temp2) >> 2);

     f1[i+1] = x - f1[i];            /* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
     f2[i+1] = y + f2[i];            /* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
   }
 }

/*-------------------------------------------------------------*
 * find the LSPs using the Chebichev pol. evaluation           *
 *-------------------------------------------------------------*/

 nf=0;          /* number of found frequencies */
 ip=0;          /* indicator for f1 or f2      */

 coef = f1;

 xlow = grid[0];
 ylow = (*pChebps)(xlow, coef, NC);

 j = 0;
 while ( (nf < M) && (j < GRID_POINTS) )
 {
   j++;
   xhigh = xlow;
   yhigh = ylow;
   xlow  = grid[j];
   ylow  = (*pChebps)(xlow,coef,NC);

   if (((Word32)ylow*yhigh) <= 0)
   {
     /* divide 2 times the interval */
     for (i = 0; i < 2; i++)
     {
       /* xmid = (xlow + xhigh)/2 */
			 xmid = (xlow >> 1) + (xhigh >> 1);

       ymid = (*pChebps)(xmid,coef,NC);

       if ( ((Word32)ylow*ymid) <= (Word32)0)
       {
         yhigh = ymid;
         xhigh = xmid;
       }
       else
       {
         ylow = ymid;
         xlow = xmid;
       }
     }

    /*-------------------------------------------------------------*
     * Linear interpolation                                        *
     *    xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);            *
     *-------------------------------------------------------------*/

     x   = xhigh - xlow;
     y   = yhigh - ylow;

     if(y == 0)
     {
       xint = xlow;
     }
     else
     {
       sign= y;
       y   = abs_s(y);
       exp = norm_s(y);
       y <<= exp;
       y   = div_s( (Word16)16383, y);
       /* y= (xhigh-xlow)/(yhigh-ylow) in Q11 */
			 y = ((Word32)x * (Word32)y) >> (19 - exp);

       if(sign < 0) y = -y;

       /* xint = xlow - ylow*y */
       xint = xlow - (Word16)(((Word32) ylow * y) >> 10);
     }

     lsp[nf] = xint;
     xlow    = xint;
     nf++;

     if(ip == 0)
     {
       ip = 1;
       coef = f2;
     }
     else
     {
       ip = 0;
       coef = f1;
     }
     ylow = (*pChebps)(xlow,coef,NC);

   }
 }

 /* Check if M roots found */

 if (nf < M)
 {
   Copy(old_lsp, lsp, M);
 /* printf("\n !!Not 10 roots found in Az_lsp()!!!\n"); */
 }
}

/*--------------------------------------------------------------*
 * function  Chebps_11, Chebps_10:                              *
 *           ~~~~~~~~~~~~~~~~~~~~                               *
 *    Evaluates the Chebichev polynomial series                 *
 *--------------------------------------------------------------*
 *                                                              *
 *  The polynomial order is                                     *
 *     n = M/2   (M is the prediction order)                    *
 *  The polynomial is given by                                  *
 *    C(x) = T_n(x) + f(1)T_n-1(x) + ... +f(n-1)T_1(x) + f(n)/2 *
 * Arguments:                                                   *
 *  x:     input value of evaluation; x = cos(frequency) in Q15 *
 *  f[]:   coefficients of the pol.                             *
 *                         in Q11(Chebps_11), in Q10(Chebps_10) *
 *  n:     order of the pol.                                    *
 *                                                              *
 * The value of C(x) is returned. (Saturated to +-1.99 in Q14)  *
 *                                                              *
 *--------------------------------------------------------------*/
static Word16 Chebps_11(Word16 x, Word16 f[], Word16 n)
{
  Word16 i, cheb;
  Word16 b1_h, b1_l;
  Word32 t0;
  Word32 L_temp;

 /* Note: All computation are done in Q24. */

  L_temp = 0x01000000;

  /* 2*x in Q24 + f[1] in Q24 */
  t0 = ((Word32)x << 10) + ((Word32)f[1] << 13);

  /* b1 = 2*x + f[1]     */
  b1_h = (Word16)(t0 >> 16);
  b1_l = (Word16)((t0 >> 1) - (b1_h << 15));

  for (i = 2; i<n; i++)
  {
    /* t0 = 2.0*x*b1              */
    t0  = ((Word32) b1_h * x) + (((Word32) b1_l * x) >> 15);
    t0 <<= 2;
    /* t0 = 2.0*x*b1 - b2         */
    t0 -= L_temp;
    /* t0 = 2.0*x*b1 - b2 + f[i]; */
    t0 += ((Word32)f[i] << 13);

    /* b2 = b1; */
    L_temp = ((Word32) b1_h << 16) + ((Word32) b1_l << 1);

    /* b0 = 2.0*x*b1 - b2 + f[i]; */
    b1_h = (Word16)(t0 >> 16);
    b1_l = (Word16)((t0 >> 1) - (b1_h << 15));
  }

  /* t0 = x*b1;              */
  t0  = ((Word32) b1_h * x) + (((Word32) b1_l * x) >> 15);
  t0 <<= 1;
  /* t0 = x*b1 - b2          */
  t0 -= L_temp;
  /* t0 = x*b1 - b2 + f[i]/2 */
  t0 += ((Word32)f[i] << 12);

  /* Q24 to Q30 with saturation */
  /* Result in Q14              */
  if ((UWord32)(t0 - 0xfe000000L) < 0x01ffffffL -  0xfe000000L)
    cheb = (Word16)(t0 >> 10);
  else
    cheb = t0 > (Word32) 0x01ffffffL ? MAX_16 : MIN_16;

  return(cheb);
}

static Word16 Chebps_10(Word16 x, Word16 f[], Word16 n)
{
  Word16 i, cheb;
  Word16 b1_h, b1_l;
  Word32 t0;
  Word32 L_temp;

 /* Note: All computation are done in Q23. */

  L_temp = 0x00800000;

  /* 2*x + f[1] in Q23          */
  t0 = ((Word32)x << 9) + ((Word32)f[1] << 13);

  /* b1 = 2*x + f[1]     */
  b1_h = (Word16)(t0 >> 16);
  b1_l = (Word16)((t0 >> 1) - (b1_h << 15));

  for (i = 2; i<n; i++)
  {
    /* t0 = 2.0*x*b1              */
    t0  = ((Word32) b1_h * x) + (((Word32) b1_l * x) >> 15);
    t0 <<= 2;
    /* t0 = 2.0*x*b1 - b2         */
    t0 -= L_temp;

    /* t0 = 2.0*x*b1 - b2 + f[i]; */
    t0 += ((Word32)f[i] << 13);

    /* b2 = b1; */
    L_temp = ((Word32) b1_h << 16) + ((Word32) b1_l << 1);

    /* b0 = 2.0*x*b1 - b2 + f[i]; */
    b1_h = (Word16)(t0 >> 16);
    b1_l = (Word16)((t0 >> 1) - (b1_h << 15));
  }

  /* t0 = x*b1;              */
  t0  = ((Word32) b1_h * x) + (((Word32) b1_l * x) >> 15);
  t0 <<= 1;
  /* t0 = x*b1 - b2          */
  t0 -= L_temp;
  /* t0 = x*b1 - b2 + f[i]/2 */
  t0 += ((Word32)f[i] << 12);

  /* Q23 to Q30 with saturation */
  /* Result in Q14              */
  if ((UWord32)(t0 - 0xff000000L) < 0x00ffffffL -  0xff000000L)
    cheb = (Word16)(t0 >> 9);
  else
    cheb = t0 > (Word32) 0x00ffffffL ? MAX_16 : MIN_16;

  return(cheb);
}
/************** End of file lpc.c *******************************************/

/************** Begin of file lpcfunc.c *************************************/
/*-------------------------------------------------------------*
 *  Procedure Lsp_Az:                                          *
 *            ~~~~~~                                           *
 *   Compute the LPC coefficients from lsp (order=10)          *
 *-------------------------------------------------------------*/

/* local function */

static void Get_lsp_pol(Word16 *lsp, Word32 *f);

static void Lsp_Az(
  Word16 lsp[],    /* (i) Q15 : line spectral frequencies            */
  Word16 a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
)
{
  Word16 i;
  Word32 f1[6], f2[6];
  Word32 ff1, ff2, fff1, fff2;

  Get_lsp_pol(&lsp[0],f1);
  Get_lsp_pol(&lsp[1],f2);

  a[0] = 4096;
  for (i = 1; i <= 5; i++)
  {
    ff1 = f1[i] + f1[i-1];
    ff2 = f2[i] - f2[i-1];

    fff1 = ff1 + ff2 + ((Word32) 1 << 12);
    fff2 = ff1 - ff2 + ((Word32) 1 << 12);

    a[i]    = (Word16)(fff1 >> 13);
    a[11-i] = (Word16)(fff2 >> 13);
  }
}


static Word32 mull(Word32 a, Word16 b)
{
#if defined(ARCH_ARM)
  register Word32 ra = a;
  register Word32 rb = b;
  Word32 lo, hi;
  __asm__("smull %0, %1, %2, %3     \n\t"
          "mov   %0, %0,     LSR #16 \n\t"
          "add   %1, %0, %1, LSL #16  \n\t"
          : "=&r"(lo), "=&r"(hi)
          : "r"(rb), "r"(ra));
  return hi;
#else
  Word16 hi, lo;
  Word32 t0;
  hi = (Word16)(a >> 16);
  lo = (Word16)((a >> 1) - ((Word32) hi << 15));
  t0  = ((Word32)hi * b);
  t0 += ((Word32)lo * b) >> 15;
  return t0;
#endif
}

/*-----------------------------------------------------------*
 * procedure Get_lsp_pol:                                    *
 *           ~~~~~~~~~~~                                     *
 *   Find the polynomial F1(z) or F2(z) from the LSPs        *
 *-----------------------------------------------------------*
 *                                                           *
 * Parameters:                                               *
 *  lsp[]   : line spectral freq. (cosine domain)    in Q15  *
 *  f[]     : the coefficients of F1 or F2           in Q24  *
 *-----------------------------------------------------------*/
static void Get_lsp_pol(Word16 *lsp, Word32 *f)
{
  Word16 i, j;

  *f++ = (Word32) 0x01000000;       /* f[0] = 1.0;             in Q24 */
  *f++ = (Word32) - *(lsp++) << 10; /* f[1] =  -2.0 * lsp[0];  in Q24 */
  lsp++;                            /* Advance lsp pointer            */

  for (i=2; i<=5; f+=i,lsp++,i++)
  {
    *f = f[-2];
    for (j=1; j<i; j++,f--)
      *f += f[-2] - (mull(f[-1], *lsp) << 2);
    *f -= (Word32)(*lsp++) << 10;
  }

  return;
}

/*___________________________________________________________________________
 |                                                                           |
 |   Functions : Lsp_lsf and Lsf_lsp                                         |
 |                                                                           |
 |      Lsp_lsf   Transformation lsp to lsf                                  |
 |      Lsf_lsp   Transformation lsf to lsp                                  |
 |---------------------------------------------------------------------------|
 |  Algorithm:                                                               |
 |                                                                           |
 |   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are       |
 |   approximated by a look-up table and interpolation.                      |
 |___________________________________________________________________________|
*/
static void Lsf_lsp2(
  Word16 lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
  Word16 lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
  Word16 m         /* (i)     : LPC order                   */
)
{
  Word16 i;
  UWord8 ind, offset;   /* in Q8 */
  Word16 freq;     /* normalized frequency in Q15 */

  for(i=0; i<m; i++)
  {
    freq = lsf[i] * 20861 >> 15;  /* 20861: 1.0/(2.0*PI) in Q17 */
    ind    = freq >> 8;           /* ind    = b8-b15 of freq */
    offset = freq;                /* offset = b0-b7  of freq */

    if ( ind > 63){
      ind = 63;                 /* 0 <= ind <= 63 */
    }

    lsp[i] = table2[ind]+ (slope_cos[ind]*offset >> 12);
  }
}



static void Lsp_lsf2(
  Word16 lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
  Word16 lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
  Word16 m         /* (i)     : LPC order                   */
)
{
  Word16 i, ind;
  Word16 offset;   /* in Q15 */
  Word16 freq;     /* normalized frequency in Q16 */

  ind = 63;           /* begin at end of table2 -1 */

  for(i= m-(Word16)1; i >= 0; i--)
  {
    /* find value in table2 that is just greater than lsp[i] */
    for (; table2[ind] < lsp[i] && ind; ind--);

    offset = lsp[i] - table2[ind];

    /* acos(lsp[i])= ind*512 + (slope_acos[ind]*offset >> 11) */
    freq = (ind << 9) + (slope_acos[ind]*offset >> 11);
    lsf[i] = freq * 25736 >> 15;           /* 25736: 2.0*PI in Q12 */
  }
}

/*-------------------------------------------------------------*
 *  procedure Weight_Az                                        *
 *            ~~~~~~~~~                                        *
 * Weighting of LPC coefficients.                              *
 *   ap[i]  =  a[i] * (gamma ** i)                             *
 *                                                             *
 *-------------------------------------------------------------*/


static void Weight_Az(
  Word16 a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
  Word16 gamma,    /* (i) Q15 : Spectral expansion factor.           */
  Word16 m,        /* (i)     : LPC order.                           */
  Word16 ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
)
{
  Word16 i, fac;

  ap[0] = a[0];
  fac   = gamma;
  for(i=1; i<m; i++)
  {
    ap[i] = g_round( (Word32)a[i] * (Word32)fac << 1 );
    fac   = g_round( (Word32)fac * (Word32)gamma << 1 );
  }
  ap[m] = g_round( (Word32)a[m] * (Word32)fac << 1 );
}

/*----------------------------------------------------------------------*
 * Function Int_qlpc()                                                  *
 * ~~~~~~~~~~~~~~~~~~~                                                  *
 * Interpolation of the LPC parameters.                                 *
 *----------------------------------------------------------------------*/

/* Interpolation of the quantized LSP's */

static void Int_qlpc(
 Word16 lsp_old[], /* input : LSP vector of past frame              */
 Word16 lsp_new[], /* input : LSP vector of present frame           */
 Word16 Az[]       /* output: interpolated Az() for the 2 subframes */
)
{
  Word16 i;
  Word16 lsp[M];

  /*  lsp[i] = lsp_new[i] * 0.5 + lsp_old[i] * 0.5 */
  for (i = 0; i < M; i++)
    lsp[i] = (lsp_new[i] >> 1) + (lsp_old[i] >> 1);


  Lsp_Az(lsp, Az);              /* Subframe 1 */

  Lsp_Az(lsp_new, &Az[MP1]);    /* Subframe 2 */
}
/************** End of file lpcfunc.c ***************************************/

/************** End of file lspdec.c ****************************************/
static void Lsp_iqua_cs(g729a_decoder_state *state, Word16 prm[], Word16 lsp_q[], Word16 erase);

/* static memory */
static Word16 freq_prev_reset[M] = { /* Q13 */
  2339, 4679, 7018, 9358, 11698, 14037, 16377, 18717, 21056, 23396
};     /* PI*(float)(j+1)/(float)(M+1) */

/*----------------------------------------------------------------------------
 * Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
static void Lsp_decw_reset(
g729a_decoder_state *state
)
{
  Word16 i;

  for(i=0; i<MA_NP; i++)
    Copy( &freq_prev_reset[0], &(state->freq_prev[i][0]), M );

  state->prev_ma = 0;

  Copy( freq_prev_reset, state->prev_lsp, M);
}



/*----------------------------------------------------------------------------
 * Lsp_iqua_cs -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
static void Lsp_iqua_cs(
 g729a_decoder_state *state,
 Word16 prm[],          /* (i)     : indexes of the selected LSP */
 Word16 lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
 Word16 erase           /* (i)     : frame erase information     */
)
{
  Word16 mode_index;
  Word16 code0;
  Word16 code1;
  Word16 code2;
  Word16 buf[M];     /* Q13 */

  if( erase==0 ) {  /* Not frame erasure */
    mode_index = (prm[0] >> NC0_B) & (Word16)1;
    code0 = prm[0] & (Word16)(NC0 - 1);
    code1 = (prm[1] >> NC1_B) & (Word16)(NC1 - 1);
    code2 = prm[1] & (Word16)(NC1 - 1);

    /* compose quantized LSP (lsp_q) from indexes */

    Lsp_get_quant(lspcb1, lspcb2, code0, code1, code2,
      fg[mode_index], state->freq_prev, lsp_q, fg_sum[mode_index]);

    /* save parameters to use in case of the frame erased situation */

    Copy(lsp_q, state->prev_lsp, M);
    state->prev_ma = mode_index;
  }
  else {           /* Frame erased */
    /* use revious LSP */

    Copy(state->prev_lsp, lsp_q, M);

    /* update freq_prev */

    Lsp_prev_extract(state->prev_lsp, buf,
      fg[state->prev_ma], state->freq_prev, fg_sum_inv[state->prev_ma]);
    Lsp_prev_update(buf, state->freq_prev);
  }
}



/*-------------------------------------------------------------------*
 * Function  D_lsp:                                                  *
 *           ~~~~~~                                                  *
 *-------------------------------------------------------------------*/

static void D_lsp(
  g729a_decoder_state *state,
  Word16 prm[],          /* (i)     : indexes of the selected LSP */
  Word16 lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  Word16 erase           /* (i)     : frame erase information     */
)
{
  Word16 lsf_q[M];       /* domain 0.0<= lsf_q <PI in Q13 */


  Lsp_iqua_cs(state, prm, lsf_q, erase);

  /* Convert LSFs to LSPs */

  Lsf_lsp2(lsf_q, lsp_q, M);
}
/************** End of file lspdec.c ****************************************/

/************** Begin of file lspgetq.c *************************************/
/* lsf_decode */
static void Lsp_get_quant(
  Word16 lspcb1[][M],      /* (i) Q13 : first stage LSP codebook      */
  Word16 lspcb2[][M],      /* (i) Q13 : Second stage LSP codebook     */
  Word16 code0,            /* (i)     : selected code of first stage  */
  Word16 code1,            /* (i)     : selected code of second stage */
  Word16 code2,            /* (i)     : selected code of second stage */
  Word16 fg[][M],          /* (i) Q15 : MA prediction coef.           */
  Word16 freq_prev[][M],   /* (i) Q13 : previous LSP vector           */
  Word16 lspq[],           /* (o) Q13 : quantized LSP parameters      */
  Word16 fg_sum[]          /* (i) Q15 : present MA prediction coef.   */
)
{
  static const UWord8 gap[2]={GAP1, GAP2};
  Word16 j, i;
  Word16 buf[M];           /* Q13 */
  Word32 diff, acc;

  for ( j = 0 ; j < NC ; j++ )
  {
    buf[j]    = lspcb1[code0][j]    + lspcb2[code1][j];
    buf[j+NC] = lspcb1[code0][j+NC] + lspcb2[code2][j+NC];
  }

  /* Lsp_expand_1_2 */
  for (i = 0; i < 2; ++i)
    for ( j = 1 ; j < M ; j++ )
    {
      diff = (buf[j-1] - buf[j] + gap[i]) >> 1;
      if ( diff > 0 )
      {
        buf[j-1] -= diff;
        buf[j]   += diff;
      }
    }

  /* Lsp_prev_compose
   * compose LSP parameter from elementary LSP with previous LSP. */
  for ( i = 0 ; i < M ; i++ ) {
    acc = buf[i] * fg_sum[i];
    for ( j = 0 ; j < MA_NP ; j++ )
      acc += freq_prev[j][i] * fg[j][i];

    lspq[i] = acc >> 15;
  }

  /* Lsp_prev_update */
  for ( j = MA_NP-1 ; j > 0 ; j-- )
    Copy(freq_prev[j-1], freq_prev[j], M);
  Copy(buf, freq_prev[0], M);

  Lsp_stability( lspq );
}

/*
  Functions which use previous LSP parameter (freq_prev).
*/

/*
  extract elementary LSP from composed LSP with previous LSP
*/
static void Lsp_prev_extract(
  Word16 lsp[M],                /* (i) Q13 : unquantized LSP parameters  */
  Word16 lsp_ele[M],            /* (o) Q13 : target vector               */
  Word16 fg[MA_NP][M],          /* (i) Q15 : MA prediction coef.         */
  Word16 freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector         */
  Word16 fg_sum_inv[M]          /* (i) Q12 : inverse previous LSP vector */
)
{
  Word16 j, k;
  Word32 L_temp;                /* Q19 */
  Word16 temp;                  /* Q13 */


  for ( j = 0 ; j < M ; j++ ) {
    L_temp = ((Word32)lsp[j]) << 15;
    for ( k = 0 ; k < MA_NP ; k++ )
      L_temp -= freq_prev[k][j] * fg[k][j];

    temp = (Word16)(L_temp >> 15);
    L_temp = ((Word32)temp * (Word32)fg_sum_inv[j]) >> 12;
    lsp_ele[j] = (Word16)L_temp;
  }
  return;
}


/*
  update previous LSP parameter
*/
static void Lsp_prev_update(
  Word16 lsp_ele[M],             /* (i)   Q13 : LSP vectors           */
  Word16 freq_prev[MA_NP][M]     /* (i/o) Q13 : previous LSP vectors  */
)
{
  Word16 k;

  for ( k = MA_NP-1 ; k > 0 ; k-- )
    Copy(freq_prev[k-1], freq_prev[k], M);

  Copy(lsp_ele, freq_prev[0], M);
}

/* ff_acelp_reorder_lsf */
static void Lsp_stability(
  Word16 buf[]       /* (i/o) Q13 : quantized LSP parameters      */
)
{
  Word16 j;
  Word16 tmp;
  Word32 L_diff;

  for(j=0; j<M-1; j++) {
    L_diff = buf[j+1] - buf[j];

    if( L_diff < 0L ) {
      /* exchange buf[j]<->buf[j+1] */
      tmp      = buf[j+1];
      buf[j+1] = buf[j];
      buf[j]   = tmp;
    }
  }

  if( buf[0] < L_LIMIT) {
    buf[0] = L_LIMIT;
  }

  for(j=0; j<M-1; j++) {
    L_diff = buf[j+1] - buf[j];

    if( L_diff < (Word32)GAP3) {
      buf[j+1] = buf[j] + GAP3;
    }
  }

  if( buf[M-1] > M_LIMIT) {
    buf[M-1] = M_LIMIT;
  }
}
/************** End of file lspgetq.c ***************************************/

/************** Begin of file p_parity.c ************************************/
/*------------------------------------------------------*
 * Parity_pitch - compute parity bit for first 6 MSBs   *
 *------------------------------------------------------*/

static Word16 Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
   Word16 pitch_index   /* input : index for which parity to compute */
)
{
  Word16 temp, sum, i, bit;

  temp = pitch_index >> 1;

  sum = 1;
  for (i = 0; i <= 5; i++) {
    temp >>= 1;
    bit = temp & (Word16)1;
    sum += bit;
  }
  sum = sum & (Word16)1;

  return sum;
 }

/*--------------------------------------------------------------------*
 * check_parity_pitch - check parity of index with transmitted parity *
 *--------------------------------------------------------------------*/

static Word16  Check_Parity_Pitch( /* output: 0 = no error, 1= error */
  Word16 pitch_index,       /* input : index of parameter     */
  Word16 parity             /* input : parity bit             */
)
{
  Word16 temp, sum, i, bit;

  temp = pitch_index >> 1;

  sum = 1;
  for (i = 0; i <= 5; i++) {
    temp = temp >> 1;
    bit  = temp & (Word16)1;
    sum += bit;
  }
  sum += parity;
  sum = sum & (Word16)1;

  return sum;
}
/************** End of file p_parity.c **************************************/

/************** Begin of file pitch_a.c *************************************/
/*---------------------------------------------------------------------------*
 * Pitch related functions                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 *---------------------------------------------------------------------------*/

static Word16 Compute_nrj_max(Word16 *scal_sig, Word16 L_frame, Word32 max)
{
  Word32 sum;
  Word16  max_h, max_l, ener_h, ener_l;
  Word16 i;

  sum = 0;
  for(i=0; i<L_frame; i+=2)
    sum += scal_sig[i] * scal_sig[i];
  sum <<= 1;
  sum++; /* to avoid division by zero */

  /* max1 = max/sqrt(energy)                  */
  /* This result will always be on 16 bits !! */

  sum = Inv_sqrt(sum);            /* 1/sqrt(energy),    result in Q30 */
  //L_Extract(max, &max_h, &max_l);
  //L_Extract(sum, &ener_h, &ener_l);
  max_h = (Word16) (max >> 16);
  max_l = (Word16)((max >> 1) - (max_h << 15));
  ener_h = (Word16) (sum >> 16);
  ener_l = (Word16)((sum >> 1) - (ener_h << 15));
  //sum  = Mpy_32(max_h, max_l, ener_h, ener_l);
  sum = (((Word32)max_h*ener_h)<<1) +
        (( (((Word32)max_h*ener_l)>>15) + (((Word32)max_l*ener_h)>>15) )<<1);

  return (Word16)sum;
}

/*---------------------------------------------------------------------------*
 * Function  Pitch_ol_fast                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~                                                   *
 * Compute the open loop pitch lag. (fast version)                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/
static Word16 Pitch_ol_fast(  /* output: open loop pitch lag                        */
   Word16 signal[],    /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
   Word16   pit_max,   /* input : maximum pitch lag                          */
   Word16   L_frame    /* input : length of frame to compute pitch           */
)
{
  Word16  i, j;
  Word16  max1, max2, max3;
  Word16  T1, T2, T3;
  Word16  *p, *p1;
  Word32  max, sum, sum1;

  /* Scaled signal */

  Word16 scaled_signal[L_FRAME+PIT_MAX];
  Word16 *scal_sig;

  scal_sig = &scaled_signal[pit_max];

  /*--------------------------------------------------------*
   *  Verification for risk of overflow.                    *
   *--------------------------------------------------------*/

   sum = 0;
   for(i= -pit_max; i< L_frame; i+=2)
   {
     sum += ((Word32)signal[i] * (Word32)signal[i]) << 1;
     if (sum < 0)  // overflow
     {
       sum = MAX_32;
       break;
     }
   }

  /*--------------------------------------------------------*
   * Scaling of input signal.                               *
   *                                                        *
   *   if overflow        -> scal_sig[i] = signal[i]>>3     *
   *   else if sum < 1^20 -> scal_sig[i] = signal[i]<<3     *
   *   else               -> scal_sig[i] = signal[i]        *
   *--------------------------------------------------------*/
   if (sum == MAX_32)
   {
     for(i=-pit_max; i<L_frame; i++)
       scal_sig[i] = signal[i] >> 3;
   }
   else {
     if (sum < (Word32)0x100000) /* if (sum < 2^20) */
     {
        for(i=-pit_max; i<L_frame; i++)
          scal_sig[i] = signal[i] << 3;
     }
     else
     {
       for(i=-pit_max; i<L_frame; i++)
         scal_sig[i] = signal[i];
     }
   }

   /*--------------------------------------------------------------------*
    *  The pitch lag search is divided in three sections.                *
    *  Each section cannot have a pitch multiple.                        *
    *  We find a maximum for each section.                               *
    *  We compare the maxima of each section by favoring small lag.      *
    *                                                                    *
    *  First section:  lag delay = 20 to 39                              *
    *  Second section: lag delay = 40 to 79                              *
    *  Third section:  lag delay = 80 to 143                             *
    *--------------------------------------------------------------------*/

    /* First section */

    max = MIN_32;
    T1  = 20;    /* Only to remove warning from some compilers */
    for (i = 20; i < 40; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
          sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T1 = i;   }
    }

    /* compute energy of maximum */
    max1 = Compute_nrj_max(&scal_sig[-T1], L_frame, max);

    /* Second section */

    max = MIN_32;
    T2  = 40;    /* Only to remove warning from some compilers */
    for (i = 40; i < 80; i++) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
          sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T2 = i;   }
    }

    /* compute energy of maximum */
    max2 = Compute_nrj_max(&scal_sig[-T2], L_frame, max);

    /* Third section */

    max = MIN_32;
    T3  = 80;    /* Only to remove warning from some compilers */
    for (i = 80; i < 143; i+=2) {
        p  = scal_sig;
        p1 = &scal_sig[-i];
        sum = 0;
        for (j=0; j<L_frame; j+=2, p+=2, p1+=2)
            sum += *p * *p1;
        sum <<= 1;
        if (sum > max) { max = sum; T3 = i;   }
    }

     /* Test around max3 */
     i = T3;
     sum = 0;
     sum1 = 0;
     for (j=0; j<L_frame; j+=2)
     {
       sum  += scal_sig[j] * scal_sig[-(i+1) + j];
       sum1 += scal_sig[j] * scal_sig[-(i-1) + j];
     }
     sum  <<= 1;
     sum1 <<= 1;

     if (sum  > max) { max = sum;  T3 = i+(Word16)1;   }
     if (sum1 > max) { max = sum1; T3 = i-(Word16)1;   }

    /* compute energy of maximum */
    max3 = Compute_nrj_max(&scal_sig[-T3], L_frame, max);

   /*-----------------------*
    * Test for multiple.    *
    *-----------------------*/

    /* if( abs(T2*2 - T3) < 5)  */
    /*    max2 += max3 * 0.25;  */
    i = T2*2 - T3;
    if (abs_s(i) < 5)
      max2 += max3 >> 2;

    /* if( abs(T2*3 - T3) < 7)  */
    /*    max2 += max3 * 0.25;  */
    i += T2;
    if (abs_s(i) < 7)
      max2 += max3 >> 2;

    /* if( abs(T1*2 - T2) < 5)  */
    /*    max1 += max2 * 0.20;  */
    i = T1 * 2 - T2;
    if (abs_s(i) < 5)
      max1 += mult(max2, 6554);

    /* if( abs(T1*3 - T2) < 7)  */
    /*    max1 += max2 * 0.20;  */

    i += T1;
    if (abs_s(i) < 7)
      max1 += mult(max2, 6554);

   /*--------------------------------------------------------------------*
    * Compare the 3 sections maxima.                                     *
    *--------------------------------------------------------------------*/

    if( max1 < max2 ) {max1 = max2; T1 = T2;  }
    if( max1 < max3 )  {T1 = T3; }

    return T1;
}

/*--------------------------------------------------------------------------*
 *  Function  Dot_Product()                                                 *
 *  ~~~~~~~~~~~~~~~~~~~~~~                                                  *
 *--------------------------------------------------------------------------*/
static Word32 Dot_Product(      /* (o)   :Result of scalar product. */
       Word16   x[],     /* (i)   :First vector.             */
       Word16   y[],     /* (i)   :Second vector.            */
       Word16   lg       /* (i)   :Number of point.          */
)
{
  Word16 i;
  Word32 sum;

  sum = 0;
  for(i=0; i<lg; i++)
    sum += x[i] * y [i];
  sum <<= 1;

  return sum;
}

/*--------------------------------------------------------------------------*
 *  Function  Pitch_fr3_fast()                                              *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~                                              *
 * Fast version of the pitch close loop.                                    *
 *--------------------------------------------------------------------------*/
static Word16 Pitch_fr3_fast(/* (o)     : pitch period.                          */
  Word16 exc[],       /* (i)     : excitation buffer                      */
  Word16 xn[],        /* (i)     : target vector                          */
  Word16 h[],         /* (i) Q12 : impulse response of filters.           */
  Word16 L_subfr,     /* (i)     : Length of subframe                     */
  Word16 t0_min,      /* (i)     : minimum value in the searched range.   */
  Word16 t0_max,      /* (i)     : maximum value in the searched range.   */
  Word16 i_subfr,     /* (i)     : indicator for first subframe.          */
  Word16 *pit_frac    /* (o)     : chosen fraction.                       */
)
{
  Word16 t, t0;
  Word16 Dn[L_SUBFR];
  Word16 exc_tmp[L_SUBFR];
  Word32 max, corr;

 /*-----------------------------------------------------------------*
  * Compute correlation of target vector with impulse response.     *
  *-----------------------------------------------------------------*/

  Cor_h_X(h, xn, Dn);

 /*-----------------------------------------------------------------*
  * Find maximum integer delay.                                     *
  *-----------------------------------------------------------------*/

  max = MIN_32;
  t0 = t0_min; /* Only to remove warning from some compilers */

  for(t=t0_min; t<=t0_max; t++)
  {
    corr = Dot_Product(Dn, &exc[-t], L_subfr);
    if(corr > max) {max = corr; t0 = t;  }
  }

 /*-----------------------------------------------------------------*
  * Test fractions.                                                 *
  *-----------------------------------------------------------------*/

  /* Fraction 0 */

  Pred_lt_3(exc, t0, 0, L_subfr);
  max = Dot_Product(Dn, exc, L_subfr);
  *pit_frac = 0;

  /* If first subframe and lag > 84 do not search fractional pitch */

  if( (i_subfr == 0) && (t0 > 84) )
    return t0;

  Copy(exc, exc_tmp, L_subfr);

  /* Fraction -1/3 */

  Pred_lt_3(exc, t0, -1, L_subfr);
  corr = Dot_Product(Dn, exc, L_subfr);
  if(corr > max) {
     max = corr;
     *pit_frac = -1;
     Copy(exc, exc_tmp, L_subfr);
  }

  /* Fraction +1/3 */

  Pred_lt_3(exc, t0, 1, L_subfr);
  corr = Dot_Product(Dn, exc, L_subfr);
  if(corr > max) {
     max = corr;
     *pit_frac =  1;
  }
  else
    Copy(exc_tmp, exc, L_subfr);

  return t0;
}


/*---------------------------------------------------------------------*
 * Function  G_pitch:                                                  *
 *           ~~~~~~~~                                                  *
 *---------------------------------------------------------------------*
 * Compute correlations <xn,y1> and <y1,y1> to use in gains quantizer. *
 * Also compute the gain of pitch. Result in Q14                       *
 *  if (gain < 0)  gain =0                                             *
 *  if (gain >1.2) gain =1.2                                           *
 *---------------------------------------------------------------------*/
static Word16 G_pitch(      /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
  Word16 xn[],       /* (i)     : Pitch target.                            */
  Word16 y1[],       /* (i)     : Filtered adaptive codebook.              */
  Word16 g_coeff[],  /* (i)     : Correlations need for gain quantization. */
  Word16 L_subfr     /* (i)     : Length of subframe.                      */
)
{
   Word16 i;
   Word16 xy, yy, exp_xy, exp_yy, gain;
   Word32 s, s1, L_temp;

   //Word16 scaled_y1[L_SUBFR];
   Word16 scaled_y1;

   s = 1; /* Avoid case of all zeros */
   for(i=0; i<L_subfr; i++)
   {
     /* divide "y1[]" by 4 to avoid overflow */
     //scaled_y1[i] = y1[i] >> 2;
     /* Compute scalar product <y1[],y1[]> */
     s += y1[i] * y1[i] << 1;
     if (s < 0)
       break;
   }

   if (i == L_subfr) {
     exp_yy = norm_l(s);
     yy     = g_round( L_shl(s, exp_yy) );
   }
   else {
     //for(; i<L_subfr; i++)
       /* divide "y1[]" by 4 to avoid overflow */
       //scaled_y1[i] = y1[i] >> 2;

     s = 0;
     for(i=0; i<L_subfr; i++)
     {
       /* divide "y1[]" by 4 to avoid overflow */
       scaled_y1 = y1[i] >> 2;
       //s += scaled_y1[i] * scaled_y1[i];
       s += scaled_y1 * scaled_y1;
     }
     s <<= 1;
     s++; /* Avoid case of all zeros */

     exp_yy = norm_l(s);
     yy     = g_round( L_shl(s, exp_yy) );
     exp_yy = exp_yy - 4;
   }

   /* Compute scalar product <xn[],y1[]> */
   s = 0;
   for(i=0; i<L_subfr; i++)
   {
     L_temp = xn[i] * y1[i];
     if (L_temp == 0x40000000)
       break;
     s1 = s;
     s = (L_temp << 1) + s1;

     if (((s1 ^ L_temp) > 0) && ((s ^ s1) < 0))
       break;
     //s = L_mac(s, xn[i], y1[i]);
   }

   if (i == L_subfr) {
     exp_xy = norm_l(s);
     xy     = g_round( L_shl(s, exp_xy) );
   }
   else {
     s = 0;
     for(i=0; i<L_subfr; i++)
       //s += xn[i] * scaled_y1[i];
       s += xn[i] * (y1[i] >> 2);
     s <<= 1;
     exp_xy = norm_l(s);
     xy     = g_round( L_shl(s, exp_xy) );
     exp_xy = exp_xy - 2;
   }

   g_coeff[0] = yy;
   g_coeff[1] = 15 - exp_yy;
   g_coeff[2] = xy;
   g_coeff[3] = 15 - exp_xy;

   /* If (xy <= 0) gain = 0 */


   if (xy <= 0)
   {
      g_coeff[3] = -15;   /* Force exp_xy to -15 = (15-30) */
      return( (Word16) 0);
   }

   /* compute gain = xy/yy */

   xy >>= 1;             /* Be sure xy < yy */
   gain = div_s( xy, yy);

   i = exp_xy - exp_yy;
   gain = shr(gain, i);         /* saturation if > 1.99 in Q14 */

   /* if(gain >1.2) gain = 1.2  in Q14 */

   if (gain > 19661)
   {
     gain = 19661;
   }


   return(gain);
}



/*----------------------------------------------------------------------*
 *    Function Enc_lag3                                                 *
 *             ~~~~~~~~                                                 *
 *   Encoding of fractional pitch lag with 1/3 resolution.              *
 *----------------------------------------------------------------------*
 * The pitch range for the first subframe is divided as follows:        *
 *   19 1/3  to   84 2/3   resolution 1/3                               *
 *   85      to   143      resolution 1                                 *
 *                                                                      *
 * The period in the first subframe is encoded with 8 bits.             *
 * For the range with fractions:                                        *
 *   index = (T-19)*3 + frac - 1;   where T=[19..85] and frac=[-1,0,1]  *
 * and for the integer only range                                       *
 *   index = (T - 85) + 197;        where T=[86..143]                   *
 *----------------------------------------------------------------------*
 * For the second subframe a resolution of 1/3 is always used, and the  *
 * search range is relative to the lag in the first subframe.           *
 * If t0 is the lag in the first subframe then                          *
 *  t_min=t0-5   and  t_max=t0+4   and  the range is given by           *
 *       t_min - 2/3   to  t_max + 2/3                                  *
 *                                                                      *
 * The period in the 2nd subframe is encoded with 5 bits:               *
 *   index = (T-(t_min-1))*3 + frac - 1;    where T[t_min-1 .. t_max+1] *
 *----------------------------------------------------------------------*/
static Word16 Enc_lag3(     /* output: Return index of encoding */
  Word16 T0,         /* input : Pitch delay              */
  Word16 T0_frac,    /* input : Fractional pitch delay   */
  Word16 *T0_min,    /* in/out: Minimum search delay     */
  Word16 *T0_max,    /* in/out: Maximum search delay     */
  Word16 pit_min,    /* input : Minimum pitch delay      */
  Word16 pit_max,    /* input : Maximum pitch delay      */
  Word16 pit_flag    /* input : Flag for 1st subframe    */
)
{
  Word16 index; //, i;

  if (pit_flag == 0)   /* if 1st subframe */
  {
    /* encode pitch delay (with fraction) */

    if (T0 <= 85)
    {
      /* index = t0*3 - 58 + t0_frac   */
      //i = T0 + (T0 << 1);
      //index = i - 58 + T0_frac;
      index = T0*3 - 58 + T0_frac;
    }
    else
    {
      index = T0 + 112;
    }

    /* find T0_min and T0_max for second subframe */
    *T0_min = T0 - 5;
    if (*T0_min < pit_min)
    {
      *T0_min = pit_min;
    }

    *T0_max = *T0_min + 9;
    if (*T0_max > pit_max)
    {
      *T0_max = pit_max;
      *T0_min = *T0_max - 9;
    }
  }
  else      /* if second subframe */
  {
    /* i = t0 - t0_min;               */
    /* index = i*3 + 2 + t0_frac;     */
    //i = T0 - *T0_min;
    //i = i + (i << 1);
    //index = i + 2 + T0_frac;
    index = (T0 - *T0_min)*3 + 2 + T0_frac;
  }

  return index;
}
/************** End of file pitch_a.c ***************************************/

/************** Begin of file post_pro.c ************************************/
/*------------------------------------------------------------------------*
 * Function Post_Process()                                                *
 *                                                                        *
 * Post-processing of output speech.                                      *
 *   - 2nd order high pass filter with cut off frequency at 100 Hz.       *
 *   - Multiplication by two of output speech with saturation.            *
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 100 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]   + b[1]*x[i-1]   + b[2]*x[i-2]                      *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.93980581E+00, -0.18795834E+01, 0.93980581E+00};          *
 *     a[3] = {0.10000000E+01, 0.19330735E+01, -0.93589199E+00};          *
 *-----------------------------------------------------------------------*/

/* Initialization of static values */

static void Init_Post_Process(g729a_post_process_state *state)
{
  state->y2_hi = 0;
  state->y2_lo = 0;
  state->y1_hi = 0;
  state->y1_lo = 0;
//  state->y1 = 0LL;
//  state->y2 = 0LL;
  state->x1 = 0;
  state->x2 = 0;
}

/* acelp_high_pass_filter */
static void Post_Process(
  g729a_post_process_state *state,
  Word16 sigin[],    /* input signal */
  Word16 sigout[],   /* output signal */
  Word16 lg)         /* length of signal    */
{
  Word16 i;
  Word32 L_tmp;

  for(i=0; i<lg; i++)
  {
     /*  y[i] = b[0]*x[i]   + b[1]*x[i-1]   + b[2]*x[i-2]    */
     /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */
     L_tmp  = ((Word32) state->y1_hi) * 15836;
     L_tmp += (Word32)(((Word32) state->y1_lo * 15836) >> 15);
     L_tmp += ((Word32) state->y2_hi) * (-7667);
     L_tmp += (Word32)(((Word32) state->y2_lo * (-7667)) >> 15);
     L_tmp += 7699 * (sigin[i] - 2*state->x1/*signal[i-1]*/ + state->x2/*signal[i-2]*/);
    //L_tmp  = (state->y1 * 15836) >> 16;
    //L_tmp += (state->y2 * -7667) >> 16;
    //L_tmp += 7699 * (signal[i] - 2*state->x1/*signal[i-1]*/ + state->x2/*signal[i-2]*/);
    L_tmp  = L_shl(L_tmp, 3);

    state->x2 = state->x1;
    state->x1 = sigin[i];

     /* Multiplication by two of output speech with saturation. */
    sigout[i] = g_round(L_shl(L_tmp, 1));

    state->y2_hi = state->y1_hi;
    state->y2_lo = state->y1_lo;
    state->y1_hi = (Word16) (L_tmp >> 16);
    state->y1_lo = (Word16)((L_tmp >> 1) - (state->y1_hi << 15));
    //state->y2 = state->y1;
    //state->y1 = state->L_tmp;
  }
}
/************** End of file post_pro.c **************************************/

/************** Begin of file postfilt.c ************************************/

/*------------------------------------------------------------------------*
 *                         POSTFILTER.C                                   *
 *------------------------------------------------------------------------*
 * Performs adaptive postfiltering on the synthesis speech                *
 * This file contains all functions related to the post filter.           *
 *------------------------------------------------------------------------*/

/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "ld8a.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   GAMMA2_PST  : Formant postfiltering factor (numerator)      *
 *   GAMMA1_PST  : Formant postfiltering factor (denominator)    *
 *   GAMMAP      : Harmonic postfiltering factor                 *
 *   MU          : Factor for tilt compensation filter           *
 *   AGC_FAC     : Factor for automatic gain control             *
 *---------------------------------------------------------------*/

/*---------------------------------------------------------------*
 * Procedure    Init_Post_Filter:                                *
 *              ~~~~~~~~~~~~~~~~                                 *
 *  Initializes the postfilter parameters:                       *
 *---------------------------------------------------------------*/

static void Init_Post_Filter(g729a_post_filter_state *state)
{
  state->res2  = state->res2_buf + PIT_MAX;
  state->scal_res2  = state->scal_res2_buf + PIT_MAX;

  Set_zero(state->mem_syn_pst, M);
  Set_zero(state->res2_buf, PIT_MAX+L_SUBFR);
  Set_zero(state->scal_res2_buf, PIT_MAX+L_SUBFR);
}

/*------------------------------------------------------------------------*
 *  Procedure     Post_Filter:                                            *
 *                ~~~~~~~~~~~                                             *
 *------------------------------------------------------------------------*
 *  The postfiltering process is described as follows:                    *
 *                                                                        *
 *  - inverse filtering of syn[] through A(z/GAMMA2_PST) to get res2[]    *
 *  - use res2[] to compute pitch parameters                              *
 *  - perform pitch postfiltering                                         *
 *  - tilt compensation filtering; 1 - MU*k*z^-1                          *
 *  - synthesis filtering through 1/A(z/GAMMA1_PST)                       *
 *  - adaptive gain control                                               *
 *------------------------------------------------------------------------*/



static void Post_Filter(
  g729a_post_filter_state *state,
  Word16 *syn,       /* in/out: synthesis speech (postfiltered is output)    */
  Word16 *Az_4,      /* input : interpolated LPC parameters in all subframes */
  Word16 *T          /* input : decoded pitch lags in all subframes          */
)
{
 /*-------------------------------------------------------------------*
  *           Declaration of parameters                               *
  *-------------------------------------------------------------------*/

 Word16 res2_pst[L_SUBFR];  /* res2[] after pitch postfiltering */
 Word16 syn_pst[L_FRAME];   /* post filtered synthesis speech   */

 Word16 Ap3[MP1], Ap4[MP1];  /* bandwidth expanded LP parameters */

 Word16 *Az;                 /* pointer to Az_4:                 */
                             /*  LPC parameters in each subframe */
 Word16   t0_max, t0_min;    /* closed-loop pitch search range   */
 Word16   i_subfr;           /* index for beginning of subframe  */

 Word16 h[L_H];

 Word16  i, j;
 Word16  temp1, temp2;
 Word32  L_tmp1, L_tmp2;

   /*-----------------------------------------------------*
    * Post filtering                                      *
    *-----------------------------------------------------*/

    Az = Az_4;

    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
      /* Find pitch range t0_min - t0_max */
      t0_min = *T++ - 3;
      t0_max = t0_min + 6;
      if (t0_max > PIT_MAX) {
        t0_max = PIT_MAX;
        t0_min = t0_max - 6;
      }

      /* Find weighted filter coefficients Ap3[] and ap[4] */

      Weight_Az(Az, GAMMA2_PST, M, Ap3);
      Weight_Az(Az, GAMMA1_PST, M, Ap4);

      /* filtering of synthesis speech by A(z/GAMMA2_PST) to find res2[] */

      Residu(Ap3, &syn[i_subfr], state->res2, L_SUBFR);

      /* scaling of "res2[]" to avoid energy overflow */

      for (j=0; j<L_SUBFR; j++)
        state->scal_res2[j] = state->res2[j] >> 2;

      /* pitch postfiltering */

      pit_pst_filt(state->res2, state->scal_res2, t0_min, t0_max, L_SUBFR, res2_pst);

      /* tilt compensation filter */

      /* impulse response of A(z/GAMMA2_PST)/A(z/GAMMA1_PST) */

      Copy(Ap3, h, M+1);
      Set_zero(&h[M+1], L_H-M-1);
      Syn_filt(Ap4, h, h, L_H, &h[M+1], 0);

      /* 1st correlation of h[] */
      L_tmp1 = h[L_H-1] * h[L_H-1];
      L_tmp2 = 0;
      for (i=0; i<L_H-1; i++)
      {
        L_tmp1 += h[i] * h[i];
        L_tmp2 += h[i] * h[i+1];
      }
      temp1 = L_tmp1 >> 15;
      temp2 = L_tmp2 >> 15;

      if(temp2 <= 0) {
        temp2 = 0;
      }
      else {
        temp2 = mult(temp2, MU);
        temp2 = div_s(temp2, temp1);
      }

      preemphasis(res2_pst, temp2, L_SUBFR);

      /* filtering through  1/A(z/GAMMA1_PST) */

      Syn_filt(Ap4, res2_pst, &syn_pst[i_subfr], L_SUBFR, state->mem_syn_pst, 1);

      /* scale output to input */

      agc(&syn[i_subfr], &syn_pst[i_subfr], L_SUBFR);

      /* update res2[] buffer;  shift by L_SUBFR */

      Copy(&(state->res2[L_SUBFR-PIT_MAX]), &(state->res2[-PIT_MAX]), PIT_MAX);
      Copy(&(state->scal_res2[L_SUBFR-PIT_MAX]), &(state->scal_res2[-PIT_MAX]), PIT_MAX);

      Az += MP1;
    }

    /* update syn[] buffer */

    Copy(&syn[L_FRAME-M], &syn[-M], M);

    /* overwrite synthesis speech by postfiltered synthesis speech */

    Copy(syn_pst, syn, L_FRAME);
}

/*---------------------------------------------------------------------------*
 * procedure pitch_pst_filt                                                  *
 * ~~~~~~~~~~~~~~~~~~~~~~~~                                                  *
 * Find the pitch period  around the transmitted pitch and perform           *
 * harmonic postfiltering.                                                   *
 * Filtering through   (1 + g z^-T) / (1+g) ;   g = min(pit_gain*gammap, 1)  *
 *--------------------------------------------------------------------------*/

static void pit_pst_filt(
  Word16 *signal,      /* (i)     : input signal                        */
  Word16 *scal_sig,    /* (i)     : input signal (scaled, divided by 4) */
  Word16 t0_min,       /* (i)     : minimum value in the searched range */
  Word16 t0_max,       /* (i)     : maximum value in the searched range */
  Word16 L_subfr,      /* (i)     : size of filtering                   */
  Word16 *signal_pst   /* (o)     : harmonically postfiltered signal    */
)
{
  Word16 i, j, t0;
  Word16 g0, gain, cmax, en, en0;
  Word16 *p, *p1, *deb_sig;
  Word32 corr, cor_max, ener, ener0, temp;

/*---------------------------------------------------------------------------*
 * Compute the correlations for all delays                                   *
 * and select the delay which maximizes the correlation                      *
 *---------------------------------------------------------------------------*/

  deb_sig = &scal_sig[-t0_min];
  cor_max = MIN_32;
  t0 = t0_min;             /* Only to remove warning from some compilers */
  for (i=t0_min; i<=t0_max; i++)
  {
    corr = 0;
    p    = scal_sig;
    p1   = deb_sig;
    for (j=0; j<L_subfr; j++)
      corr += *p++ * *p1++;
    corr <<= 1;

    if (corr > cor_max)
    {
      cor_max = corr;
      t0 = i;
    }
    deb_sig--;
  }

  /* Compute the signal energy in the present subframe */
  ener0 = 0;
  /* Compute the energy of the signal delayed by t0 */
  ener = 0;
  for ( i=0; i<L_subfr; i++)
  {
    ener0 += scal_sig[i   ]*scal_sig[i   ];
    ener  += scal_sig[i-t0]*scal_sig[i-t0];
  }
  ener0 <<= 1; ener0 += 1;
  ener  <<= 1; ener  += 1;

  if (cor_max < 0)
    cor_max = 0;

  /* scale "cor_max", "ener" and "ener0" on 16 bits */

  temp = cor_max;
  if (ener > temp)
  {
    temp = ener;
  }
  if (ener0 > temp)
  {
    temp = ener0;
  }
  j = norm_l(temp);
  cmax = g_round(L_shl(cor_max, j));
  en = g_round(L_shl(ener, j));
  en0 = g_round(L_shl(ener0, j));

  /* prediction gain (dB)= -10 log(1-cor_max*cor_max/(ener*ener0)) */

  /* temp = (cor_max * cor_max) - (0.5 * ener * ener0)  */
  temp = (Word32)cmax * (Word32)cmax - ((Word32)en * (Word32)en0 >> 1);

  if (temp < (Word32)0)           /* if prediction gain < 3 dB   */
  {                               /* switch off pitch postfilter */
		Copy(signal, signal_pst, L_subfr);
    return;
  }

  if (cmax > en)      /* if pitch gain > 1 */
  {
    g0 = INV_GAMMAP;
    gain = GAMMAP_2;
  }
  else {
    /* cmax(Q14) = cmax(Q15) * GAMMAP */
    cmax = (Word16)((Word32)cmax * (Word32)GAMMAP >> 16);
    i = cmax + (en >> 1);  /* Q14 */
    if(i > 0)
    {
      gain = div_s(cmax, i);    /* gain(Q15) = cor_max/(cor_max+ener)  */
      g0 = 32767 - gain;    /* g0(Q15) = 1 - gain */
    }
    else
    {
      g0 =  32767;
      gain = 0;
    }
  }


  for (i = 0; i < L_subfr; i++)
  {
    /* signal_pst[i] = g0*signal[i] + gain*signal[i-t0]; */
		signal_pst[i] = (Word16)(((Word32)g0    * (Word32)signal[i])    >> 15) +
                    (Word16)(((Word32)gain * (Word32)signal[i-t0]) >> 15);
  }
}


/*---------------------------------------------------------------------*
 * routine preemphasis()                                               *
 * ~~~~~~~~~~~~~~~~~~~~~                                               *
 * Preemphasis: filtering through 1 - g z^-1                           *
 *---------------------------------------------------------------------*/

static void preemphasis(
  Word16 *signal,  /* (i/o)   : input signal overwritten by the output */
  Word16 g,        /* (i) Q15 : preemphasis coefficient                */
  Word16 L         /* (i)     : size of filtering                      */
)
{
  static Word16 mem_pre = 0;
  Word16 temp, i;

  temp = signal[L-1];

  for (i = L - 1; i > 0; --i)
    signal[i] -= ((Word32)g * (Word32)signal[i-1]) >> 15;

  signal[0] -= ((Word32)g * (Word32)mem_pre) >> 15;

  mem_pre = temp;
}



/*----------------------------------------------------------------------*
 *   routine agc()                                                      *
 *   ~~~~~~~~~~~~~                                                      *
 * Scale the postfilter output on a subframe basis by automatic control *
 * of the subframe gain.                                                *
 *  gain[n] = AGC_FAC * gain[n-1] + (1 - AGC_FAC) g_in/g_out            *
 *----------------------------------------------------------------------*/

static void agc(
  Word16 *sig_in,   /* (i)     : postfilter input signal  */
  Word16 *sig_out,  /* (i/o)   : postfilter output signal */
  Word16 l_trm      /* (i)     : subframe size            */
)
{
  static Word16 past_gain=4096;         /* past_gain = 1.0 (Q12) */
  Word16 i, exp;
  Word16 gain_in, gain_out, g0, gain;                     /* Q12 */
  Word32 s;

  Word16 sig;

  /* calculate gain_out with exponent */
  s = 0;
  for(i=0; i<l_trm; i++)
  {
    sig = sig_out[i] >> 2;
    s = L_mac(s, sig, sig);
  }

  if (s == 0) {
    past_gain = 0;
    return;
  }
  exp = norm_l(s) - 1;
  gain_out = g_round(L_shl(s, exp));

  /* calculate gain_in with exponent */
  s = 0;
  for(i=0; i<l_trm; i++)
  {
    sig = sig_in[i] >> 2;
    s = L_mac(s, sig, sig);
  }

  if (s == 0) {
    g0 = 0;
  }
  else {
    i = norm_l(s);
    gain_in = g_round(L_shl(s, i));
    exp = exp - i;

   /*---------------------------------------------------*
    *  g0(Q12) = (1-AGC_FAC) * sqrt(gain_in/gain_out);  *
    *---------------------------------------------------*/

    s = L_deposit_l(div_s(gain_out,gain_in));   /* Q15 */
    s = L_shl(s, 7);           /* s(Q22) = gain_out / gain_in */
    s = L_shr(s, exp);         /* Q22, add exponent */

    /* i(Q12) = s(Q19) = 1 / sqrt(s(Q22)) */
    s = Inv_sqrt(s);           /* Q19 */
    i = g_round(L_shl(s,9));     /* Q12 */

    /* g0(Q12) = i(Q12) * (1-AGC_FAC)(Q15) */
    g0 = mult(i, AGC_FAC1);       /* Q12 */
  }

  /* compute gain(n) = AGC_FAC gain(n-1) + (1-AGC_FAC)gain_in/gain_out */
  /* sig_out(n) = gain(n) sig_out(n)                                   */

  gain = past_gain;
  for(i=0; i<l_trm; i++) {
    gain = ((Word32)gain * (Word32)AGC_FAC) >> 15;
    gain += g0;
    sig_out[i] = (Word32)sig_out[i] * (Word32)gain >> 12;
  }
  past_gain = gain;
}
/************** End of file postfilt.c **************************************/

/************** Begin of file pre_proc.c ************************************/
/*------------------------------------------------------------------------*
 * Function Pre_Process()                                                 *
 *                                                                        *
 * Preprocessing of input speech.                                         *
 *   - 2nd order high pass filter with cut off frequency at 140 Hz.       *
 *   - Divide input by two.                                               *
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 140 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2                    *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.92727435E+00, -0.18544941E+01, 0.92727435E+00};          *
 *     a[3] = {0.10000000E+01, 0.19059465E+01, -0.91140240E+00};          *
 *                                                                        *
 *  Input are divided by two in the filtering process.                    *
 *-----------------------------------------------------------------------*/

/* Initialization of static values */

static void Init_Pre_Process(g729a_pre_process_state *state)
{
  state->y2_hi = 0;
  state->y2_lo = 0;
  state->y1_hi = 0;
  state->y1_lo = 0;
 // state->y1 = 0;
 // state->y2 = 0;
  state->x1   = 0;
  state->x2   = 0;
}


static void Pre_Process(
  g729a_pre_process_state *state,
  Word16 sigin[],    /* input signal */
  Word16 sigout[],   /* output signal */
  Word16 lg)          /* length of signal    */
{
  Word16 i;
  Word32 L_tmp;
  Word32 L_temp;

  for(i=0; i<lg; i++)
  {
     /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b140[2]*x[i-2]/2  */
     /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */
     L_tmp     = ((Word32) state->y1_hi) * 7807;
     L_tmp    += (Word32)(((Word32) state->y1_lo * 7807) >> 15);
     //L_tmp = (y1 * 7807LL) >> 12;

     L_tmp    += ((Word32) state->y2_hi) * (-3733);
     L_tmp    += (Word32)(((Word32) state->y2_lo * (-3733)) >> 15);
     //L_tmp += (y2 * -3733LL) >> 12;

     L_tmp += 1899 * (sigin[i] - 2*state->x1/*signal[i-1]*/ + state->x2/*signal[i-2]*/);

     state->x2 = state->x1;
     state->x1 = sigin[i];
     //signal[i] = (Word16)((L_tmp + 0x800L) >> 12);

     state->y2_hi = state->y1_hi;
     state->y2_lo = state->y1_lo;
     //state->y2 = state->y1;

     L_temp = L_tmp;
     L_tmp = L_temp << 4;
     if (L_tmp >> 4 != L_temp)
     //y1 = L_tmp << 4;
     //if (y1 >> 4 != L_tmp)
     {
       sigout[i] = MIN_16;
       //y1 = (L_tmp & 0x80000000 ? MIN_32 : MAX_32);
       if (L_temp & 0x80000000)
       {
         state->y1_hi = MIN_16;
         state->y1_lo = 0x0000;
       }
       else
       {
         state->y1_hi = MAX_16;
         state->y1_lo = 0xffff;
       }
     }
     else
     {
       sigout[i] = (Word16)((L_tmp + 0x00008000) >> 16);
       state->y1_hi = (Word16) (L_tmp >> 16);
       state->y1_lo = (Word16)((L_tmp >> 1) - (state->y1_hi << 15));
     }
  }
}
/************** End of file pre_proc.c **************************************/

/************** Begin of file pred_lt3.c ************************************/
/*-------------------------------------------------------------------*
 * Function  Pred_lt_3()                                             *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractional        *
 * interpolation of resolution 1/3.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/


/*ff_acelp_interpolate / ff_acelp_weighted_vector_sum */
static void Pred_lt_3(
  Word16   exc[],       /* in/out: excitation buffer */
  Word16   T0,          /* input : integer pitch lag */
  Word16   frac,        /* input : fraction of lag   */
  Word16   L_subfr      /* input : subframe size     */
)
{
  Word16   i, j, k;
  Word16   *x0, *x1, *x2, *c1, *c2;
  Word32  s;

  x0 = &exc[-T0];

  if (frac > 0)
  {
    frac = UP_SAMP - frac;
    x0--;
  }
  else
    frac = -frac;

  for (j=0; j<L_subfr; j++)
  {
    x1 = x0++;
    x2 = x0;
    c1 = &inter_3l[frac];
    c2 = &inter_3l[UP_SAMP - frac];

    s = 0;
    for(i=0, k=0; i< L_INTER10; i++, k+=UP_SAMP)
    {
      s = L_add(s, (Word32)x1[-i] * (Word32)c1[k] << 1);
      s = L_add(s, (Word32)x2[i]  * (Word32)c2[k] << 1);
    }
    exc[j] = (s + 0x8000L) >> 16;
  }
}
/************** End of file pred_lt3.c **************************************/

/************** Begin of qua_gain.c *****************************************/
static void Gbk_presel(
   Word16 best_gain[],     /* (i) [0] Q9 : unquantized pitch gain     */
                           /* (i) [1] Q2 : unquantized code gain      */
   Word16 *cand1,          /* (o)    : index of best 1st stage vector */
   Word16 *cand2,          /* (o)    : index of best 2nd stage vector */
   Word16 gcode0           /* (i) Q4 : presearch for gain codebook    */
);


/*---------------------------------------------------------------------------*
 * Function  Qua_gain                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 * Inputs:                                                                   *
 *   code[]     :Innovative codebook.                                        *
 *   g_coeff[]  :Correlations compute for pitch.                             *
 *   L_subfr    :Subframe length.                                            *
 *                                                                           *
 * Outputs:                                                                  *
 *   gain_pit   :Quantized pitch gain.                                       *
 *   gain_cod   :Quantized code gain.                                        *
 *                                                                           *
 * Return:                                                                   *
 *   Index of quantization.                                                  *
 *                                                                           *
 *--------------------------------------------------------------------------*/
static Word16 Qua_gain(
   Word16 code[],       /* (i) Q13 :Innovative vector.             */
   Word16 g_coeff[],    /* (i)     :Correlations <xn y1> -2<y1 y1> */
                        /*            <y2,y2>, -2<xn,y2>, 2<y1,y2> */
   Word16 exp_coeff[],  /* (i)     :Q-Format g_coeff[]             */
   Word16 L_subfr,      /* (i)     :Subframe length.               */
   Word16 *gain_pit,    /* (o) Q14 :Pitch gain.                    */
   Word16 *gain_cod,    /* (o) Q1  :Code gain.                     */
   Word16 tameflag      /* (i)     : set to 1 if taming is needed  */
)
{
   Word16  i, j, index1, index2;
   Word16  cand1, cand2;
   Word16  exp, gcode0, exp_gcode0, gcode0_org, e_min ;
   Word16  nume, denom, inv_denom;
   Word16  exp1,exp2,exp_nume,exp_denom,exp_inv_denom,sft,tmp;
   Word16  g_pitch, g2_pitch, g_code, g2_code, g_pit_cod;
   Word16  coeff[5], coeff_lsf[5];
   Word16  exp_min[5];
   Word32  L_gbk12;
   Word32  L_tmp, L_dist_min, L_tmp1, L_tmp2, L_acc, L_accb;
   Word16  best_gain[2];

        /* Gain predictor, Past quantized energies = -14.0 in Q10 */

 static Word16 past_qua_en[4] = { -14336, -14336, -14336, -14336 };

  /*---------------------------------------------------*
   *-  energy due to innovation                       -*
   *-  predicted energy                               -*
   *-  predicted codebook gain => gcode0[exp_gcode0]  -*
   *---------------------------------------------------*/

   Gain_predict( past_qua_en, code, L_subfr, &gcode0, &exp_gcode0 );

  /*-----------------------------------------------------------------*
   *  pre-selection                                                  *
   *-----------------------------------------------------------------*/
  /*-----------------------------------------------------------------*
   *  calculate best gain                                            *
   *                                                                 *
   *  tmp = -1./(4.*coeff[0]*coeff[2]-coeff[4]*coeff[4]) ;           *
   *  best_gain[0] = (2.*coeff[2]*coeff[1]-coeff[3]*coeff[4])*tmp ;  *
   *  best_gain[1] = (2.*coeff[0]*coeff[3]-coeff[1]*coeff[4])*tmp ;  *
   *  gbk_presel(best_gain,&cand1,&cand2,gcode0) ;                   *
   *                                                                 *
   *-----------------------------------------------------------------*/

  /*-----------------------------------------------------------------*
   *  tmp = -1./(4.*coeff[0]*coeff[2]-coeff[4]*coeff[4]) ;           *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[0], g_coeff[2] );
   exp1   = add( add( exp_coeff[0], exp_coeff[2] ), 1-2 );
   L_tmp2 = L_mult( g_coeff[4], g_coeff[4] );
   exp2   = add( add( exp_coeff[4], exp_coeff[4] ), 1 );

   //if( sub(exp1, exp2)>0 ){
   if( exp1 > exp2 ){
      L_tmp = L_sub( L_shr( L_tmp1, sub(exp1,exp2) ), L_tmp2 );
      exp = exp2;
   }
   else{
      L_tmp = L_sub( L_tmp1, L_shr( L_tmp2, sub(exp2,exp1) ) );
      exp = exp1;
   }
   sft = norm_l( L_tmp );
   denom = extract_h( L_shl(L_tmp, sft) );
   exp_denom = sub( add( exp, sft ), 16 );

   inv_denom = div_s(16384,denom);
   inv_denom = negate( inv_denom );
   exp_inv_denom = sub( 14+15, exp_denom );

  /*-----------------------------------------------------------------*
   *  best_gain[0] = (2.*coeff[2]*coeff[1]-coeff[3]*coeff[4])*tmp ;  *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[2], g_coeff[1] );
   exp1   = add( exp_coeff[2], exp_coeff[1] );
   L_tmp2 = L_mult( g_coeff[3], g_coeff[4] );
   exp2   = add( add( exp_coeff[3], exp_coeff[4] ), 1 );

   //if( sub(exp1, exp2)>0 ){
   if (exp1 > exp2){
      L_tmp = L_sub( L_shr( L_tmp1, add(sub(exp1,exp2),1 )), L_shr( L_tmp2,1 ) );
      exp = sub(exp2,1);
   }
   else{
      L_tmp = L_sub( L_shr( L_tmp1,1 ), L_shr( L_tmp2, add(sub(exp2,exp1),1 )) );
      exp = sub(exp1,1);
   }
   sft = norm_l( L_tmp );
   nume = extract_h( L_shl(L_tmp, sft) );
   exp_nume = sub( add( exp, sft ), 16 );

   sft = sub( add( exp_nume, exp_inv_denom ), (9+16-1) );
   L_acc = L_shr( L_mult( nume,inv_denom ), sft );
   best_gain[0] = extract_h( L_acc );             /*-- best_gain[0]:Q9 --*/

   if (tameflag == 1){
     //if(sub(best_gain[0], GPCLIP2) > 0) best_gain[0] = GPCLIP2;
     if(best_gain[0] > GPCLIP2) best_gain[0] = GPCLIP2;
   }

  /*-----------------------------------------------------------------*
   *  best_gain[1] = (2.*coeff[0]*coeff[3]-coeff[1]*coeff[4])*tmp ;  *
   *-----------------------------------------------------------------*/
   L_tmp1 = L_mult( g_coeff[0], g_coeff[3] );
   exp1   = add( exp_coeff[0], exp_coeff[3] ) ;
   L_tmp2 = L_mult( g_coeff[1], g_coeff[4] );
   exp2   = add( add( exp_coeff[1], exp_coeff[4] ), 1 );

   //if( sub(exp1, exp2)>0 ){
   if( exp1 > exp2 ){
      L_tmp = L_sub( L_shr( L_tmp1, add(sub(exp1,exp2),1) ), L_shr( L_tmp2,1 ) );
      exp = sub(exp2,1);
      //exp = exp2--;
   }
   else{
      L_tmp = L_sub( L_shr( L_tmp1,1 ), L_shr( L_tmp2, add(sub(exp2,exp1),1) ) );
      exp = sub(exp1,1);
      //exp = exp1--;
   }
   sft = norm_l( L_tmp );
   nume = extract_h( L_shl(L_tmp, sft) );
   exp_nume = sub( add( exp, sft ), 16 );

   sft = sub( add( exp_nume, exp_inv_denom ), (2+16-1) );
   L_acc = L_shr( L_mult( nume,inv_denom ), sft );
   best_gain[1] = extract_h( L_acc );             /*-- best_gain[1]:Q2 --*/

   /*--- Change Q-format of gcode0 ( Q[exp_gcode0] -> Q4 ) ---*/
   //if( sub(exp_gcode0,4) >= 0 ){
   if (exp_gcode0 >=4) {
      gcode0_org = shr( gcode0, sub(exp_gcode0,4) );
   }
   else{
      L_acc = L_deposit_l( gcode0 );
      L_acc = L_shl( L_acc, sub( (4+16), exp_gcode0 ) );
      gcode0_org = extract_h( L_acc );              /*-- gcode0_org:Q4 --*/
   }

  /*----------------------------------------------*
   *   - presearch for gain codebook -            *
   *----------------------------------------------*/

   Gbk_presel(best_gain, &cand1, &cand2, gcode0_org );

/*---------------------------------------------------------------------------*
 *                                                                           *
 * Find the best quantizer.                                                  *
 *                                                                           *
 *  dist_min = MAX_32;                                                       *
 *  for ( i=0 ; i<NCAN1 ; i++ ){                                             *
 *    for ( j=0 ; j<NCAN2 ; j++ ){                                           *
 *      g_pitch = gbk1[cand1+i][0] + gbk2[cand2+j][0];                       *
 *      g_code = gcode0 * (gbk1[cand1+i][1] + gbk2[cand2+j][1]);             *
 *      dist = g_pitch*g_pitch * coeff[0]                                    *
 *           + g_pitch         * coeff[1]                                    *
 *           + g_code*g_code   * coeff[2]                                    *
 *           + g_code          * coeff[3]                                    *
 *           + g_pitch*g_code  * coeff[4] ;                                  *
 *                                                                           *
 *      if (dist < dist_min){                                                *
 *        dist_min = dist;                                                   *
 *        indice1 = cand1 + i ;                                              *
 *        indice2 = cand2 + j ;                                              *
 *      }                                                                    *
 *    }                                                                      *
 *  }                                                                        *
 *                                                                           *
 * g_pitch         = Q13                                                     *
 * g_pitch*g_pitch = Q11:(13+13+1-16)                                        *
 * g_code          = Q[exp_gcode0-3]:(exp_gcode0+(13-1)+1-16)                *
 * g_code*g_code   = Q[2*exp_gcode0-21]:(exp_gcode0-3+exp_gcode0-3+1-16)     *
 * g_pitch*g_code  = Q[exp_gcode0-5]:(13+exp_gcode0-3+1-16)                  *
 *                                                                           *
 * term 0: g_pitch*g_pitch*coeff[0] ;exp_min0 = 13             +exp_coeff[0] *
 * term 1: g_pitch        *coeff[1] ;exp_min1 = 14             +exp_coeff[1] *
 * term 2: g_code*g_code  *coeff[2] ;exp_min2 = 2*exp_gcode0-21+exp_coeff[2] *
 * term 3: g_code         *coeff[3] ;exp_min3 = exp_gcode0  - 3+exp_coeff[3] *
 * term 4: g_pitch*g_code *coeff[4] ;exp_min4 = exp_gcode0  - 4+exp_coeff[4] *
 *                                                                           *
 *---------------------------------------------------------------------------*/

   exp_min[0] = add( exp_coeff[0], 13 );
   exp_min[1] = add( exp_coeff[1], 14 );
   exp_min[2] = add( exp_coeff[2], sub( shl( exp_gcode0, 1 ), 21 ) );
   exp_min[3] = add( exp_coeff[3], sub( exp_gcode0, 3 ) );
   exp_min[4] = add( exp_coeff[4], sub( exp_gcode0, 4 ) );

   e_min = exp_min[0];
   for(i=1; i<5; i++){
      //if( sub(exp_min[i], e_min) < 0 ){
     if (exp_min[i] < e_min) {
         e_min = exp_min[i];
      }
   }

   /* align coeff[] and save in special 32 bit double precision */

   for(i=0; i<5; i++){
     j = sub( exp_min[i], e_min );
     L_tmp = (Word32)g_coeff[i] << 16;
     L_tmp = L_shr( L_tmp, j );          /* L_tmp:Q[exp_g_coeff[i]+16-j] */
     L_Extract( L_tmp, &coeff[i], &coeff_lsf[i] );          /* DPF */
   }

   /* Codebook search */

   L_dist_min = MAX_32;

   /* initialization used only to suppress Microsoft Visual C++  warnings */
   index1 = cand1;
   index2 = cand2;

if(tameflag == 1){
   for(i=0; i<NCAN1; i++){
      for(j=0; j<NCAN2; j++){
         g_pitch = add( gbk1[cand1+i][0], gbk2[cand2+j][0] );     /* Q14 */
         if(g_pitch < GP0999) {
         L_acc = L_deposit_l( gbk1[cand1+i][1] );
         L_accb = L_deposit_l( gbk2[cand2+j][1] );                /* Q13 */
         L_tmp = L_add( L_acc,L_accb );
         tmp = extract_l( L_shr( L_tmp,1 ) );                     /* Q12 */

         g_code   = mult( gcode0, tmp );         /*  Q[exp_gcode0+12-15] */
         g2_pitch = mult(g_pitch, g_pitch);                       /* Q13 */
         g2_code  = mult(g_code,  g_code);       /* Q[2*exp_gcode0-6-15] */
         g_pit_cod= mult(g_code,  g_pitch);      /* Q[exp_gcode0-3+14-15] */

         L_tmp = Mpy_32_16(coeff[0], coeff_lsf[0], g2_pitch);
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[2], coeff_lsf[2], g2_code) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[3], coeff_lsf[3], g_code) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod) );
         L_tmp += Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch);
         L_tmp += Mpy_32_16(coeff[2], coeff_lsf[2], g2_code);
         L_tmp += Mpy_32_16(coeff[3], coeff_lsf[3], g_code);
         L_tmp += Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod);

         //L_temp = L_sub(L_tmp, L_dist_min);

         //if( L_temp < 0L ){
         if( L_tmp < L_dist_min ){
            L_dist_min = L_tmp;
            index1 = add(cand1,i);
            index2 = add(cand2,j);
         }
        }
      }
   }

}
else{
   for(i=0; i<NCAN1; i++){
      for(j=0; j<NCAN2; j++){
         g_pitch = add( gbk1[cand1+i][0], gbk2[cand2+j][0] );     /* Q14 */
         L_acc = L_deposit_l( gbk1[cand1+i][1] );
         L_accb = L_deposit_l( gbk2[cand2+j][1] );                /* Q13 */
         L_tmp = L_add( L_acc,L_accb );
         tmp = extract_l( L_shr( L_tmp,1 ) );                     /* Q12 */

         g_code   = mult( gcode0, tmp );         /*  Q[exp_gcode0+12-15] */
         g2_pitch = mult(g_pitch, g_pitch);                       /* Q13 */
         g2_code  = mult(g_code,  g_code);       /* Q[2*exp_gcode0-6-15] */
         g_pit_cod= mult(g_code,  g_pitch);      /* Q[exp_gcode0-3+14-15] */

         L_tmp = Mpy_32_16(coeff[0], coeff_lsf[0], g2_pitch);
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[2], coeff_lsf[2], g2_code) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[3], coeff_lsf[3], g_code) );
         //L_tmp = L_add(L_tmp, Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod) );
         L_tmp += Mpy_32_16(coeff[1], coeff_lsf[1], g_pitch);
         L_tmp += Mpy_32_16(coeff[2], coeff_lsf[2], g2_code);
         L_tmp += Mpy_32_16(coeff[3], coeff_lsf[3], g_code);
         L_tmp += Mpy_32_16(coeff[4], coeff_lsf[4], g_pit_cod);

         if( L_tmp < L_dist_min ){
            L_dist_min = L_tmp;
            index1 = add(cand1,i);
            index2 = add(cand2,j);
         }

      }
   }
}
   /* Read the quantized gain */

  /*-----------------------------------------------------------------*
   * *gain_pit = gbk1[indice1][0] + gbk2[indice2][0];                *
   *-----------------------------------------------------------------*/
   *gain_pit = add( gbk1[index1][0], gbk2[index2][0] );      /* Q14 */

  /*-----------------------------------------------------------------*
   * *gain_code = (gbk1[indice1][1]+gbk2[indice2][1]) * gcode0;      *
   *-----------------------------------------------------------------*/
   L_gbk12 = (Word32)gbk1[index1][1] + (Word32)gbk2[index2][1]; /* Q13 */
   tmp = extract_l( L_shr( L_gbk12,1 ) );                     /* Q12 */
   L_acc = L_mult(tmp, gcode0);                /* Q[exp_gcode0+12+1] */

   L_acc = L_shl(L_acc, add( negate(exp_gcode0),(-12-1+1+16) ));
   *gain_cod = extract_h( L_acc );                             /* Q1 */

  /*----------------------------------------------*
   * update table of past quantized energies      *
   *----------------------------------------------*/
   Gain_update( past_qua_en, L_gbk12 );

   return( add( map1[index1]*(Word16)NCODE2, map2[index2] ) );

}
/*---------------------------------------------------------------------------*
 * Function Gbk_presel                                                       *
 * ~~~~~~~~~~~~~~~~~~~                                                       *
 *   - presearch for gain codebook -                                         *
 *---------------------------------------------------------------------------*/
static void Gbk_presel(
   Word16 best_gain[],     /* (i) [0] Q9 : unquantized pitch gain     */
                           /* (i) [1] Q2 : unquantized code gain      */
   Word16 *cand1,          /* (o)    : index of best 1st stage vector */
   Word16 *cand2,          /* (o)    : index of best 2nd stage vector */
   Word16 gcode0           /* (i) Q4 : presearch for gain codebook    */
)
{
   Word16    acc_h;
   Word16    sft_x,sft_y;
   Word32    L_acc,L_preg,L_cfbg,L_tmp,L_tmp_x,L_tmp_y;
   Word32 L_temp;

 /*--------------------------------------------------------------------------*
   x = (best_gain[1]-(coef[0][0]*best_gain[0]+coef[1][1])*gcode0) * inv_coef;
  *--------------------------------------------------------------------------*/
   L_cfbg = L_mult( coef[0][0], best_gain[0] );        /* L_cfbg:Q20 -> !!y */
   L_acc = L_shr( L_coef[1][1], 15 );                  /* L_acc:Q20     */
   L_acc = L_add( L_cfbg , L_acc );
   acc_h = extract_h( L_acc );                         /* acc_h:Q4      */
   L_preg = L_mult( acc_h, gcode0 );                   /* L_preg:Q9     */
   L_acc = L_shl( L_deposit_l( best_gain[1] ), 7 );    /* L_acc:Q9      */
   L_acc = L_sub( L_acc, L_preg );
   acc_h = extract_h( L_shl( L_acc,2 ) );              /* L_acc_h:Q[-5] */
   L_tmp_x = L_mult( acc_h, INV_COEF );                /* L_tmp_x:Q15   */

 /*--------------------------------------------------------------------------*
   y = (coef[1][0]*(-coef[0][1]+best_gain[0]*coef[0][0])*gcode0
                                      -coef[0][0]*best_gain[1]) * inv_coef;
  *--------------------------------------------------------------------------*/
   L_acc = L_shr( L_coef[0][1], 10 );                  /* L_acc:Q20   */
   L_acc = L_sub( L_cfbg, L_acc );                     /* !!x -> L_cfbg:Q20 */
   acc_h = extract_h( L_acc );                         /* acc_h:Q4    */
   acc_h = mult( acc_h, gcode0 );                      /* acc_h:Q[-7] */
   L_tmp = L_mult( acc_h, coef[1][0] );                /* L_tmp:Q10   */

   L_preg = L_mult( coef[0][0], best_gain[1] );        /* L_preg:Q13  */
   L_acc = L_sub( L_tmp, L_shr(L_preg,3) );            /* L_acc:Q10   */

   acc_h = extract_h( L_shl( L_acc,2 ) );              /* acc_h:Q[-4] */
   L_tmp_y = L_mult( acc_h, INV_COEF );                /* L_tmp_y:Q16 */

   sft_y = (14+4+1)-16;         /* (Q[thr1]+Q[gcode0]+1)-Q[L_tmp_y] */
   sft_x = (15+4+1)-15;         /* (Q[thr2]+Q[gcode0]+1)-Q[L_tmp_x] */

   if(gcode0>0){
      /*-- pre select codebook #1 --*/
      *cand1 = 0 ;
      do{
         L_temp = L_sub( L_tmp_y, L_shr(L_mult(thr1[*cand1],gcode0),sft_y));
         if(L_temp >0L  ){
        //(*cand1) =add(*cand1,1);
           *cand1 += 1;
     }
         else               break ;
      //} while(sub((*cand1),(NCODE1-NCAN1))<0) ;
      } while ((*cand1) < (NCODE1-NCAN1));
      /*-- pre select codebook #2 --*/
      *cand2 = 0 ;
      do{
        L_temp = L_sub( L_tmp_x , L_shr(L_mult(thr2[*cand2],gcode0),sft_x));
         if( L_temp >0L) {
        //(*cand2) =add(*cand2,1);
           *cand2 += 1;
     }
         else               break ;
      //} while(sub((*cand2),(NCODE2-NCAN2))<0) ;
      } while((*cand2) < (NCODE2-NCAN2)) ;
   }
   else{
      /*-- pre select codebook #1 --*/
      *cand1 = 0 ;
      do{
        L_temp = L_sub(L_tmp_y ,L_shr(L_mult(thr1[*cand1],gcode0),sft_y));
         if( L_temp <0L){
        //(*cand1) =add(*cand1,1);
           *cand1 += 1;
     }
         else               break ;
      //} while(sub((*cand1),(NCODE1-NCAN1))) ;
      } while (*cand1 != (NCODE1-NCAN1)) ;
      /*-- pre select codebook #2 --*/
      *cand2 = 0 ;
      do{
         L_temp =L_sub(L_tmp_x ,L_shr(L_mult(thr2[*cand2],gcode0),sft_x));
         if( L_temp <0L){
        //(*cand2) =add(*cand2,1);
           *cand2 += 1;
     }
         else               break ;
      //} while(sub( (*cand2),(NCODE2-NCAN2))) ;
      } while(*cand2 != (NCODE2-NCAN2)) ;
   }

   return ;
}
/************** End of qua_gain.c *******************************************/

/************** Begin of qua_lsp.c ******************************************/
/*-------------------------------------------------------------------*
 * Function  Qua_lsp:                                                *
 *           ~~~~~~~~                                                *
 *-------------------------------------------------------------------*/

static void Lsp_qua_cs   (g729a_encoder_state *state, Word16 flsp_in[M], Word16 lspq_out[M], Word16 *code);
static void Relspwed     (Word16 lsp[], Word16 wegt[], Word16 lspq[],
                          Word16 lspcb1[][M], Word16 lspcb2[][M],
                          Word16 fg[MODE][MA_NP][M], Word16 freq_prev[MA_NP][M],
                          Word16 fg_sum[MODE][M], Word16 fg_sum_inv[MODE][M],
                          Word16 code_ana[]);
static void Get_wegt     (Word16 flsp[], Word16 wegt[]);
static void Lsp_get_tdist(Word16 wegt[], Word16 buf[], Word32 *L_tdist,
                          Word16 rbuf[], Word16 fg_sum[]);
static void Lsp_pre_select(Word16 rbuf[], Word16 lspcb1[][M], Word16 *cand);

static void Qua_lsp(
             g729a_encoder_state *state,
  Word16 lsp[],       /* (i) Q15 : Unquantized LSP            */
  Word16 lsp_q[],     /* (o) Q15 : Quantized LSP              */
  Word16 ana[]        /* (o)     : indexes                    */
)
{
  Word16 lsf[M], lsf_q[M];  /* domain 0.0<= lsf <PI in Q13 */

  /* Convert LSPs to LSFs */
  Lsp_lsf2(lsp, lsf, M);

  Lsp_qua_cs(state, lsf, lsf_q, ana );

  /* Convert LSFs to LSPs */
  Lsf_lsp2(lsf_q, lsp_q, M);
}

#if 0
/* static memory */
static Word16 freq_prev_reset[M] = {  /* Q13:previous LSP vector(init) */
  2339, 4679, 7018, 9358, 11698, 14037, 16377, 18717, 21056, 23396
};     /* PI*(float)(j+1)/(float)(M+1) */
#endif

static void Lsp_encw_reset(
  g729a_encoder_state *state
)
{
  Word16 i;

  for(i=0; i<MA_NP; i++)
    Copy( &freq_prev_reset[0], &(state->freq_prev[i][0]), M );
}


static void Lsp_qua_cs(
                g729a_encoder_state *state,
  Word16 flsp_in[M],    /* (i) Q13 : Original LSP parameters    */
  Word16 lspq_out[M],   /* (o) Q13 : Quantized LSP parameters   */
  Word16 *code          /* (o)     : codes of the selected LSP  */
)
{
  Word16 wegt[M];       /* Q11->normalized : weighting coefficients */

  Get_wegt( flsp_in, wegt );

  Relspwed( flsp_in, wegt, lspq_out, lspcb1, lspcb2, fg,
    state->freq_prev, fg_sum, fg_sum_inv, code);
}

static void Relspwed(
  Word16 lsp[],                 /* (i) Q13 : unquantized LSP parameters */
  Word16 wegt[],                /* (i) norm: weighting coefficients     */
  Word16 lspq[],                /* (o) Q13 : quantized LSP parameters   */
  Word16 lspcb1[][M],           /* (i) Q13 : first stage LSP codebook   */
  Word16 lspcb2[][M],           /* (i) Q13 : Second stage LSP codebook  */
  Word16 fg[MODE][MA_NP][M],    /* (i) Q15 : MA prediction coefficients */
  Word16 freq_prev[MA_NP][M],   /* (i) Q13 : previous LSP vector        */
  Word16 fg_sum[MODE][M],       /* (i) Q15 : present MA prediction coef.*/
  Word16 fg_sum_inv[MODE][M],   /* (i) Q12 : inverse coef.              */
  Word16 code_ana[]             /* (o)     : codes of the selected LSP  */
)
{
  static const UWord8 gap[2]={GAP1, GAP2};
  Word16 mode, i, j;
  Word16 index1, index2, mode_index;
  Word16 cand[MODE], cand_cur;
  Word16 tindex1[MODE], tindex2[MODE];
  Word32 L_tdist[MODE];         /* Q26 */
  Word16 rbuf[M];               /* Q13 */
  Word16 buf[M];                /* Q13 */
  Word32 diff;

  Word32 L_dist1, L_dmin1;              /* Q26 */
  Word32 L_dist2, L_dmin2;              /* Q26 */
  Word16 tmp,tmp2;            /* Q13 */

  for(mode = 0; mode<MODE; mode++) {
    Lsp_prev_extract(lsp, rbuf, fg[mode], freq_prev, fg_sum_inv[mode]);

    Lsp_pre_select(rbuf, lspcb1, &cand_cur );
    cand[mode] = cand_cur;

 //   Lsp_select_1(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index1);
 //   Lsp_select_2(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index2);

    for ( j = 0 ; j < M ; j++ )
      buf[j] = rbuf[j] - lspcb1[cand_cur][j];

                     /* avoid the worst case. (all over flow) */
    index1 = 0;
    index2 = 0;
    L_dmin1 = MAX_32;
    L_dmin2 = MAX_32;
    for ( i = 0 ; i < NC1 ; i++ ) {
      L_dist1 = 0;
      L_dist2 = 0;
      for ( j = 0 ; j < NC ; j++ ) {
        tmp = sub(buf[j], lspcb2[i][j]);
        tmp2 = mult( wegt[j], tmp );
        L_dist1 += (Word32)tmp2 * tmp;
        tmp = sub(buf[j+NC], lspcb2[i][j+NC]);
        tmp2 = mult( wegt[j+NC], tmp );
        L_dist2 += (Word32)tmp2 * tmp;
      }
      L_dist1 <<= 1;
      L_dist2 <<= 1;

      if ( L_dist1 < L_dmin1) {
        L_dmin1 = L_dist1;
        index1 = i;
      }
      if ( L_dist2 < L_dmin2) {
        L_dmin2 = L_dist2;
        index2 = i;
      }
    }

    tindex1[mode] = index1;
    tindex2[mode] = index2;

    for( j = 0 ; j < NC ; j++ )
    {
      buf[j   ] = lspcb1[cand_cur][j   ] + lspcb2[index1][j];
      buf[j+NC] = lspcb1[cand_cur][j+NC] + lspcb2[index2][j+NC];
    }

    /* Lsp_expand_1_2 */
    for (i = 0; i < 2; ++i)
      for ( j = 1 ; j < M ; j++ )
      {
        diff = (buf[j-1] - buf[j] + gap[i]) >> 1;
        if ( diff > 0 )
        {
          buf[j-1] -= diff;
          buf[j]   += diff;
        }
      }

    Lsp_get_tdist(wegt, buf, &L_tdist[mode], rbuf, fg_sum[mode]);
  }

  //Lsp_last_select(L_tdist, &mode_index);
  mode_index = 0;
  if (L_tdist[1] < L_tdist[0])
    mode_index = 1;


  code_ana[0] = shl( mode_index,NC0_B ) | cand[mode_index];
  code_ana[1] = shl( tindex1[mode_index],NC1_B ) | tindex2[mode_index];

  Lsp_get_quant(lspcb1, lspcb2, cand[mode_index],
      tindex1[mode_index], tindex2[mode_index],
      fg[mode_index], freq_prev, lspq, fg_sum[mode_index]) ;
}

static void Lsp_pre_select(
  Word16 rbuf[],              /* (i) Q13 : target vetor             */
  Word16 lspcb1[][M],         /* (i) Q13 : first stage LSP codebook */
  Word16 *cand                /* (o)     : selected code            */
)
{
  Word16 i, j;
  Word16 tmp;                 /* Q13 */
  Word32 L_dmin;              /* Q26 */
  Word32 L_tmp;               /* Q26 */

  /* avoid the worst case. (all over flow) */

  *cand = 0;
  L_dmin = MAX_32;
  for ( i = 0 ; i < NC0 ; i++ ) {
    L_tmp = 0;
    for ( j = 0 ; j < M ; j++ ) {
      tmp = rbuf[j] - lspcb1[i][j];
      L_tmp += (Word32)tmp * (Word32)tmp;
    }
    L_tmp <<= 1;

    if (L_tmp < L_dmin) {
      L_dmin = L_tmp;
      *cand = i;
    }
  }
}

static void Lsp_get_tdist(
  Word16 wegt[],        /* (i) norm: weight coef.                */
  Word16 buf[],         /* (i) Q13 : candidate LSP vector        */
  Word32 *L_tdist,      /* (o) Q27 : distortion                  */
  Word16 rbuf[],        /* (i) Q13 : target vector               */
  Word16 fg_sum[]       /* (i) Q15 : present MA prediction coef. */
)
{
  Word16 j;
  Word16 tmp, tmp2;     /* Q13 */
  Word32 L_acc;         /* Q25 */

  *L_tdist = 0;
  for ( j = 0 ; j < M ; j++ ) {
    /* tmp = (buf - rbuf)*fg_sum */
    //tmp = sub( buf[j], rbuf[j] );
    //tmp = mult( tmp, fg_sum[j] );
    tmp = mult( sub( buf[j], rbuf[j] ), fg_sum[j] );

    /* *L_tdist += wegt * tmp * tmp */
    //L_acc = L_mult( wegt[j], tmp );
    L_acc = (Word32)wegt[j] * (Word32)tmp;
    tmp2 = extract_h( L_shl( L_acc, 5 ) );
    //*L_tdist = L_mac( *L_tdist, tmp2, tmp );
    *L_tdist += (Word32)tmp2 * (Word32)tmp;
  }
  *L_tdist <<=1;
}


#if 0
void Lsp_last_select(
  Word32 L_tdist[],     /* (i) Q27 : distortion         */
  Word16 *mode_index    /* (o)     : the selected mode  */
)
{
    Word32 L_temp;
  *mode_index = 0;
  L_temp =L_sub(L_tdist[1] ,L_tdist[0]);
  if (  L_temp<0L){
    *mode_index = 1;

}
#endif

static void Get_wegt(
  Word16 flsp[],    /* (i) Q13 : M LSP parameters  */
  Word16 wegt[]     /* (o) Q11->norm : M weighting coefficients */
)
{
  Word16 i;
  Word16 tmp;
  Word32 L_acc;
  Word16 sft;

  tmp = flsp[1] - (PI04+8192);           /* 8192:1.0(Q13) */
  wegt[0] = 2048;                        /* 2048:1.0(Q11) */
  if (tmp < 0)
  {
    L_acc = (Word32)tmp*(Word32)tmp; /* L_acc in Q27 */
    tmp = (Word16)(L_acc >> 13);     /* tmp in Q13 */

    L_acc = (Word32)tmp * (Word32)CONST10;    /* L_acc in Q25 */
    tmp = (Word16)(L_acc >> 13);              /* tmp in Q11 */

    wegt[0] += tmp;                 /* wegt in Q11 */
  }

  for ( i = 1 ; i < M - 1 ; i++ ) {
      tmp = flsp[i+1] - flsp[i-1] - 8192;
      wegt[i] = 2048; /* 2048:1.0(Q11) */
      if (tmp < 0) {
        L_acc = (Word32)tmp*(Word32)tmp; /* L_acc in Q27 */
        tmp = (Word16)(L_acc >> 13);     /* tmp in Q13 */

        L_acc = (Word32)tmp * (Word32)CONST10;    /* L_acc in Q25 */
        tmp = (Word16)(L_acc >> 13);              /* tmp in Q11 */

        wegt[i] += tmp;                 /* wegt in Q11 */
      }
    }
  /* case M-1 */
  tmp = (PI92-8192) - flsp[M-2];
  wegt[M-1] = 2048;                        /* 2048:1.0(Q11) */
  if (tmp < 0)
  {
    L_acc = (Word32)tmp*(Word32)tmp; /* L_acc in Q27 */
    tmp = (Word16)(L_acc >> 13);     /* tmp in Q13 */

    L_acc = (Word32)tmp * (Word32)CONST10;    /* L_acc in Q25 */
    tmp = (Word16)(L_acc >> 13);              /* tmp in Q11 */

    wegt[M-1] += tmp;                 /* wegt in Q11 */
  }

  /* */
  L_acc = (Word32)wegt[4] * (Word32)CONST12;             /* L_acc in Q26 */
  wegt[4] = (Word16)(L_acc >> 14);       /* wegt in Q11 */

  L_acc = (Word32)wegt[5] * (Word32)CONST12;             /* L_acc in Q26 */
  wegt[5] = (Word16)(L_acc >> 14);       /* wegt in Q11 */


  /* wegt: Q11 -> normalized */
  tmp = 0;
  for ( i = 0; i < M; i++ ) {
    if ( wegt[i] > tmp) {
      tmp = wegt[i];
    }
  }

  sft = norm_s(tmp);
  for ( i = 0; i < M; i++ ) {
    wegt[i] = shl(wegt[i], sft);                  /* wegt in Q(11+sft) */
  }
}
/************** End of qua_lsp.c ********************************************/

/************** Begin of file taming.c **************************************/
/**************************************************************************
 * Taming functions.                                                      *
 **************************************************************************/

static void Init_exc_err(g729a_encoder_state *state)
{
  Word16 i;
  for(i=0; i<4; i++) state->L_exc_err[i] = 0x00004000L;   /* Q14 */
}

/**************************************************************************
 * routine test_err - computes the accumulated potential error in the     *
 * adaptive codebook contribution                                         *
 **************************************************************************/

static Word16 test_err(  /* (o) flag set to 1 if taming is necessary  */
 g729a_encoder_state *state,
 Word16 T0,       /* (i) integer part of pitch delay           */
 Word16 T0_frac   /* (i) fractional part of pitch delay        */
)
 {
    Word16 i, t1, zone1, zone2, flag;
    Word32 L_maxloc;

    /*if(T0_frac > 0) {
      t1 = T0 + 1;
    }
    else {
        t1 = T0;
    }*/
    t1 = T0 + (T0_frac > 0);

    i = t1 - (L_SUBFR+L_INTER10);
    if(i < 0) {
        i = 0;
    }
    zone1 = tab_zone[i];

    //i = add(t1, (L_INTER10 - 2));
    i = t1 + (L_INTER10 - 2);
    zone2 = tab_zone[i];

    L_maxloc = -1L;
    flag = 0 ;
    for(i=zone2; i>=zone1; i--) {
      if (state->L_exc_err[i] > L_maxloc) {
                L_maxloc = state->L_exc_err[i];
        }
    }

    if (L_maxloc > L_THRESH_ERR) {
        flag = 1;
    }

    return(flag);
}

/**************************************************************************
 *routine update_exc_err - maintains the memory used to compute the error *
 * function due to an adaptive codebook mismatch between encoder and      *
 * decoder                                                                *
 **************************************************************************/

static void update_exc_err(
 g729a_encoder_state *state,
 Word16 gain_pit,      /* (i) pitch gain */
 Word16 T0             /* (i) integer part of pitch delay */
)
 {
    Word16 i, zone1, zone2, n;
    Word32 L_worst, L_temp;
    Word16 hi, lo;

    L_worst = -1L;
    n = T0 - L_SUBFR;

    if(n < 0) {
        hi = (Word16)(state->L_exc_err[0] >> 16);
        lo = (Word16)((state->L_exc_err[0] >> 1) - ((Word32)(hi) << 15));
        L_temp = (hi*gain_pit);
        L_temp += ((lo*gain_pit)>>15);
        L_temp <<= 2;
        L_temp = L_add(0x00004000L, L_temp);

        if(L_temp > L_worst) {
                L_worst = L_temp;
        }

        hi = (Word16)(L_temp >> 16);
        lo = (Word16)((L_temp >> 1) - ((Word32)(hi) << 15));
        L_temp = (hi*gain_pit);
        L_temp += ((lo*gain_pit)>>15);
        L_temp <<= 2;
        L_temp = L_add(0x00004000L, L_temp);
        if (L_temp > L_worst) {
                L_worst = L_temp;
        }
    }

    else {
        zone1 = tab_zone[n];

        i = T0 - 1;
        zone2 = tab_zone[i];

        for(i = zone1; i <= zone2; i++) {
          hi = (Word16)(state->L_exc_err[i] >> 16);
          lo = (Word16)((state->L_exc_err[i] >> 1) - ((Word32)(hi) << 15));
          L_temp = (hi*gain_pit);
          L_temp += ((lo*gain_pit)>>15);
          L_temp <<= 2;
          L_temp = L_add(0x00004000L, L_temp);
          if (L_temp > L_worst)
            L_worst = L_temp;
        }
    }

    for(i=3; i>=1; i--) {
        state->L_exc_err[i] = state->L_exc_err[i-1];
    }
    //L_exc_err[3] = L_exc_err[2];
    //L_exc_err[2] = L_exc_err[1];
    //L_exc_err[1] = L_exc_err[0];
    state->L_exc_err[0] = L_worst;
}
/************** End of file taming.c ****************************************/

/************** Begin of file util.c ****************************************/
/* Random generator  */

Word16 Random()
{
  static Word16 seed = 21845;

  //seed = (Word32)seed * 31821LL + 13849LL;
  seed = extract_l(L_add(L_shr(L_mult(seed, 31821), 1), 13849L));

  return(seed);
}
/************** End of file util.c ******************************************/

/************** Begin of file g729a_decoder.c *******************************/
Word32 g729a_dec_mem_size ()
{
  return sizeof(g729a_decode_frame_state);
}

Flag   g729a_dec_init (void *decState)
{
  g729a_decode_frame_state *state;
  if (decState == NULL)
    return 0;

  state = (g729a_decode_frame_state *)decState;

  Set_zero(state->synth_buf, M);
  state->synth = state->synth_buf + M;

  Init_Decod_ld8a(&state->decoderState);
  Init_Post_Filter(&state->postFilterState);
  Init_Post_Process(&state->postProcessState);

  return 1;
}

#if defined(CONTROL_OPT) && (CONTROL_OPT == 1)
void   g729a_dec_process  (void *decState, Word16 *bitstream, Word16 *pcm,
                           Flag badFrame)
#else
void   g729a_dec_process  (void *decState, UWord8 *bitstream, Word16 *pcm,
                           Flag badFrame)
#endif
{
  g729a_decode_frame_state *state = (g729a_decode_frame_state *)decState;
  static Word16 bad_lsf = 0;          /* Initialize bad LSF indicator */
  Word16  parm[PRM_SIZE+1];             /* Synthesis parameters        */
  Word16  Az_dec[MP1*2];                /* Decoded Az for post-filter  */
  Word16  T2[2];                        /* Pitch lag for 2 subframes   */

  bits2prm_ld8k( bitstream, &parm[1]);

  parm[0] = badFrame ? 1 : 0;           /* No frame erasure */

  /* check pitch parity and put 1 in parm[4] if parity error */
  parm[4] = Check_Parity_Pitch(parm[3], parm[4]);

  Decod_ld8a(&state->decoderState, parm, state->synth, Az_dec, T2, bad_lsf);

  Post_Filter(&state->postFilterState, state->synth, Az_dec, T2);        /* Post-filter */

  Post_Process(&state->postProcessState, state->synth, pcm, L_FRAME);
}

void   g729a_dec_deinit   (void *decState)
{
  if (decState == NULL)
    return;

  memset(decState,0,sizeof(g729a_decode_frame_state));
}
/************** End of file g729a_decoder.c *********************************/

/************** Begin of file g729a_encoder.c *******************************/
Word32 g729a_enc_mem_size ()
{
  return sizeof(g729a_encode_frame_state);
}

Flag   g729a_enc_init     (void *encState)
{
  g729a_encode_frame_state *state;
  if (encState == NULL)
    return 0;

  state = (g729a_encode_frame_state *)encState;

  Init_Pre_Process(&state->preProcessState);
  Init_Coder_ld8a(&state->encoderState);

  return 1;
}

#if defined(CONTROL_OPT) && (CONTROL_OPT == 1)
void   g729a_enc_process  (void *encState, Word16 *pcm, Word16 *bitstream)
#else
void   g729a_enc_process  (void *encState, Word16 *pcm, UWord8 *bitstream)
#endif
{
  g729a_encode_frame_state *state = (g729a_encode_frame_state *)encState;
  Word16 prm[PRM_SIZE];          /* Analysis parameters.                  */

  Pre_Process(&state->preProcessState, pcm, state->encoderState.new_speech, L_FRAME);

  Coder_ld8a(&state->encoderState, prm);

  prm2bits_ld8k( prm, bitstream);
}

void   g729a_enc_deinit   (void *encState)
{
  if (encState == NULL)
    return;

  memset(encState,0,sizeof(g729a_encode_frame_state));
}
/************** End of file g729a_encoder.c *********************************/
