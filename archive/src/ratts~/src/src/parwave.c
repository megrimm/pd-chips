/*
file: PARWAVE.C
date: 20/4/94
version: 3.03

An implementation of a Klatt cascade-parallel formant synthesizer.
A re-implementation in C of Dennis Klatt's Fortran code, by: 

Jon Iles (j.p.iles@cs.bham.ac.uk)
Nick Ing-Simmons (nicki@lobby.ti.com)

See the README file for further details.

+ 2002/11/01 Bryan Jurish <moocow@ling.uni-potsdam.de>
  - adapted to PD


(c) 1993,94 Jon Iles and Nick Ing-Simmons

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "proto.h"
#include "parwave.h"

#ifdef _MSC_VER
#define getrandom(min,max) ((rand()%(int)(((max)+1)-(min)))+(min))
#else
#define getrandom(min,max) ((rand()%(long)(((max)+1)-(min)))+(min))
#endif



/* from Nick's nsynth.c:
 * Constant natglot[] controls shape of glottal pulse as a function
 * of desired duration of open phase N0
 * (Note that N0 is specified in terms of 40,000 samples/sec of speech)
 *
 *    Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3
 *
 *    If the radiation characterivative, a temporal derivative
 *      is folded in, and we go from continuous time to discrete
 *      integers n:  dV/dt = vwave[n]
 *                         = sum over i=1,2,...,n of { a - (i * b) }
 *                         = a n  -  b/2 n**2
 *
 *      where the  constants a and b control the detailed shape
 *      and amplitude of the voicing waveform over the open
 *      potion of the voicing cycle "nopen".
 *
 *    Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3
 *
 *    Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
 *      meaning as nopen gets bigger, V has bigger peak proportional to n
 *
 *    Thus, to generate the table below for 40 <= nopen <= 263:
 *
 *      natglot[nopen - 40] = 1920000 / (nopen * nopen)
 *
 * moocow: in Jon's 'parwave.c', this data was a static local
 * variable 'B0' in 'pitch_synch_par_reset()'.
 */
const short natglot[224] =
{
 1200, 1142, 1088, 1038, 991, 948, 907, 869, 833, 799,
 768, 738, 710, 683, 658, 634, 612, 590, 570, 551,
 533, 515, 499, 483, 468, 454, 440, 427, 415, 403,
 391, 380, 370, 360, 350, 341, 332, 323, 315, 307,
 300, 292, 285, 278, 272, 265, 259, 253, 247, 242,
 237, 231, 226, 221, 217, 212, 208, 204, 199, 195,
 192, 188, 184, 180, 177, 174, 170, 167, 164, 161,
 158, 155, 153, 150, 147, 145, 142, 140, 137, 135,
 133, 131, 128, 126, 124, 122, 120, 119, 117, 115,
 113, 111, 110, 108, 106, 105, 103, 102, 100, 99,
 97, 96, 95, 93, 92, 91, 90, 88, 87, 86,
 85, 84, 83, 82, 80, 79, 78, 77, 76, 75,
 75, 74, 73, 72, 71, 70, 69, 68, 68, 67,
 66, 65, 64, 64, 63, 62, 61, 61, 60, 59,
 59, 58, 57, 57, 56, 56, 55, 55, 54, 54,
 53, 53, 52, 52, 51, 51, 50, 50, 49, 49,
 48, 48, 47, 47, 46, 46, 45, 45, 44, 44,
 43, 43, 42, 42, 41, 41, 41, 41, 40, 40,
 39, 39, 38, 38, 38, 38, 37, 37, 36, 36,
 36, 36, 35, 35, 35, 35, 34, 34, 33, 33,
 33, 33, 32, 32, 32, 32, 31, 31, 31, 31,
 30, 30, 30, 30, 29, 29, 29, 29, 28, 28,
 28, 28, 27, 27
};



/* from nsynth.c:
 * Convertion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                 ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                 ...
 *                                  0 dB -->     0
 *
 * The just noticeable difference for a change in intensity of a vowel
 *   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 *   steps.
 *
 * moo: in Jon's parwave.c, this was a static local in 'DBtoLIN()'
 */
static const float amptable[88] =
{
 0.0, 0.0, 0.0, 0.0, 0.0,
 0.0, 0.0, 0.0, 0.0, 0.0,
 0.0, 0.0, 0.0, 6.0, 7.0,
 8.0, 9.0, 10.0, 11.0, 13.0,
 14.0, 16.0, 18.0, 20.0, 22.0,
 25.0, 28.0, 32.0, 35.0, 40.0,
 45.0, 51.0, 57.0, 64.0, 71.0,
 80.0, 90.0, 101.0, 114.0, 128.0,
 142.0, 159.0, 179.0, 202.0, 227.0,
 256.0, 284.0, 318.0, 359.0, 405.0,
 455.0, 512.0, 568.0, 638.0, 719.0,
 811.0, 911.0, 1024.0, 1137.0, 1276.0,
 1438.0, 1622.0, 1823.0, 2048.0, 2273.0,
 2552.0, 2875.0, 3244.0, 3645.0, 4096.0,
 4547.0, 5104.0, 5751.0, 6488.0, 7291.0,
 8192.0, 9093.0, 10207.0, 11502.0, 12976.0,
 14582.0, 16384.0, 18350.0, 20644.0, 23429.0,
 26214.0, 29491.0, 32767.0
};


