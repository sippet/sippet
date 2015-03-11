/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#ifndef __SID_H__
#define __SID_H__

#define         TRUE 1
#define         FALSE 0
#define         sqr(a)  ((a)*(a))
#define         R_LSFQ 10

void WebRtcG729fix_Init_lsfq_noise(int16_t noise_fg[MODE][MA_NP][M]);
void WebRtcG729fix_lsfq_noise(int16_t noise_fg[MODE][MA_NP][M],
                int16_t *lsp_new, int16_t *lspq,
                int16_t freq_prev[MA_NP][M], int16_t *idx);
void WebRtcG729fix_sid_lsfq_decode(int16_t noise_fg[MODE][MA_NP][M],
                     int16_t *index, int16_t *lspq, 
                     int16_t freq_prev[MA_NP][M]); 

#endif /* __SID_H__ */

