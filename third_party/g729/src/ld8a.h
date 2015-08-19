/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

#ifndef __LD8A_H__
#define __LD8A_H__

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

#define PI04      ((int16_t)1029)        /* Q13  pi*0.04 */
#define PI92      ((int16_t)23677)       /* Q13  pi*0.92 */
#define CONST10   ((int16_t)10*(1<<11))  /* Q11  10.0 */
#define CONST12   ((int16_t)19661)       /* Q14  1.2 */

/*--------------------------------------------------------------------------*
 * Constants for DTX/CNG                                                    *
 *--------------------------------------------------------------------------*/

/* DTX constants */
#define FLAG_COD        (int)1
#define FLAG_DEC        (int)0
#define INIT_SEED       11111
#define FR_SID_MIN      3
#define NB_SUMACF       3
#define NB_CURACF       2
#define NB_GAIN         2
#define FRAC_THRESH1    4855
#define FRAC_THRESH2    3161
#define A_GAIN0         28672

#define SIZ_SUMACF      (NB_SUMACF * MP1)
#define SIZ_ACF         (NB_CURACF * MP1)
#define A_GAIN1         4096    /* 32768L - A_GAIN0 */

#define RATE_8000       80      /* Full rate  (8000 bit/s)       */
#define RATE_SID        15      /* SID                           */
#define RATE_0           0      /* 0 bit/s rate                  */

/* CNG excitation generation constant */
                                        /* alpha = 0.5 */
#define FRAC1           19043           /* (sqrt(40)xalpha/2 - 1) * 32768 */
#define K0              24576           /* (1 - alpha ** 2) in Q15        */
#define G_MAX           5000

/*--------------------------------------------------------------------------*
 * Constants used in VAD                                                    *
 *--------------------------------------------------------------------------*/

#define NP            12                  /* Increased LPC order */
#define NOISE         0
#define VOICE         1
#define INIT_FRAME    32
#define INIT_COUNT    20
#define ZC_START      120
#define ZC_END        200


/*-------------------------------*
 * VAD state                     *
 *-------------------------------*/

typedef struct vad_state {
  int16_t MeanLSF[M];
  int16_t Min_buffer[16];
  int16_t Prev_Min, Next_Min, Min;
  int16_t MeanE, MeanSE, MeanSLE, MeanSZC;
  int16_t prev_energy;
  int16_t count_sil, count_update, count_ext;
  int16_t flag, v_flag, less_count;
} vad_state;


/*-------------------------------*
 * Coder/Decoder states          *
 *-------------------------------*/
typedef struct Coder_ld8a_state {
  /* Speech vector */
  int16_t old_speech[L_TOTAL];
  int16_t *speech, *p_window;
  int16_t *new_speech;

  /* Weighted speech vector */
  int16_t old_wsp[L_FRAME+PIT_MAX];
  int16_t *wsp;

  /* Excitation vector */
  int16_t old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
  int16_t *exc;

  /* Lsp (Line spectral pairs) */
  int16_t lsp_old[M]/*={
        30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000}*/;
  int16_t lsp_old_q[M];

  /* Filter's memory */
  int16_t  mem_w0[M], mem_w[M], mem_zero[M];
  int16_t  sharp;

  /* Used in qua_lsp.c */
  int16_t freq_prev[MA_NP][M];    /* Q13:previous LSP vector       */

  /* Used in taming.c */
  int32_t L_exc_err[4];

  /* For G.729B */
  /* DTX variables */
  int16_t pastVad;
  int16_t ppastVad;
  int16_t seed;

  /* Used in dtx.c */
  int16_t lspSid_q[M];
  int16_t pastCoeff[MP1];
  int16_t RCoeff[MP1];
  int16_t sh_RCoeff;
  int16_t Acf[SIZ_ACF];
  int16_t sh_Acf[NB_CURACF];
  int16_t sumAcf[SIZ_SUMACF];
  int16_t sh_sumAcf[NB_SUMACF];
  int16_t ener[NB_GAIN];
  int16_t sh_ener[NB_GAIN];
  int16_t fr_cur;
  int16_t cur_gain;
  int16_t nb_ener;
  int16_t sid_gain;
  int16_t flag_chang;
  int16_t prev_energy;
  int16_t count_fr0;

  /* Quantization of LSF vector */
  int16_t noise_fg[MODE][MA_NP][M];

  /* VAD */
  vad_state vad_state;

  /* Used in Qua_gain */
  int16_t past_qua_en[4];

  /* Used in Levinson */
  int16_t old_A[M+1]/*={4096,0,0,0,0,0,0,0,0,0,0}*/;
  int16_t old_rc[2]/*={0,0}*/;

} Coder_ld8a_state;

