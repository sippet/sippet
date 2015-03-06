/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#include <stdint.h>
#include "ld8a.h"
#include "basic_op.h"
#include "tab_ld8a.h"


/*----------------------------------------------------------------------------
 * Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void Lsp_decw_reset(
  Decod_ld8a_state *st
)
{
  int16_t i;

  for(i=0; i<MA_NP; i++)
    Copy( &freq_prev_reset[0], &st->freq_prev[i][0], M );

  st->prev_ma = 0;

  Copy( freq_prev_reset, st->prev_lsp, M);
}



/*----------------------------------------------------------------------------
 * Lsp_iqua_cs -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
void Lsp_iqua_cs(
  Decod_ld8a_state *st,
  int16_t prm[],          /* (i)     : indexes of the selected LSP */
  int16_t lsp_q[],        /* (o) Q13 : Quantized LSP parameters    */
  int16_t erase           /* (i)     : frame erase information     */
)
{
  int16_t mode_index;
  int16_t code0;
  int16_t code1;
  int16_t code2;
  int16_t buf[M];     /* Q13 */

  if( erase==0 ) {  /* Not frame erasure */
    mode_index = shr(prm[0] ,NC0_B) & (int16_t)1;
    code0 = prm[0] & (int16_t)(NC0 - 1);
    code1 = shr(prm[1] ,NC1_B) & (int16_t)(NC1 - 1);
    code2 = prm[1] & (int16_t)(NC1 - 1);

    /* compose quantized LSP (lsp_q) from indexes */

    Lsp_get_quant(lspcb1, lspcb2, code0, code1, code2,
      fg[mode_index], st->freq_prev, lsp_q, fg_sum[mode_index]);

    /* save parameters to use in case of the frame erased situation */

    Copy(lsp_q, st->prev_lsp, M);
    st->prev_ma = mode_index;
  }
  else {           /* Frame erased */
    /* use revious LSP */

    Copy(st->prev_lsp, lsp_q, M);

    /* update freq_prev */

    Lsp_prev_extract(st->prev_lsp, buf,
      fg[st->prev_ma], st->freq_prev, fg_sum_inv[st->prev_ma]);
    Lsp_prev_update(buf, st->freq_prev);
  }

  return;
}



/*-------------------------------------------------------------------*
 * Function  D_lsp:                                                  *
 *           ~~~~~~                                                  *
 *-------------------------------------------------------------------*/

void D_lsp(
  Decod_ld8a_state *st,
  int16_t prm[],          /* (i)     : indexes of the selected LSP */
  int16_t lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  int16_t erase           /* (i)     : frame erase information     */
)
{
  int16_t lsf_q[M];       /* domain 0.0<= lsf_q <PI in Q13 */


  Lsp_iqua_cs(st, prm, lsf_q, erase);

  /* Convert LSFs to LSPs */

  Lsf_lsp2(lsf_q, lsp_q, M);

  return;
}

void Get_decfreq_prev(Decod_ld8a_state *st, int16_t x[MA_NP][M])
{
  int16_t i;

  for (i=0; i<MA_NP; i++)
    Copy(&st->freq_prev[i][0], &x[i][0], M);
}
  
void Update_decfreq_prev(Decod_ld8a_state *st, int16_t x[MA_NP][M])
{
  int16_t i;

  for (i=0; i<MA_NP; i++)
    Copy(&x[i][0], &st->freq_prev[i][0], M);
}