/* function prototypes for functions private to this file */

static void flutter PROTO((klatt_global_ptr,klatt_frame_ptr));
static float resonator PROTO((resonator_ptr, float));
static float antiresonator PROTO((resonator_ptr, float));

static float impulsive_source PROTO((klatt_global_ptr));
static float natural_source PROTO((klatt_global_ptr));
static float sampled_source PROTO((klatt_global_ptr));  // parwave only

static void setabc PROTO((long,long,resonator_ptr,klatt_global_ptr));
static void setabcg PROTO((long,long,resonator_ptr,float,klatt_global_ptr)); // nsynth.c
static void setzeroabc PROTO((long,long,resonator_ptr,klatt_global_ptr));

static float DBtoLIN PROTO((long));
//static float dBconvert PROTO((long int arg)); // nsynth.c
//static void overload_warning PROTO((klatt_global_ptr globals, long int arg)); // nsynth.c
//static short clip PROTO((klatt_global_ptr globals, Float input)); // nsynth.c

static void pitch_synch_par_reset PROTO((klatt_global_ptr,klatt_frame_ptr)); 
//static void pitch_synch_par_reset PROTO((klatt_global_ptr globals, klatt_frame_ptr frame, long ns)); // nsynth.c

static float gen_noise PROTO((float,klatt_global_ptr));
static void frame_init PROTO((klatt_global_ptr,klatt_frame_ptr)); 
//void show_parms PROTO((klatt_global_ptr globals, int *pars)); // nsynth.c



/*----------------------------------------------------------------------
 *   function FLUTTER
 *
 * This function adds F0 flutter, as specified in:
 *
 * "Analysis, synthesis and perception of voice quality variations among
 * female and male talkers" D.H. Klatt and L.C. Klatt JASA 87(2) February 1990.
 * Flutter is added by applying a quasi-random element constructed from three
 * slowly varying sine waves.
 */
static void flutter(klatt_global_ptr globals, klatt_frame_ptr frame)
{
  //--- parwave
  double delta_f0;
  double fla,flb,flc,fld,fle;
  fla = (double) globals->f0_flutter / 50;
  flb = (double) globals->original_f0 / 100;
  flc = sin(2*PI*12.7*globals->time_count);
  fld = sin(2*PI*7.1*globals->time_count);
  fle = sin(2*PI*4.7*globals->time_count);
  delta_f0 =  fla * flb * (flc + fld + fle) * 10;
  frame->F0hz10 = frame->F0hz10 + (long) delta_f0;
  globals->time_count++;
  /* --- nsynth
     long original_f0 = frame->F0hz10 / 10;
     double fla = (double) globals->f0_flutter / 50;
     double flb = (double) original_f0 / 100;
     double flc = sin(2 * PI * 12.7 * globals->time_count);
     double fld = sin(2 * PI * 7.1 * globals->time_count);
     double fle = sin(2 * PI * 4.7 * globals->time_count);
     double delta_f0 = fla * flb * (flc + fld + fle) * 10;
     globals->F0hz10 += (long) delta_f0;
     // moo: time_count NOT incremented here
     */
}



/*----------------------------------------------------------------------
 * function IMPULSIVE_SOURCE
 * 
 * Generate a low pass filtered train of impulses as an approximation of 
 * a natural excitation waveform. Low-pass filter the differentiated impulse 
 * with a critically-damped second-order filter, time constant proportional 
 * to Kopen.
 */
static float impulsive_source(klatt_global_ptr globals) 
{
  float doublet[] = {0.0, 13000000.0, -13000000.0};
  //float vwave;

  if (globals->nper < 3)  {
    globals->vwave = doublet[globals->nper];
  }
  else  {
    globals->vwave = 0.0;
  }
  // Low-pass filter the differenciated impulse with a critically-damped
  // second-order filter, time constant proportional to Kopen
  return resonator(&(globals->rgl),globals->vwave);
}



/*----------------------------------------------------------------------
 * function NATURAL_SOURCE
 * 
 * Vwave is the differentiated glottal flow waveform, there is a weak
 * spectral zero around 800 Hz, magic constants a,b reset pitch synchronously.
 */
static float natural_source(klatt_global_ptr globals) 
{
  float lgtemp;
  //static float vwave; // moo: using globals->vwave

  if (globals->nper < globals->nopen)  {
    // Glottis is open
    globals->pulse_shape_a -= globals->pulse_shape_b;
    globals->vwave += globals->pulse_shape_a;
    lgtemp = globals->vwave * 0.028;
    return lgtemp;
  }
  else  {
    globals->vwave = 0.0;
    return(0.0);
  }
}


/*----------------------------------------------------------------------
 * function SAMPLED_SOURCE
 * 
 * Allows the use of a glottal excitation waveform sampled from a real
 * voice.
 */