typedef struct Decod_ld8a_state {
  /* Excitation vector */
  int16_t old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
  int16_t *exc;

  /* Lsp (Line spectral pairs) */
  int16_t lsp_old[M]/*={
       30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000}*/;

  /* Filter's memory */
  int16_t mem_syn[M];

  int16_t sharp;           /* pitch sharpening of previous frame */
  int16_t old_T0;          /* integer delay of previous frame    */
  int16_t gain_code;       /* Code gain                          */
  int16_t gain_pitch;      /* Pitch gain                         */

  /* for G.729B */
  int16_t seed_fer;
  /* CNG variables */
  int16_t past_ftyp;
  int16_t seed;
  int16_t sid_sav, sh_sid_sav;

  /* Used in lspdec.c */
  int16_t freq_prev[MA_NP][M];   /* Q13 */

  /* static memory for frame erase operation */
  int16_t prev_ma;                  /* previous MA prediction coef.*/
  int16_t prev_lsp[M];              /* previous LSP vector         */

  /* Quantization of LSF vector */
  int16_t noise_fg[MODE][MA_NP][M];

  /* Used in dec_sid.c */
  int32_t L_exc_err[4];

  int16_t cur_gain;
  int16_t lspSid[M];
  int16_t sid_gain;

  /* Used in Dec_gain */
  int16_t past_qua_en[4];

} Decod_ld8a_state;


/*-------------------------------*
 * Pre_Process state             *
 *-------------------------------*/

typedef struct Pre_Process_state {

  /* Static values to be preserved between calls */
  /* y[] values is keep in double precision      */

  int16_t y2_hi, y2_lo, y1_hi, y1_lo, x0, x1;

} Pre_Process_state;


/*-------------------------------*
 * Post_Process state            *
 *-------------------------------*/

typedef struct Post_Process_state {

  /* Static values to be preserved between calls */
  /* y[] values is keep in double precision      */

  int16_t y2_hi, y2_lo, y1_hi, y1_lo, x0, x1;
} Post_Process_state;


/*-------------------------------*
 * Post_Filter state             *
 *-------------------------------*/

typedef struct Post_Filter_state {

  /* inverse filtered synthesis (with A(z/GAMMA2_PST))   */

  int16_t res2_buf[PIT_MAX+L_SUBFR];
  int16_t *res2;
  int16_t scal_res2_buf[PIT_MAX+L_SUBFR];
  int16_t *scal_res2;

  /* memory of filter 1/A(z/GAMMA1_PST) */

  int16_t mem_syn_pst[M];

  /* memory of preemphasis */

  int16_t mem_pre;

  /* memory of agc */

  int16_t past_gain;

} Post_Filter_state;


/*-------------------------------*
 * Mathematic functions.         *
 *-------------------------------*/

int32_t WebRtcG729fix_Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  int32_t L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

