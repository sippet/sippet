/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#ifndef __OCTET_H__
#define __OCTET_H__

/* Definition for Octet Transmission mode */
/* When Annex B is used for transmission systems that operate on octet boundary, 
   an extra bit (with value zero) will be packed at the end of a SID bit stream. 
   This will change the number of bits in a SID bit stream from 15 bits to
   16 bits (i.e., 2 bytes).
*/

#define OCTET_TX_MODE
#define RATE_SID_OCTET    16     /* number of bits in Octet Transmission mode */


#endif /* __OCTET_H__ */