static float sampled_source(klatt_global_ptr globals)
{
  int itemp;
  float ftemp;
  float result;
  float diff_value;
  int current_value;
  int next_value;
  float temp_diff;

  if(globals->T0!=0)
  {
    ftemp = (float) globals->nper;
    ftemp = ftemp / globals->T0;
    ftemp = ftemp * globals->num_samples;
    itemp = (int) ftemp;

    temp_diff = ftemp - (float) itemp;
  
    current_value = globals->natural_samples[itemp];
    next_value = globals->natural_samples[itemp+1];

    diff_value = (float) next_value - (float) current_value;
    diff_value = diff_value * temp_diff;

    result = globals->natural_samples[itemp] + diff_value;
    result = result * globals->sample_factor;
  }
  else
  {
    result = 0;
  }
  return(result);
}



/*----------------------------------------------------------------------
 * function SETABC
 * 
 * Convert formant freqencies and bandwidth into resonator difference 
 * equation constants.
 *
 * params:
 *  f  : Frequency of resonator in Hz
 *  bw : Bandwidth of resonator in Hz
 *  rp : resonator
*/
static void setabc(long int f, long int bw, resonator_ptr rp, klatt_global_ptr globals)
{
 double arg = globals->minus_pi_t * bw;
 float r = exp(arg);              // Let r  =  exp(-pi bw t)
 rp->c = -(r * r);                // Let c  =  -r**2
 arg = globals->two_pi_t * f;
 rp->b = r * cos(arg) * 2.0;      // Let b = r * 2*cos(2 pi f t)
 rp->a = 1.0 - rp->b - rp->c;     // Let a = 1.0 - b - c
}


/*----------------------------------------------------------------------
 * function SETABCG
 * 
 * from nsynth.c: Convienience function for setting parallel resonators with gain
 *
 * params:
 *   f   : Frequency of resonator in Hz
 *   bw  : Bandwidth of resonator in Hz
 *   rp  : resonator
 *  gain : linear gain
*/
static void setabcg(long int f, long int bw, resonator_ptr rp, float gain, klatt_global_ptr globals)
{
 setabc(f, bw, rp, globals);
 rp->a *= gain;
}


/*----------------------------------------------------------------------
 * function SETZEROABC
 * 
 * Convert formant freqencies and bandwidth into anti-resonator difference 
 * equation constants.
 *
 * params:
 *   f   : Frequency of resonator in Hz
 *   bw  : Bandwidth of resonator in Hz
 *   rp  : resonator
*/
static void setzeroabc(long int f, long int bw, resonator_ptr rp, klatt_global_ptr globals)
{
  // --- parwave.c version:
  float r;
  double arg;
     
  f = -f;
     
  if(f>=0) f = -1;

  // First compute ordinary resonator coefficients
  // Let r  =  exp(-pi bw t)
  arg = globals->minus_pi_t * bw;
  r = exp(arg);
     
  // Let c  =  -r**2
  rp->c = -(r * r);
     
  // Let b = r * 2*cos(2 pi f t)
  arg = globals->two_pi_t * f;
  rp->b = r * cos(arg) * 2.;
     
  // Let a = 1.0 - b - c
  rp->a = 1.0 - rp->b - rp->c;
     
  // Now convert to antiresonator coefficients (a'=1/a, b'=b/a, c'=c/a)
  rp->a = 1.0 / rp->a;
  rp->c *= -rp->a;
  rp->b *= -rp->a;
  /* --- nsynth.c version
     setabc(f, bw, rp, globals);      // First compute ordinary resonator coefficients
     // Now convert to antiresonator coefficients
     rp->a = 1.0 / rp->a;             // a'=  1/a
     rp->b *= -rp->a;                 // b'= -b/a
     rp->c *= -rp->a;                 // c'= -c/a
  */
}



/*----------------------------------------------------------------------
 * function DBTOLIN
 * 
 * Convert from decibels to a linear scale factor
 * 
 * 
 * Conversion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                  ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                  ...
 *                                  0 dB -->     0
 *  
 * The just noticeable difference for a change in intensity of a vowel
 * is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 * steps.
 *
 * parwave.c: 'amptable' was a local variable here.
 */
static float DBtoLIN(long dB)  {
  // --- parwave version
  float lgtemp;
  if ((dB < 0) || (dB > 87)) return 0;
  lgtemp=amptable[dB] * .001;
  return(lgtemp);

    /* --- nsynth version
       if (dB < 0) {
         dB = 0;
       } else if (dB >= 88) {
         dB = 87;
         //if (!globals->quiet_flag) printf("Try to compute amptable[%ld]\n", dB);
       }
       return amptable[dB] * 0.001;
    */
}


/* WHAT WERE THESE FOR ? */
#if 0
# define ACOEF           0.005
# define BCOEF           (1.0 - ACOEF)	/* Slight decay to remove dc */
#endif

/*----------------------------------------------------------------------
 * function : dBconvert
 * from nsynth.c
 *//*
static float dBconvert(long int arg) {
  return 20.0 * log10((double) arg / 32767.0);
}
*/

/*----------------------------------------------------------------------
 * function : dBconvert
 * from nsynth.c
 *//*
static void overload_warning(klatt_global_ptr globals, long int arg)
{
  if (globals->warnsw == 0) {
    globals->warnsw++;
    if (!globals->quiet_flag) {
      printf("\n* * * WARNING: ");
      printf(" Signal at output of synthesizer (+%3.1f dB) exceeds 0 dB\n",
	     dBconvert(arg));
    }
  }
}
*/


