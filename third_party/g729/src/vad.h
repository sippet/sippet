/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#ifndef __VAD_H__
#define __VAD_H__

void WebRtcG729fix_vad_init(vad_state *st);

void WebRtcG729fix_vad(vad_state *st,
         int16_t rc,
         int16_t *lsf, 
         int16_t *r_h,
         int16_t *r_l,
         int16_t exp_R0,
         int16_t *sigpp,
         int16_t frm_count,
         int16_t prev_marker,
         int16_t pprev_marker,
         int16_t *marker);

#endif /* __VAD_H__ */