void WebRtcG729fix_Log2(
  int32_t L_x,       /* (i) Q0 : input value                                 */
  int16_t *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  int16_t *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

int32_t WebRtcG729fix_Pow2(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  int16_t exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  int16_t fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);

/*-------------------------------*
 * Pre and post-process.         *
 *-------------------------------*/

void WebRtcG729fix_Init_Pre_Process(Pre_Process_state *st);
void WebRtcG729fix_Init_Post_Process(Post_Process_state *st);

void WebRtcG729fix_Pre_Process(
  Pre_Process_state *st,
  const int16_t sigin[], /* Input signal        */
  int16_t sigout[],      /* Output signal       */
  int16_t lg             /* Length of signal    */
);

void WebRtcG729fix_Post_Process(
  Post_Process_state *st,
  const int16_t sigin[], /* Input signal        */
  int16_t sigout[],      /* Output signal       */
  int16_t lg             /* Length of signal    */
);

/*----------------------------------*
 * Main coder and decoder functions *
 *----------------------------------*/

void WebRtcG729fix_Init_Coder_ld8a(Coder_ld8a_state *st);

void WebRtcG729fix_Coder_ld8a(
  Coder_ld8a_state *st,
  int16_t ana[],       /* output  : Analysis parameters */
  int16_t frame,
  int16_t vad_enable
);

void WebRtcG729fix_Init_Decod_ld8a(Decod_ld8a_state *st);

void WebRtcG729fix_Decod_ld8a(
  Decod_ld8a_state *st,
  int16_t  parm[],      /* (i)   : vector of synthesis parameters
                                   parm[0] = bad frame indicator (bfi)  */
  int16_t  synth[],     /* (o)   : synthesis speech                     */
  int16_t  A_t[],       /* (o)   : decoded LP filter in 2 subframes     */
  int16_t  *T2,         /* (o)   : decoded pitch lag in 2 subframes     */
  int16_t  *Vad,        /* (o)   : VAD                                  */
  int16_t  bad_lsf      /* (i)   : bad LSF indicator                    */
);

/*
   The bad LSF parameter above should be always set to zero unless
   transmission errors in LSP indices are detected.
   This flag is useful if the channel coding designer decides to perform
   error checking on these important parameters. If an error is detected
   on the  LSP indices, the corresponding flag is set to 1 signalling to
   the decoder to perform parameter substitution. (The flags should be set
   back to 0 for correct transmission).
*/

/*-------------------------------*
 * LPC analysis and filtering.   *
 *-------------------------------*/

void WebRtcG729fix_Autocorr(
  int16_t x[],      /* (i)    : Input signal                      */
  int16_t m,        /* (i)    : LPC order                         */
  int16_t r_h[],    /* (o)    : Autocorrelations  (msb)           */
  int16_t r_l[],    /* (o)    : Autocorrelations  (lsb)           */
  int16_t *exp_R0
);

void WebRtcG729fix_Lag_window(
  int16_t m,         /* (i)     : LPC order                        */
  int16_t r_h[],     /* (i/o)   : Autocorrelations  (msb)          */
  int16_t r_l[]      /* (i/o)   : Autocorrelations  (lsb)          */
);

void WebRtcG729fix_Levinson(
  Coder_ld8a_state *st,
  int16_t Rh[],      /* (i)     : Rh[m+1] Vector of autocorrelations (msb) */
  int16_t Rl[],      /* (i)     : Rl[m+1] Vector of autocorrelations (lsb) */
  int16_t A[],       /* (o) Q12 : A[m]    LPC coefficients  (m = 10)       */
  int16_t rc[],      /* (o) Q15 : rc[M]   Relection coefficients.          */
  int16_t *Err       /* (o)     : Residual energy                          */
);

void WebRtcG729fix_Az_lsp(
  int16_t a[],        /* (i) Q12 : predictor coefficients              */
  int16_t lsp[],      /* (o) Q15 : line spectral pairs                 */
  int16_t old_lsp[]   /* (i)     : old lsp[] (in case not found 10 roots) */
);

void WebRtcG729fix_Lsp_Az(
  int16_t lsp[],    /* (i) Q15 : line spectral frequencies            */
  int16_t a[]       /* (o) Q12 : predictor coefficients (order = 10)  */
);

void WebRtcG729fix_Lsf_lsp(
  int16_t lsf[],    /* (i) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  int16_t lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)                */
  int16_t m         /* (i)     : LPC order                                */
);

void WebRtcG729fix_Lsp_lsf(
  int16_t lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)                */
  int16_t lsf[],    /* (o) Q15 : lsf[m] normalized (range: 0.0<=val<=0.5) */
  int16_t m         /* (i)     : LPC order                                */
);

void WebRtcG729fix_Int_qlpc(
 int16_t lsp_old[], /* input : LSP vector of past frame              */
 int16_t lsp_new[], /* input : LSP vector of present frame           */
 int16_t Az[]       /* output: interpolated Az() for the 2 subframes */
);

void WebRtcG729fix_Weight_Az(
  int16_t a[],      /* (i) Q12 : a[m+1]  LPC coefficients             */
  int16_t gamma,    /* (i) Q15 : Spectral expansion factor.           */
  int16_t m,        /* (i)     : LPC order.                           */
  int16_t ap[]      /* (o) Q12 : Spectral expanded LPC coefficients   */
);

void WebRtcG729fix_Residu(
  int16_t a[],    /* (i) Q12 : prediction coefficients                     */
  int16_t x[],    /* (i)     : speech (values x[-m..-1] are needed (m=10)  */
  int16_t y[],    /* (o)     : residual signal                             */
  int16_t lg      /* (i)     : size of filtering                           */
);