/*----------------------------------------------------------------------
 *  function PITCH_SYNCH_PAR_RESET
 * 
 * Reset selected parameters pitch-synchronously.
 * See also comments to 'natglot', above.
 *
 * moo: in nsynth.c, this takes a third param 'long ns' : for us, it's in globals already
 */
static void pitch_synch_par_reset(klatt_global_ptr globals, klatt_frame_ptr frame) 
{
  long temp;
  float temp1;
  // static long skew;  // -- moo: using globals->skew

  if (frame->F0hz10 > 0)
  //if (globals->F0hz10 > 0) // nsynth
  {
    // T0 is 4* the number of samples in one pitch period
    globals->T0 = (40 * globals->samrate) / frame->F0hz10;
    //globals->T0 = (40 * globals->samrate) / globals->F0hz10; // nsynth

    globals->amp_voice = DBtoLIN(frame->AVdb);

    // Duration of period before amplitude modulation
    globals->nmod = globals->T0;
    if (frame->AVdb > 0)  {
      globals->nmod >>= 1;
    }

    // Breathiness of voicing waveform
    globals->amp_breth = DBtoLIN(frame->Aturb) * 0.1;

    // Set open phase of glottal period where  40 <= open phase <= 263
    globals->nopen = 4 * frame->Kopen;
    if ((globals->glsource == IMPULSIVE) && (globals->nopen > 263)) {
      globals->nopen = 263;
    }

    if (globals->nopen >= (globals->T0 - 1))  {
      globals->nopen = globals->T0 - 2;
      if(!globals->quiet_flag) {
	printf("Warning: glottal open period (nopen=%ld) cannot exceed T0 (=%ld), truncated\n",
	       globals->nopen, globals->T0);
      }
    }

    if (globals->nopen < 40) {
      globals->nopen = 40;              // F0 max = 1000 Hz
      if(!globals->quiet_flag)
      {
	printf("Warning: minimum glottal open period is 10 samples.");
	printf("         Truncated, nopen = %ld\n", globals->nopen);
      }
    }


    // Reset a & b, which determine shape of "natural" glottal waveform
    globals->pulse_shape_b = natglot[globals->nopen - 40];
    globals->pulse_shape_a = (globals->pulse_shape_b * globals->nopen) * 0.333;

    // Reset width of "impulsive" glottal pulse
    temp = globals->samrate / globals->nopen;
    setabc((long)0,temp,&(globals->rgl),globals);

    // Make gain at F1 about constant
    temp1 = globals->nopen *.00833;
    globals->rgl.a *= (temp1 * temp1);
    
    //
    // Truncate skewness so as not to exceed duration of closed phase
    // of glottal period.
    //
    temp = globals->T0 - globals->nopen;
    if (frame->Kskew > temp) {
      if(!globals->quiet_flag) {
	printf("Kskew duration=%ld > glottal closed period=%ld, truncate\n",
	       frame->Kskew, (globals->T0 - globals->nopen));
      }
      frame->Kskew = temp;
    }
    if (globals->skew >= 0)  {
      globals->skew = frame->Kskew;
    }
    else {
      globals->skew = - frame->Kskew;
    }

    // Add skewness to closed portion of voicing period
    globals->T0 = globals->T0 + globals->skew;
    globals->skew = - globals->skew;
  }
  else { // (F0hz10 <= 0)
    globals->T0 = 4;                     // Default for f0 undefined
    globals->amp_voice = 0.0;
    globals->nmod = globals->T0;
    globals->amp_breth = 0.0;
    globals->pulse_shape_a = 0.0;
    globals->pulse_shape_b = 0.0;
  }

  // Reset these pars pitch synchronously or at update rate if f0=0
  if ((globals->T0 != 4) || (globals->ns == 0))  {
    // Set one-pole low-pass filter that tilts glottal source
    globals->decay = (0.033 * frame->TLTdb);
    if (globals->decay > 0.0) {
      globals->onemd = 1.0 - globals->decay;
    } else {
      globals->onemd = 1.0;
    }
  }
}


/*----------------------------------------------------------------------
 * function FRAME_INIT
 * 
 * Use parameters from the input frame to set up resonator coefficients.
 *
 * Initially also get definition of fixed pars
 */
