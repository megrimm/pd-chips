/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: klatt~.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD
 *
 *     A PD external derived from John Iles' and Nick Ing-Simmons'
 *     implementation of the Klatt Cascade-Parallel Formant
 *     Speech Synthesizer.
 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/

#include "klatt_frame.h"

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <math.h>
#include <string.h>  // for memcpy()
#include <stdlib.h>  // for free()


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define KLATT_DEBUG 1
//#undef KLATT_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/

/* --- from Jon's "klatt.c" --- */
#define NUMBER_OF_SAMPLES 100
#define SAMPLE_FACTOR 0.00001
static int natural_samples[NUMBER_OF_SAMPLES]=
  {
    -310,-400,530,356,224,89,23,-10,-58,-16,461,599,536,701,770,
    605,497,461,560,404,110,224,131,104,-97,155,278,-154,-1165,
    -598,737,125,-592,41,11,-247,-10,65,92,80,-304,71,167,-1,122,
    233,161,-43,278,479,485,407,266,650,134,80,236,68,260,269,179,
    53,140,275,293,296,104,257,152,311,182,263,245,125,314,140,44,
    203,230,-235,-286,23,107,92,-91,38,464,443,176,98,-784,-2449,
    -1891,-1045,-1600,-1462,-1384,-1261,-949,-730
  };


/*=====================================================================
 * klatt_tilde
 *=====================================================================*/
static t_class *klatt_tilde_class;
typedef struct _klatt_tilde
{
  t_object x_obj;                    /* black magic (probably inheritance-related) */
  t_float         enabled;           /* global on/off switch */
  klatt_global_t  kglobals;          /* per-synthesizer globals */
  klatt_frame_t   lframe;            /* last frame (to avoid dropouts) */
  klatt_frame_t   wframe;            /* working frame */
  t_int           msec_per_frame;    /* milliseconds per frame */
  long            samples_remaining; /* samples remaining for working frame */
  char            got_new_frame;     /* flag on receive new frame data */
  char            zero_frame_state;  /* 1: got null-frame, 2: played null-frame */
  t_outlet       *b_out;             /* bang-outlet on begin new frame (ouch) */
} t_klatt_tilde;


/*=====================================================================
 * pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * klatt_tilde_configure(): configure according to keyword arguments
 *--------------------------------------------------------------------*/
void klatt_tilde_configure(t_klatt_tilde *x, t_symbol *sel, int argc, t_atom *argv)
{
  for ( ; argc >= 0; argc -= 2) {
    if (sel == gensym("mspf")) {
      // -- Milliseconds per synthesizer frame (hack: forced to be a multiple of pd-blocksize!)
      t_int mspf = argc > 0 ? atom_getint(argv) : 0;
      x->msec_per_frame = mspf > 0 ? (t_int)mspf : 10;
    }
    else if (sel == gensym("ncasc")) {
      // -- Number of formants in cascade vocal tract : zero means disable cascade tract
      t_int ncasc = argc > 0 ? atom_getint(argv) : 0;
      if (ncasc <= 0) {
	x->kglobals.synthesis_model = ALL_PARALLEL;
      } else {
	x->kglobals.synthesis_model = CASCADE_PARALLEL;
	x->kglobals.nfcascade = ncasc;
      }
    }
    else if (sel == gensym("verbose")) {
      // -- Enable verbose warnings?
      t_int verbose = argc > 0 ? atom_getint(argv) : 1;
      x->kglobals.quiet_flag = !verbose;
    } 
    else if (sel == gensym("gls")) {
      // -- Type of glottal source: 0=NATURAL(default) 1=IMPULSIVE
      t_int gls = argc > 0 ? atom_getint(argv) : 0;
      if (gls == 1) {
	// -- IMPLUSIVE glottal source
	x->kglobals.glsource = IMPULSIVE;
      } else {
	// -- Default: NATURAL glottal source
	x->kglobals.glsource = NATURAL;
      }
    }
    else if (sel == gensym("flutter")) {
      // -- Percentage of f0 flutter 0-100
      t_int flutter = argc > 0 ? atom_getint(argv) : 0;
      if (flutter < 0) flutter = -flutter;
      x->kglobals.f0_flutter = flutter > 99 ? 100 : flutter;
    }
    else {
      // -- Unknown: assume it's a frame parameter
      //post("klatt~: unknown configuration parameter '%s' -- ignored.", sel ? sel->s_name : "(nil)");
      klatt_frame_set_kw("klatt_tilde_anything()", &(x->wframe), sel, argc, argv);
      // -- restart frame
      x->got_new_frame = 1;
      x->samples_remaining = 0;
    }

    // -- update counters
    sel = argc > 1 ? atom_getsymbol(++argv) : NULL;
    ++argv;
  }
}

