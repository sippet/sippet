/**
 *  g729a codec for iPhone and iPod Touch
 *  Copyright (C) 2009 Samuel <samuelv0304@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/****************************************************************************************
Portions of this file are derived from the following ITU standard:
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
****************************************************************************************/

/*___________________________________________________________________________
 |                                                                           |
 | Basics operators.                                                         |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Include-Files                                                           |
 |___________________________________________________________________________|
*/

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"




/*___________________________________________________________________________
 |                                                                           |
 |   Functions                                                               |
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

Word16 div_s(Word16 var1, Word16 var2)
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
