/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#ifndef __DTX_H__
#define __DTX_H__

/*--------------------------------------------------------------------------*
 * Prototypes for DTX/CNG                                                   *
 *--------------------------------------------------------------------------*/

/* Encoder DTX/CNG functions */
void WebRtcG729fix_Init_Cod_cng(Coder_ld8a_state *st);
void WebRtcG729fix_Cod_cng(
  Coder_ld8a_state *st,
  int16_t *exc,          /* (i/o) : excitation array                     */
  int16_t pastVad,       /* (i)   : previous VAD decision                */
  int16_t *lsp_old_q,    /* (i/o) : previous quantized lsp               */
  int16_t *Aq,           /* (o)   : set of interpolated LPC coefficients */
  int16_t *ana,          /* (o)   : coded SID parameters                 */
  int16_t freq_prev[MA_NP][M],
                         /* (i/o) : previous LPS for quantization        */
  int16_t *seed          /* (i/o) : random generator seed                */
);
void WebRtcG729fix_Update_cng(
  Coder_ld8a_state *st,
  int16_t *r_h,      /* (i) :   MSB of frame autocorrelation        */
  int16_t exp_r,     /* (i) :   scaling factor associated           */
  int16_t Vad        /* (i) :   current Vad decision                */
);

/* SID gain Quantization */
void WebRtcG729fix_Qua_Sidgain(
  int16_t *ener,     /* (i)   array of energies                   */
  int16_t *sh_ener,  /* (i)   corresponding scaling factors       */
  int16_t nb_ener,   /* (i)   number of energies or               */
  int16_t *enerq,    /* (o)   decoded energies in dB              */
  int16_t *idx       /* (o)   SID gain quantization index         */
);

/* CNG excitation generation */
void WebRtcG729fix_Calc_exc_rand(
  int32_t L_exc_err[],
  int16_t cur_gain,      /* (i)   :   target sample gain                 */
  int16_t *exc,          /* (i/o) :   excitation array                   */
  int16_t *seed,         /* (i/o) : random generator seed                */
  int flag_cod           /* (i)   :   encoder/decoder flag               */
);

/* SID LSP Quantization */
void WebRtcG729fix_Get_freq_prev(Coder_ld8a_state *st, int16_t x[MA_NP][M]);
void WebRtcG729fix_Update_freq_prev(Coder_ld8a_state *st, int16_t x[MA_NP][M]);
void WebRtcG729fix_Get_decfreq_prev(Decod_ld8a_state *st, int16_t x[MA_NP][M]);
void WebRtcG729fix_Update_decfreq_prev(Decod_ld8a_state *st, int16_t x[MA_NP][M]);

/* Decoder CNG generation */
void WebRtcG729fix_Init_Dec_cng(Decod_ld8a_state *st);
void WebRtcG729fix_Dec_cng(
  Decod_ld8a_state *st,
  int16_t past_ftyp,     /* (i)   : past frame type                      */
  int16_t sid_sav,       /* (i)   : energy to recover SID gain           */
  int16_t sh_sid_sav,    /* (i)   : corresponding scaling factor         */
  int16_t *parm,         /* (i)   : coded SID parameters                 */
  int16_t *exc,          /* (i/o) : excitation array                     */
  int16_t *lsp_old,      /* (i/o) : previous lsp                         */
  int16_t *A_t,          /* (o)   : set of interpolated LPC coefficients */
  int16_t *seed,         /* (i/o) : random generator seed                */
  int16_t freq_prev[MA_NP][M]
                         /* (i/o) : previous LPS for quantization        */
);
int16_t WebRtcG729fix_read_frame(FILE *f_serial, int16_t *parm);

#endif /* __DTX_H__ */