/*--------------------------------------------------------------------
 * klatt_tilde_anything : alter some current frame parameters
 *--------------------------------------------------------------------*/
/*
  void klatt_tilde_anything(t_klatt_tilde *x, t_symbol *sel, int argc, t_atom *argv) {
  klatt_frame_set_kw("klatt_tilde_anything()", &(x->wframe), sel, argc, argv);
  // -- restart frame
  x->got_new_frame = 1;
  x->samples_remaining = 0;
}
*/
void klatt_tilde_anything(t_klatt_tilde *x, t_symbol *sel, int argc, t_atom *argv)
{
  klatt_tilde_configure(x,sel,argc,argv);
}


/*--------------------------------------------------------------------
 * klatt_tilde_set : set whole current working frame
 *--------------------------------------------------------------------*/
void klatt_tilde_set(t_klatt_tilde *x, t_symbol *sel, int argc, t_atom *argv) {
#ifdef KLATT_DEBUG
  post("klatt~: klatt_tilde_set() called with %d arguments.", argc);
#endif

  klatt_frame_set_l("klatt_tilde_set()", (long *)&(x->wframe), argc, argv);
  // -- restart frame
  x->got_new_frame = 1;
  x->samples_remaining = 0;
}

/*--------------------------------------------------------------------
 * klatt_tilde_float : adjust fundamental frequency (input: Hz)
 *--------------------------------------------------------------------*/
void klatt_tilde_float(t_klatt_tilde *x, t_floatarg f) {
#ifdef KLATT_DEBUG
  post("klatt_tilde_float(): [f0 offset] called with arg=%f", f);
#endif
  x->kglobals.f0offset = (long)f*10;
}

/*--------------------------------------------------------------------
 * klatt_tilde_clear : zero the current and last frames
 *--------------------------------------------------------------------*/
void klatt_tilde_clear(t_klatt_tilde *x) {
  memset(&(x->lframe),0,sizeof(klatt_frame_t));
  memset(&(x->wframe),0,sizeof(klatt_frame_t));
  // -- restart frame
  x->got_new_frame = 1;
  x->samples_remaining = 0;
}


/*--------------------------------------------------------------------
 * new SIZE
 *--------------------------------------------------------------------*/
static void *klatt_tilde_new(t_symbol *sel, int argc, t_atom *argv)
{
  t_klatt_tilde *x;

  x = (t_klatt_tilde *)pd_new(klatt_tilde_class);

  /*-- shared initialization  --*/
  x->kglobals.samrate = sys_getsr();
  x->kglobals.natural_samples = natural_samples;
  x->kglobals.num_samples = NUMBER_OF_SAMPLES;
  x->kglobals.sample_factor = (float)SAMPLE_FACTOR;
  x->kglobals.outsl = 0;   // -- output waveform selector (?!)

  /* -- frame initialization -- */
  klatt_tilde_clear(x);

  /*-- configuration: defaults --*/
  x->msec_per_frame = 10;
  x->kglobals.synthesis_model = ALL_PARALLEL;
  x->kglobals.quiet_flag = 1;
  x->kglobals.glsource = NATURAL;
  x->kglobals.f0_flutter = 0;

  /*-- configuration: instantiation arguments --*/
  if (argc) klatt_tilde_configure(x, atom_getsymbol(argv), argc-1, argv+1);


  /*-- pd i/o --*/

  //-- null-frame detection hack
  x->zero_frame_state = 2;

  //-- switching inlet
  x->enabled = 1;
  floatinlet_new(&x->x_obj, &x->enabled);

  //-- signal outlet
  outlet_new(&x->x_obj, &s_signal);

  //-- bang-on-last-frame outlet
  x->b_out = outlet_new(&x->x_obj, &s_bang);

  return (void *)x;
}

/*--------------------------------------------------------------------
 * utility: have zero frame
 *--------------------------------------------------------------------*/
char klatt_tilde_have_null_frame(t_klatt_tilde *x)
{
  return (x->wframe.Gain0 <= 0
	  || (x->wframe.AVdb <= 0
	      && x->wframe.ASP <= 0 
	      && x->wframe.Aturb <= 0 
	      && x->wframe.AF <= 0
	      && x->wframe.A1 <= 0
	      && x->wframe.A2 <= 0
	      && x->wframe.A3 <= 0
	      && x->wframe.A4 <= 0
	      && x->wframe.A5 <= 0
	      && x->wframe.A6 <= 0
	      && x->wframe.ANP <= 0
	      && x->wframe.AB <= 0
	      && x->wframe.AVpdb <= 0));
}