int WebRtcG729fix_Syn_filt(
  int16_t a[],     /* (i) Q12 : a[m+1] prediction coefficients   (m=10)  */
  int16_t x[],     /* (i)     : input signal                             */
  int16_t y[],     /* (o)     : output signal                            */
  int16_t lg,      /* (i)     : size of filtering                        */
  int16_t mem[],   /* (i/o)   : memory associated with this filtering.   */
  int16_t update   /* (i)     : 0=no update, 1=update of memory.         */
);

void WebRtcG729fix_Convolve(
  int16_t x[],      /* (i)     : input vector                           */
  int16_t h[],      /* (i) Q12 : impulse response                       */
  int16_t y[],      /* (o)     : output vector                          */
  int16_t L         /* (i)     : vector size                            */
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

int16_t WebRtcG729fix_Pitch_ol_fast(  /* output: open loop pitch lag                        */
   int16_t signal[],    /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
   int16_t   pit_max,   /* input : maximum pitch lag                          */
   int16_t   L_frame    /* input : length of frame to compute pitch           */
);

int16_t WebRtcG729fix_Pitch_fr3_fast(/* (o)     : pitch period.                          */
  int16_t exc[],       /* (i)     : excitation buffer                      */
  int16_t xn[],        /* (i)     : target vector                          */
  int16_t h[],         /* (i) Q12 : impulse response of filters.           */
  int16_t L_subfr,     /* (i)     : Length of subframe                     */
  int16_t t0_min,      /* (i)     : minimum value in the searched range.   */
  int16_t t0_max,      /* (i)     : maximum value in the searched range.   */
  int16_t i_subfr,     /* (i)     : indicator for first subframe.          */
  int16_t *pit_frac    /* (o)     : chosen fraction.                       */
);

int16_t WebRtcG729fix_G_pitch(      /* (o) Q14 : Gain of pitch lag saturated to 1.2       */
  int16_t xn[],       /* (i)     : Pitch target.                            */
  int16_t y1[],       /* (i)     : Filtered adaptive codebook.              */
  int16_t g_coeff[],  /* (i)     : Correlations need for gain quantization. */
  int16_t L_subfr     /* (i)     : Length of subframe.                      */
);

int16_t WebRtcG729fix_Enc_lag3(     /* output: Return index of encoding */
  int16_t T0,         /* input : Pitch delay              */
  int16_t T0_frac,    /* input : Fractional pitch delay   */
  int16_t *T0_min,    /* in/out: Minimum search delay     */
  int16_t *T0_max,    /* in/out: Maximum search delay     */
  int16_t pit_min,    /* input : Minimum pitch delay      */
  int16_t pit_max,    /* input : Maximum pitch delay      */
  int16_t pit_flag    /* input : int for 1st subframe    */
);

void WebRtcG729fix_Dec_lag3(        /* output: return integer pitch lag       */
  int16_t index,       /* input : received pitch index           */
  int16_t pit_min,     /* input : minimum pitch lag              */
  int16_t pit_max,     /* input : maximum pitch lag              */
  int16_t i_subfr,     /* input : subframe flag                  */
  int16_t *T0,         /* output: integer part of pitch lag      */
  int16_t *T0_frac     /* output: fractional part of pitch lag   */
);

int16_t WebRtcG729fix_Interpol_3(      /* (o)  : interpolated value  */
  int16_t *x,            /* (i)  : input vector        */
  int16_t frac           /* (i)  : fraction            */
);

void WebRtcG729fix_Pred_lt_3(
  int16_t   exc[],       /* in/out: excitation buffer */
  int16_t   T0,          /* input : integer pitch lag */
  int16_t   frac,        /* input : fraction of lag   */
  int16_t   L_subfr      /* input : subframe size     */
);

int16_t WebRtcG729fix_Parity_Pitch(    /* output: parity bit (XOR of 6 MSB bits)    */
   int16_t pitch_index   /* input : index for which parity to compute */
);

int16_t  WebRtcG729fix_Check_Parity_Pitch( /* output: 0 = no error, 1= error */
  int16_t pitch_index,       /* input : index of parameter     */
  int16_t parity             /* input : parity bit             */
);

void WebRtcG729fix_Cor_h_X(
     int16_t h[],        /* (i) Q12 :Impulse response of filters      */
     int16_t X[],        /* (i)     :Target vector                    */
     int16_t D[]         /* (o)     :Correlations between h[] and D[] */
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

#define _1_2    (int16_t)(16384)
#define _1_4    (int16_t)( 8192)
#define _1_8    (int16_t)( 4096)
#define _1_16   (int16_t)( 2048)

int16_t  WebRtcG729fix_ACELP_Code_A(    /* (o)     :index of pulses positions    */
  int16_t x[],            /* (i)     :Target vector                */
  int16_t h[],            /* (i) Q12 :Inpulse response of filters  */
  int16_t T0,             /* (i)     :Pitch lag                    */
  int16_t pitch_sharp,    /* (i) Q14 :Last quantized pitch gain    */
  int16_t code[],         /* (o) Q13 :Innovative codebook          */
  int16_t y[],            /* (o) Q12 :Filtered innovative codebook */
  int16_t *sign           /* (o)     :Signs of 4 pulses            */
);

void WebRtcG729fix_Decod_ACELP(
  int16_t sign,      /* (i)     : signs of 4 pulses.                       */
  int16_t index,     /* (i)     : Positions of the 4 pulses.               */
  int16_t cod[]      /* (o) Q13 : algebraic (fixed) codebook excitation    */
);

/*-------------------------------*
 * LSP VQ functions.             *
 *-------------------------------*/

void WebRtcG729fix_Lsf_lsp2(
  int16_t lsf[],    /* (i) Q13 : lsf[m] (range: 0.0<=val<PI) */
  int16_t lsp[],    /* (o) Q15 : lsp[m] (range: -1<=val<1)   */
  int16_t m         /* (i)     : LPC order                   */
);

void WebRtcG729fix_Lsp_lsf2(
  int16_t lsp[],    /* (i) Q15 : lsp[m] (range: -1<=val<1)   */
  int16_t lsf[],    /* (o) Q13 : lsf[m] (range: 0.0<=val<PI) */
  int16_t m         /* (i)     : LPC order                   */
);

void WebRtcG729fix_Qua_lsp(
  Coder_ld8a_state *st,
  int16_t lsp[],       /* (i) Q15 : Unquantized LSP            */
  int16_t lsp_q[],     /* (o) Q15 : Quantized LSP              */
  int16_t ana[]        /* (o)     : indexes                    */
);

void WebRtcG729fix_Get_wegt(
  int16_t flsp[],    /* Q13 */
  int16_t wegt[]     /* Q11 -> normalized */
);

void WebRtcG729fix_Lsp_encw_reset(
  Coder_ld8a_state *st
);

void WebRtcG729fix_Lsp_qua_cs(
  Coder_ld8a_state *st,
  int16_t flsp_in[M],       /* Q13 */
  int16_t lspq_out[M],      /* Q13 */
  int16_t *code
);

void WebRtcG729fix_Lsp_expand_1(
  int16_t buf[],            /* Q13 */
  int16_t gap               /* Q13 */
);

void WebRtcG729fix_Lsp_expand_2(
  int16_t buf[],            /* Q13 */
  int16_t gap               /* Q13 */
);

void WebRtcG729fix_Lsp_expand_1_2(
  int16_t buf[],            /* Q13 */
  int16_t gap               /* Q13 */
);

void WebRtcG729fix_Lsp_get_quant(
  int16_t lspcb1[][M],      /* Q13 */
  int16_t lspcb2[][M],      /* Q13 */
  int16_t code0,
  int16_t code1,
  int16_t code2,
  int16_t fg[][M],          /* Q15 */
  int16_t freq_prev[][M],   /* Q13 */
  int16_t lspq[],           /* Q13 */
  int16_t fg_sum[]          /* Q15 */
);

void WebRtcG729fix_Lsp_get_tdist(
  int16_t wegt[],        /* normalized */
  int16_t buf[],         /* Q13 */
  int32_t *L_tdist,      /* Q27 */
  int16_t rbuf[],        /* Q13 */
  int16_t fg_sum[]       /* Q15 */
);

void WebRtcG729fix_Lsp_last_select(
  int32_t L_tdist[],     /* Q27 */
  int16_t *mode_index
);

void WebRtcG729fix_Lsp_pre_select(
  int16_t rbuf[],              /* Q13 */
  int16_t lspcb1[][M],         /* Q13 */
  int16_t *cand
);

void WebRtcG729fix_Lsp_select_1(
  int16_t rbuf[],              /* Q13 */
  int16_t lspcb1[],            /* Q13 */
  int16_t wegt[],              /* normalized */
  int16_t lspcb2[][M],         /* Q13 */
  int16_t *index
);

void WebRtcG729fix_Lsp_select_2(
  int16_t rbuf[],              /* Q13 */
  int16_t lspcb1[],            /* Q13 */
  int16_t wegt[],              /* normalized */
  int16_t lspcb2[][M],         /* Q13 */
  int16_t *index
);

void WebRtcG729fix_Lsp_stability(
  int16_t buf[]     /* Q13 */
);

void WebRtcG729fix_Relspwed(
  int16_t lsp[],                       /* Q13 */
  int16_t wegt[],                      /* normalized */
  int16_t lspq[],                      /* Q13 */
  int16_t lspcb1[][M],                 /* Q13 */
  int16_t lspcb2[][M],                 /* Q13 */
  int16_t fg[MODE][MA_NP][M],          /* Q15 */
  int16_t freq_prev[MA_NP][M],         /* Q13 */
  int16_t fg_sum[MODE][M],             /* Q15 */
  int16_t fg_sum_inv[MODE][M],         /* Q12 */
  int16_t code_ana[]
);

void WebRtcG729fix_D_lsp(
  Decod_ld8a_state *st,
  int16_t prm[],          /* (i)     : indexes of the selected LSP */
  int16_t lsp_q[],        /* (o) Q15 : Quantized LSP parameters    */
  int16_t erase           /* (i)     : frame erase information     */
);

void WebRtcG729fix_Lsp_decw_reset(
  Decod_ld8a_state *st
);

void WebRtcG729fix_Lsp_iqua_cs(
  Decod_ld8a_state *st,
  int16_t prm[],          /* input : codes of the selected LSP*/
  int16_t lsp_q[],        /* output: Quantized LSP parameters*/
  int16_t erase           /* input : frame erase information */
);

void WebRtcG729fix_Lsp_prev_compose(
  int16_t lsp_ele[],             /* Q13 */
  int16_t lsp[],                 /* Q13 */
  int16_t fg[][M],            /* Q15 */
  int16_t freq_prev[][M],     /* Q13 */
  int16_t fg_sum[]               /* Q15 */
);

void WebRtcG729fix_Lsp_prev_extract(
  int16_t lsp[M],                 /* Q13 */
  int16_t lsp_ele[M],             /* Q13 */
  int16_t fg[MA_NP][M],           /* Q15 */
  int16_t freq_prev[MA_NP][M],    /* Q13 */
  int16_t fg_sum_inv[M]           /* Q12 */
);

void WebRtcG729fix_Lsp_prev_update(
  int16_t lsp_ele[M],             /* Q13 */
  int16_t freq_prev[MA_NP][M]     /* Q13 */
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

int16_t WebRtcG729fix_Qua_gain(
  Coder_ld8a_state *st,
  int16_t code[],    /* (i) Q13 : Innovative vector.                         */
  int16_t g_coeff[], /* (i)     : Correlations <xn y1> -2<y1 y1>             */
                    /*            <y2,y2>, -2<xn,y2>, 2<y1,y2>              */
  int16_t exp_coeff[],/* (i)    : Q-Format g_coeff[]                         */
  int16_t L_subfr,   /* (i)     : Subframe length.                           */
  int16_t *gain_pit, /* (o) Q14 : Pitch gain.                                */
  int16_t *gain_cod, /* (o) Q1  : Code gain.                                 */
  int16_t tameflag   /* (i)     : flag set to 1 if taming is needed          */
);

void WebRtcG729fix_Dec_gain(
  Decod_ld8a_state *st,
  int16_t index,     /* (i)     : Index of quantization.                     */
  int16_t code[],    /* (i) Q13 : Innovative vector.                         */
  int16_t L_subfr,   /* (i)     : Subframe length.                           */
  int16_t bfi,       /* (i)     : Bad frame indicator                        */
  int16_t *gain_pit, /* (o) Q14 : Pitch gain.                                */
  int16_t *gain_cod  /* (o) Q1  : Code gain.                                 */
);

void WebRtcG729fix_Gain_predict(
  int16_t past_qua_en[], /* (i) Q10 : Past quantized energies                */
  int16_t code[],        /* (i) Q13 : Innovative vector.                     */
  int16_t L_subfr,       /* (i)     : Subframe length.                       */
  int16_t *gcode0,       /* (o) Qxx : Predicted codebook gain                */
  int16_t *exp_gcode0    /* (o)     : Q-Format(gcode0)                       */
);

void WebRtcG729fix_Gain_update(
  int16_t past_qua_en[],  /* (i) Q10 :Past quantized energies                */
  int32_t L_gbk12         /* (i) Q13 : gbk1[indice1][1]+gbk2[indice2][1]     */
);

void WebRtcG729fix_Gain_update_erasure(
  int16_t past_qua_en[]   /* (i) Q10 :Past quantized energies                */
);

void WebRtcG729fix_Corr_xy2(
  int16_t xn[],           /* (i) Q0  :Target vector.                         */
  int16_t y1[],           /* (i) Q0  :Adaptive codebook.                     */
  int16_t y2[],           /* (i) Q12 :Filtered innovative vector.            */
  int16_t g_coeff[],      /* (o) Q[exp]:Correlations between xn,y1,y2        */
  int16_t exp_g_coeff[]   /* (o)       :Q-format of g_coeff[]                */
);

/*-----------------------*
 * Bitstream function    *
 *-----------------------*/

void  WebRtcG729fix_prm2bits_ld8k(int16_t prm[], int16_t bits[]);
void  WebRtcG729fix_bits2prm_ld8k(int16_t bits[], int16_t prm[]);
#define BIT_0     (short)0x007f /* definition of zero-bit in bit-stream      */
#define BIT_1     (short)0x0081 /* definition of one-bit in bit-stream       */
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
#define  AGC_FAC1 (int16_t)(32767 - AGC_FAC)   /* 1-AGC_FAC in Q15          */


void WebRtcG729fix_Init_Post_Filter(Post_Filter_state *st);

void WebRtcG729fix_Post_Filter(
  Post_Filter_state *st,
  int16_t *syn,     /* in/out: synthesis speech (postfiltered is output)    */
  int16_t *Az_4,    /* input : interpolated LPC parameters in all subframes */
  int16_t *T,       /* input : decoded pitch lags in all subframes          */
  int16_t Vad
);

void WebRtcG729fix_pit_pst_filt(
  int16_t *signal,      /* (i)     : input signal                           */
  int16_t *scal_sig,    /* (i)     : input signal (scaled, divided by 4)    */
  int16_t t0_min,       /* (i)     : minimum value in the searched range    */
  int16_t t0_max,       /* (i)     : maximum value in the searched range    */
  int16_t L_subfr,      /* (i)     : size of filtering                      */
  int16_t *signal_pst   /* (o)     : harmonically postfiltered signal       */
);

void WebRtcG729fix_preemphasis(
  int16_t *mem_pre,
  int16_t *signal,      /* (i/o)   : input signal overwritten by the output */
  int16_t g,            /* (i) Q15 : preemphasis coefficient                */
  int16_t L             /* (i)     : size of filtering                      */
);

void WebRtcG729fix_agc(
  int16_t *past_gain,   /* (i/o)   : past gain in Q12                       */
  int16_t *sig_in,      /* (i)     : postfilter input signal                */
  int16_t *sig_out,     /* (i/o)   : postfilter output signal               */
  int16_t l_trm         /* (i)     : subframe size                          */
);

/*--------------------------------------------------------------------------*
 * Constants and prototypes for taming procedure.                           *
 *--------------------------------------------------------------------------*/

#define GPCLIP      15564      /* Maximum pitch gain if taming is needed Q14*/
#define GPCLIP2     481        /* Maximum pitch gain if taming is needed Q9 */
#define GP0999      16383      /* Maximum pitch gain if taming is needed    */
#define L_THRESH_ERR 983040000L /* Error threshold taming 16384. * 60000.   */

void   WebRtcG729fix_Init_exc_err(int32_t L_exc_err[]);
void   WebRtcG729fix_update_exc_err(int32_t L_exc_err[], int16_t gain_pit, int16_t t0);
int16_t WebRtcG729fix_test_err(int32_t L_exc_err[], int16_t t0, int16_t t0_frac);

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions.                                      *
 *--------------------------------------------------------------------------*/

#define Copy(x,y,L)    memmove((y), (x), (L)*sizeof(int16_t))

int16_t WebRtcG729fix_Random(int16_t *seed);

#endif /* __LD8A_H__ */