static void frame_init(klatt_global_ptr globals, klatt_frame_ptr frame) 
{
  //long Gain0;          // nsynth.c: Overall gain, 60 dB is unity : 0 to 60
  float amp_parF1;     // A1 converted to linear gain 
  float amp_parFNP;    // ANP converted to linear gain 
  float amp_parF2;     // A2 converted to linear gain 
  float amp_parF3;     // A3 converted to linear gain 
  float amp_parF4;     // A4 converted to linear gain 
  float amp_parF5;     // A5 converted to linear gain 
  float amp_parF6;     // A6 converted to linear gain 

  /*
  // -- begin nsynth
  //
  // Read  speech frame definition into temp store
  // and move some parameters into active use immediately
  // (voice-excited ones are updated pitch synchronously
  // to avoid waveform glitches).
  //
  globals->F0hz10 = frame->F0hz10;
  globals->AVdb = frame->AVdb - 7;
  if (globals->AVdb < 0) globals->AVdb = 0;
  // -- end nsynth
  */

  // -- moocow: add f0 offset
  frame->F0hz10 += globals->f0offset;

  // -- begin parwave
  globals->original_f0 = frame->F0hz10 / 10;
  if (frame->AVdb < 0) frame->AVdb = 0;
  // -- end parwave

  globals->amp_aspir = DBtoLIN(frame->ASP) * 0.05;
  globals->amp_frica = DBtoLIN(frame->AF) * 0.25;

  globals->Kskew = frame->Kskew;  // -- nsynth

  globals->par_amp_voice = DBtoLIN(frame->AVpdb);

  // 
  // Fudge factors (which comprehend affects of formants on each other?)
  // with these in place ALL_PARALLEL should sound as close as 
  // possible to CASCADE_PARALLEL.
  // Possible problem feeding in Holmes's amplitudes given this.
  //
  amp_parF1 = DBtoLIN(frame->A1) * 0.4;            // -7.96 dB
  amp_parF2 = DBtoLIN(frame->A2) * 0.15;           // -16.5 dB
  amp_parF3 = DBtoLIN(frame->A3) * 0.06;           // -24.4 dB
  amp_parF4 = DBtoLIN(frame->A4) * 0.04;           // -28.0 dB
  amp_parF5 = DBtoLIN(frame->A5) * 0.022;          // -33.2 dB
  amp_parF6 = DBtoLIN(frame->A6) * 0.03;           // -30.5 dB
  amp_parFNP = DBtoLIN(frame->ANP) * 0.6;          // -4.44 dB

  globals->amp_bypas = DBtoLIN(frame->AB) * 0.05;  // -26.0 db


  // --- parwave    
  frame->Gain0 = frame->Gain0 - 3;
  if (frame->Gain0 <= 0) frame->Gain0 = 57;
  globals->amp_gain0 = DBtoLIN(frame->Gain0);

  // Set coefficients of variable cascade resonators
  if (globals->nfcascade >= 8) {
    // nsynth: Inside Nyquist rate ?
    if (globals->samrate >= 16000)
      setabc(7500, 600, &(globals->r8c), globals);
    else
      globals->nfcascade = 6;
  }
  if (globals->nfcascade >= 7) {
    // nsynth: Inside Nyquist rate ?
    if (globals->samrate >= 16000)
      setabc(6500,500,&(globals->r7c),globals);
    else
      globals->nfcascade = 6;
  }

  // Set coefficients of variable cascade resonators
  if (globals->nfcascade >= 6)
    setabc(frame->F6hz, frame->B6hz, &(globals->r6c), globals);

  if (globals->nfcascade >= 5)    
    setabc(frame->F5hz, frame->B5hz, &(globals->r5c), globals);

  setabc(frame->F4hz, frame->B4hz, &(globals->r4c), globals);
  setabc(frame->F3hz, frame->B3hz, &(globals->r3c), globals);
  setabc(frame->F2hz, frame->B2hz, &(globals->r2c), globals);
  setabc(frame->F1hz, frame->B1hz, &(globals->r1c), globals);

  // Set coeficients of nasal resonator and zero antiresonator
  setabc(frame->FNPhz, frame->BNPhz, &(globals->rnpc), globals);
  setzeroabc(frame->FNZhz, frame->BNZhz, &(globals->rnz), globals);
  
  // Set coefficients of parallel resonators, and amplitude of outputs
  // moo: nsynth uses equivalent 'setabcg()' convenience function
  setabc(frame->F1hz,  frame->B1phz, &(globals->r1p),  globals);
  globals->r1p.a *= amp_parF1;
  setabc(frame->FNPhz, frame->BNPhz, &(globals->rnpp), globals);
  globals->rnpp.a *= amp_parFNP;
  setabc(frame->F2hz,  frame->B2phz, &(globals->r2p),  globals);
  globals->r2p.a *= amp_parF2;
  setabc(frame->F3hz,  frame->B3phz, &(globals->r3p),  globals);
  globals->r3p.a *= amp_parF3;
  setabc(frame->F4hz,  frame->B4phz, &(globals->r4p),  globals);
  globals->r4p.a *= amp_parF4;
  setabc(frame->F5hz,  frame->B5phz, &(globals->r5p),  globals);
  globals->r5p.a *= amp_parF5;
  setabc(frame->F6hz,  frame->B6phz, &(globals->r6p),  globals);
  globals->r6p.a *= amp_parF6;

  // --- nsynth
  // fold overall gain into output resonator
  //frame->Gain0 = frame->Gain0 - 3;
  //if (frame->Gain0 <= 0) frame->Gain0 = 57;

  //
  // output low-pass filter - resonator with freq 0 and BW = globals->samrate
  // Thus 3db point is globals->samrate/2 i.e. Nyquist limit.
  // Only 3db down seems rather mild...
  //
  setabc(0L, (long)globals->samrate, &(globals->rout), globals);
  globals->rout.a *= DBtoLIN(frame->Gain0);
}