/*--------------------------------------------------------------------
 * perform method : produce bona fide signal output
 *--------------------------------------------------------------------*/
t_int *klatt_tilde_perform(t_int *w)
{
  t_klatt_tilde     *x = (t_klatt_tilde *)(w[1]);
  t_sample        *out = (t_sample *)(w[2]);
  int                n =        (int)(w[3]);
  char          doinit = FALSE;

  // -- output *something*
  if (x->enabled) {
    if (x->samples_remaining <= 0) {
      // -- time for a new parwave-frame
      if (x->got_new_frame) {
	//
	// save the new current frame in 'lframe' : we have to copy
	// here (rather than use pointers), because parwave() alters
	// the frame in the course of its computation (argh)
	//
	x->lframe = x->wframe;
	x->got_new_frame = 0;
      } else {
	// -- try and re-use the last frame
	x->wframe = x->lframe;
      }
      //-- zero frame detection hack
      if (klatt_tilde_have_null_frame(x)) {
	x->zero_frame_state = x->zero_frame_state ? 2 : 1;
      }
      else {
	x->zero_frame_state = 0;
      }
      // -- started a new frame : request more (hack!)
      outlet_bang(x->b_out);
      x->samples_remaining = x->kglobals.nspfr;
      doinit = TRUE;
    }
  }

  // -- generate audio for the current frame
  if (!x->enabled || x->zero_frame_state > 1)
    {
      while (n--) *out++ = 0;
      return (w+4);
    }
  else
    {
      parwave(&(x->kglobals), &(x->wframe), out, n, doinit);
    }

  if (x->enabled) x->samples_remaining -= n;

  return (w+4);
}

/*--------------------------------------------------------------------
 * dsp method : called when audio is enabled
 *--------------------------------------------------------------------*/
void klatt_tilde_dsp(t_klatt_tilde *x, t_signal **sp)
{
  int blksize = sys_getblksize();
  double nbpf; // pd-blocks per frame

  // -- last-minute synth setup --
  nbpf = ceil(x->kglobals.samrate * x->msec_per_frame / (blksize*1000.0));
  x->kglobals.nspfr = (long)blksize*nbpf;
  parwave_init(&(x->kglobals));

# ifdef KLATT_DEBUG
  post("klatt~: blksize=%ld, samrate=%ld, mspf=%ld, nbpf=%f, nspfr=%ld",
       (long)blksize,
       (long)x->kglobals.samrate,
       (long)x->msec_per_frame,
       nbpf,
       (long)x->kglobals.nspfr);
# endif

  // -- register perform method --
  dsp_add(klatt_tilde_perform,  // perform method
	  3,                    // num/args
	  x,                    // arguments...
          sp[0]->s_vec,         // outlet sigvec
	  sp[0]->s_n            // outlet sigsize
	  );
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void klatt_tilde_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:         klatt~ : Klatt cascade-parallel formant speech synthesizer");
#endif

  /* ensure that klatt_frame() has been initialized */
  klatt_frame_setup();

  /* register class */
  klatt_tilde_class = class_new(gensym("klatt~"),              // name 
				(t_newmethod)klatt_tilde_new,  // newmethod
				0,//(t_method)klatt_tilde_free,    // freemethod
				sizeof(t_klatt_tilde),         // size
				CLASS_DEFAULT,                 // flags
				A_GIMME,                       // args
				0);
  
  /* dsp method */
  class_addmethod(klatt_tilde_class, (t_method)klatt_tilde_dsp, gensym("dsp"), 0);

  /* parameter-setting methods */
  class_addfloat(klatt_tilde_class,  klatt_tilde_float);
  class_addmethod(klatt_tilde_class, (t_method)klatt_tilde_set,   &s_list,       A_GIMME, 0);
  class_addmethod(klatt_tilde_class, (t_method)klatt_tilde_set,   gensym("set"), A_GIMME, 0);
  class_addmethod(klatt_tilde_class, (t_method)klatt_tilde_clear, gensym("clear"), 0);
  class_addanything(klatt_tilde_class, klatt_tilde_anything);

  class_sethelpsymbol(klatt_tilde_class, gensym("klatt~-help.pd"));
}