/*----------------------------------------------------------------------
 * function CLIP
 *
 * From nsynth: avoid clipping, possibly complain.
 *//*
static short clip(klatt_global_ptr globals, float input) {
 long temp = input;
 // clip on boundaries of 16-bit word
 if (temp < -32767) {
   overload_warning(globals, -temp);
   temp = -32767;
 }
 else if (temp > 32767) {
   overload_warning(globals, temp);
   temp = 32767;
 }
 return temp;
}
*/


/*----------------------------------------------------------------------
 * function RESONATOR
 *
 * This is a generic resonator function. Internal memory for the resonator
 * is stored in the globals structure.
*/
static float resonator(resonator_ptr r, float input)
{
  float x = (float) (r->a * input + r->b * r->p1 + r->c * r->p2);   // parwave
  //register float x = r->a * input + r->b * r->p1 + r->c * r->p2;    // nsynth
  r->p2 = r->p1;
  r->p1 = x;
  return x;
}


/*----------------------------------------------------------------------
 * function ANTIRESONATOR
 * 
 * This is a generic anti-resonator function. The code is the same as resonator 
 * except that a,b,c need to be set with setzeroabc() and we save inputs in 
 * p1/p2 rather than outputs. There is currently only one of these - "rnz"
 *
 * Output = (rnz.a * input) + (rnz.b * oldin1) + (rnz.c * oldin2) 
 */
static float antiresonator(resonator_ptr r, float input)
{
  //register float x = r->a * input + r->b * r->p1 + r->c * r->p2; // parwave & nsynth
  float x = r->a * input + r->b * r->p1 + r->c * r->p2;
  r->p2 = r->p1;
  r->p1 = input;
  return x;
}



/*======================================================================
 * function PARWAVE
 * 
 * Converts synthesis parameters to a waveform.
*/
void parwave(klatt_global_ptr globals, klatt_frame_ptr frame,
	     float *output, long maxsamples, char doinit)
{
  float out = 0;          // Output of cascade branch, also final output
  float outbypas;

  if (doinit != FALSE) {
    frame_init(globals,frame);  /* get parameters for next frame of speech */
    flutter(globals,frame);  /* add f0 flutter */
    //globals->time_count++; // nsynth-version
  }


  //
  // MAIN LOOP, for each output sample of current frame:
  //
  for (globals->ns=0;
       globals->ns<globals->nspfr && globals->ns<maxsamples;
       globals->ns++) 
  {
    //static unsigned long seed = 5; // Fixed staring value
    float noise = 0;
    //int n4; // moo: only needed for 4*samplerate loop
    float sourc;                   // Sound source if all-parallel config used 
    float glotout;                 // Output of glottal sound source 
    float par_glotout;             // Output of parallelglottal sound sourc 
    float voice;                   // Current sample of voicing waveform 
    float frics;                   // Frication sound source 
    float aspiration;              // Aspiration sound source 
    //long nrand;                    // Varible used by random number generator 

    // -- parwave: Get low-passed random number for aspiration and frication noise
    noise = gen_noise(noise,globals);

    /* --- nsynth:
    // Our own code like rand(), but portable
    //  whole upper 31 bits of seed random 
    // assumes 32-bit unsigned arithmetic
    //  with untested code to handle larger.
    seed = seed * 1664525 + 1;
    if (8 * sizeof(unsigned long) > 32)
      seed &= 0xFFFFFFFF;

    // Shift top bits of seed up to top of long then back down to LS 14 bits
    // Assumes 8 bits per sizeof unit i.e. a "byte"
    nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);
    
    // Tilt down noise spectrum by soft low-pass filter having
    //   a pole near the origin in the z-plane, i.e.
    //   output = input + (0.75 * lastoutput)
    noise = nrand + (0.75 * nlast);	// Function of samp_rate ?
    nlast = noise;
    */


    //
    // Amplitude modulate noise (reduce noise amplitude during
    // second half of glottal period) if voicing simultaneously present.
    //
    if (globals->nper > globals->nmod)  {
      noise *= (float) 0.5;
    }

    // Compute frication noise
    sourc = frics = globals->amp_frica * noise;

    //
    // Compute voicing waveform. Run glottal source simulation at 4 
    // times normal sample rate to minimize quantization noise in 
    // period of female voice.
    //
    // moocow: don't bother
    //
    ////for (n4=0; n4 < 4; n4++) {
      switch(globals->glsource)
      {
      case IMPULSIVE:
	// Use impulsive glottal source
	voice = impulsive_source(globals);
	break;
      case SAMPLED:
	// Use sampled data (not in nsynth!)
	voice = sampled_source(globals);
	break;
      case NATURAL:
      default:
	// Or use a more-natural-shaped source waveform with excitation
        // occurring both upon opening and upon closure, stronest at closure
	voice = natural_source(globals);	
      }

      // Reset period when counter 'nper' reaches T0
      if (globals->nper >= globals->T0) 
      {
	globals->nper = 0;
	pitch_synch_par_reset(globals,frame);
      }

      //        
      // Low-pass filter voicing waveform before downsampling from 4*samrate
      // to samrate samples/sec.  Resonator f=.09*samrate, bw=.06*samrate 
      //
      voice = resonator(&(globals->rlp),voice);  // in=voice, out=voice

      // Increment counter that keeps track of 4*samrate samples per sec
      // moo: don't bother with oversampling & interpolation
      globals->nper += 4;
      ////globals->nper++;
      ////}

    //
    // Tilt spectrum of voicing source down by soft low-pass filtering, amount
    // of tilt determined by TLTdb
    //
    // moo: using globals->vlast (vs. static vlast)
    voice = (voice * globals->onemd) + (globals->vlast * globals->decay);
    globals->vlast = voice;


    //
    // Add breathiness during glottal open phase. Amount of breathiness 
    // determined by parameter Aturb.  Use nrand rather than noise because 
    // noise is low-passed. 
    //
    if (globals->nper < globals->nopen) {
      voice += globals->amp_breth * globals->nrand;
    }


    // Set amplitude of voicing
    glotout = globals->amp_voice * voice;
    par_glotout = globals->par_amp_voice * voice;  // parwave-only

    // Compute aspiration amplitude and add to voicing source
    aspiration = globals->amp_aspir * noise;
    glotout += aspiration;
  
    par_glotout += aspiration;                     // parwave
    //par_glotout = glotout;                       // nsynth


    if (globals->synthesis_model != ALL_PARALLEL) {
      // Cascade vocal tract, excited by laryngeal sources.
      float casc_next_in = antiresonator(&(globals->rnz),glotout);   // parwave
      casc_next_in = resonator(&(globals->rnpc),casc_next_in);       // parwave

      /* --- nsynth
      // Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
      float rnzout = antiresonator(&(globals->rnz), glotout);   // Output of cascade nazal zero resonator
      float casc_next_in = resonator(&(globals->rnpc), rnzout); // in=rnzout, out=rnpc.p1
      */

      //
      // Recoded from sequence of if's to use C's fall through switch
      // semantics. May allow compiler to optimize.
      // From Nick's 'nsynth.c' (safe)
      //
      switch (globals->nfcascade) {
      case 8:
	// Do not use unless sample rate >= 16000
	casc_next_in= resonator(&(globals->r8c),casc_next_in);
      case 7:
	// Do not use unless sample rate >= 16000
	casc_next_in = resonator(&(globals->r7c),casc_next_in);          
      case 6:
	// Do not use unless long vocal tract or sample rate increased
	casc_next_in = resonator(&(globals->r6c),casc_next_in);
      case 5:
	casc_next_in = resonator(&(globals->r5c),casc_next_in);
      case 4:
	casc_next_in = resonator(&(globals->r4c),casc_next_in);
      case 3:
	casc_next_in = resonator(&(globals->r3c),casc_next_in);
      case 2:
	casc_next_in = resonator(&(globals->r2c),casc_next_in);
      case 1:
	out = resonator(&(globals->r1c),casc_next_in);
	break;
      default:
	// we are not using the cascade tract, set out to zero
	out = 0.0; 
      }
      /* nsynth:
	 #if 0
	 // Excite parallel F1 and FNP by voicing waveform
	 // Source is voicing plus aspiration
	 // Add in phase, boost lows for nasalized
	 out += (resonator(&(globals->rnpp), par_glotout) + resonator(&(globals->r1p), par_glotout));
	 #endif
      */
    }
    else {
      // Is ALL_PARALLEL
      out = 0.0;  // -- parwave

      /*
      // --- nsynth
      //
      // NIS - rsynth "hack"
      // As Holmes' scheme is weak at nasals and (physically) nasal cavity
      // is "back near glottis" feed glottal source through nasal resonators
      // Don't think this is quite right, but improves things a bit
      //
      par_glotout = antiresonator(&(globals->rnz), par_glotout);
      par_glotout = resonator(&(globals->rnpc), par_glotout);

      // And just use r1p NOT rnpp
      out = resonator(&(globals->r1p), par_glotout);

      // Sound sourc for other parallel resonators is frication
      // plus first difference of voicing waveform.
      sourc += (par_glotout - globals->glotlast);
      globals->glotlast = par_glotout;
      */
    }


    // -- BEGIN parwave-only
    // Excite parallel F1 and FNP by voicing waveform
    //
    // moo: use globals instead of statics
    sourc = par_glotout;       // Source is voicing plus aspiration

    // moo: hack!
#if 0
#if 0
    fprintf(stderr,"voice=%g, par_glotout=%g, frics=%g\n",
	    voice, par_glotout, frics);
#endif
    if (voice != 0 || par_glotout != 0 || frics != 0 ||
	aspiration != 0 || globals->amp_bypas != 0) {
#endif
       //
      // Standard parallel vocal tract Formants F6,F5,F4,F3,F2, 
      // outputs added with alternating sign. Sound sourc for other 
      // parallel resonators is frication plus first difference of 
      // voicing waveform. 
      //
      out += resonator(&(globals->r1p),sourc);
      out += resonator(&(globals->rnpp),sourc);
      
      sourc = frics + par_glotout - globals->glotlast;
      globals->glotlast = par_glotout;
      // -- END parwave only

      out = resonator(&(globals->r6p), sourc) - out;
      out = resonator(&(globals->r5p), sourc) - out;
      out = resonator(&(globals->r4p), sourc) - out;
      out = resonator(&(globals->r3p), sourc) - out;
      out = resonator(&(globals->r2p), sourc) - out;
      
      outbypas = globals->amp_bypas * sourc; // parwave
      out = outbypas - out;                  // parwave

      out = globals->amp_bypas * sourc - out; // nsynth
      
      
#  ifdef PARWAVE_DEBUG
      fprintf(stderr, "pre-switch: out=%f; ", out);
#  endif

      // -- BEGIN parwave only
      if (globals->outsl != 0)  {
	switch(globals->outsl) {
	case 1:
	  out = voice;
	  break;
	case 2:
	  out = aspiration;
	  break;
	case 3: 
	  out = frics;
	  break;
	case 4:
	  out = glotout;
	  break;
	case 5:
	  out = par_glotout;
	  break;
	case 6:
	  out = outbypas;
	  break;
	case 7:
	  out = sourc;
	  break;
	}
      }
      // -- END parwave-only

#  ifdef PARWAVE_DEBUG
      fprintf(stderr, "pre-resonator: out=%f; ", out);
#  endif

      //out = resonator(&rout, out); // nsynth
      //*jwave++ = clip(globals, out); // nsynth: Convert back to integer */

    *output++ = (float)(resonator(&(globals->rout), out) * globals->amp_gain0 / 32767.0);
#if 0
    } // hack!
    else {
      *output++ = 0;
    } // END hack!
#endif

    //*output++ = (float)(resonator(&(globals->rout), out) * / 32767.0); // -- nsynth
  }
}




/*----------------------------------------------------------------------
 * function PARWAVE_INIT
 * 
 * Initialises all parameters used in parwave, sets resonator internal memory
 * to zero.
 */
void parwave_init(klatt_global_ptr globals)
{
  globals->FLPhz = (950 * globals->samrate) / 10000;
  globals->BLPhz = (630 * globals->samrate) / 10000;

  globals->minus_pi_t = -PI / globals->samrate;
  globals->two_pi_t = -2.0 * globals->minus_pi_t;

  setabc(globals->FLPhz, globals->BLPhz, &(globals->rlp), globals);
  globals->nper = 0;
  globals->T0 = 0;

  globals->nopen = 0; // parwave
  globals->nmod = 0;  // parwave

  // imported data from nsynth.c : initialize to zero  (???)
  globals->time_count = 0;
  globals->warnsw = 0;
  globals->skew = 0;
  globals->vwave = 0.0;
  globals->vlast = 0.0;
  globals->nlast = 0.0;
  globals->glotlast = 0.0;
  globals->decay = 0.0;
  globals->onemd = 0.0;

  // --- resonators
  globals->rnpp.p1 = 0;                     // parallel nasal pole 
  globals->rnpp.p2 = 0;

  globals->r1p.p1 = 0;                      // parallel 1st formant
  globals->r1p.p2 = 0;

  globals->r2p.p1 = 0;                      // parallel 2nd formant
  globals->r2p.p2 = 0;

  globals->r3p.p1 = 0;                      // parallel 3rd formant
  globals->r3p.p2 = 0;

  globals->r4p.p1 = 0;                      // parallel 4th formant
  globals->r4p.p2 = 0;

  globals->r5p.p1 = 0;                      // parallel 5th formant
  globals->r5p.p2 = 0;

  globals->r6p.p1 = 0;                      // parallel 6th formant
  globals->r6p.p2 = 0;

  globals->r1c.p1 = 0;                      // cascade 1st formant 
  globals->r1c.p2 = 0;

  globals->r2c.p1 = 0;                      // cascade 2nd formant 
  globals->r2c.p2 = 0;

  globals->r3c.p1 = 0;                      // cascade 3rd formant 
  globals->r3c.p2 = 0;

  globals->r4c.p1 = 0;                      // cascade 4th formant 
  globals->r4c.p2 = 0;

  globals->r5c.p1 = 0;                      // cascade 5th formant 
  globals->r5c.p2 = 0;

  globals->r6c.p1 = 0;                      // cascade 6th formant 
  globals->r6c.p2 = 0;

  globals->r7c.p1 = 0;
  globals->r7c.p2 = 0;

  globals->r8c.p1 = 0;
  globals->r8c.p2 = 0;

  globals->rnpc.p1 = 0;                     // cascade nasal pole 
  globals->rnpc.p2 = 0;

  globals->rnz.p1 = 0;                      // cascade nasal zero 
  globals->rnz.p2 = 0;

  globals->rgl.p1 = 0;                      // crit-damped glot low-pass filter
  globals->rgl.p2 = 0;

  globals->rlp.p1 = 0;                      // downsamp low-pass filter 
  globals->rlp.p2 = 0;

}


/*----------------------------------------------------------------------
 * function GEN_NOISE
 * 
 * Random number generator (return a number between -8191 and +8191) 
 * Noise spectrum is tilted down by soft low-pass filter having a pole near 
 * the origin in the z-plane, i.e. output = input + (0.75 * lastoutput) 
 */
static float gen_noise(float noise, klatt_global_ptr globals) 
{
  long temp;
  static float nlast;

  temp = (long) getrandom(-8191,8191);
  globals->nrand = (long) temp;

  noise = globals->nrand + (0.75 * nlast);
  nlast = noise;

  return(noise);
}

