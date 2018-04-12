/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: holmes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: phoneme-to-klatt-frame conversion
 *
 *     A PD external derived from Nick Ing-Simmons'
 *     implementation of the phoneme to vocal tract parameters
 *     system described in:
 *
 *        J. N. Holmes, I. Mattingly, and J. Shearme, "Speech synthesis by rule",
 *        Language and Speech 7, pp 127--143, 1964.
 *
 *
 * Copyright (c) 2003 Bryan Jurish.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/


/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#include <math.h>
#include <string.h>  // for memcpy()
#include <stdlib.h>  // for free()

#include "dsqueue.h"

#include "proto.h"
#include "parwave.h"
#include "klatt_frame.h"
#include "phfeat.h"
#include "rholmes.h"
#include "phtoelm.h"
#include "pd_holmes.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*=====================================================================
 * Globals
 *=====================================================================*/

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define HOLMES_DEBUG 1
//#undef HOLMES_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/


/*=====================================================================
 * holmes_class
 *=====================================================================*/
static t_class *holmes_class;
typedef struct _holmes
{
  t_object x_obj;                    // black magic (probably inheritance-related)
  holmes_global_t  hg;               // global data for Holmes algorithm
  holmes_state_t   hs;               // state data for Holmes algorithm
  t_atom           aframe[NPAR];     // pd-friendly frame-list data
  t_outlet         *kfr_out;         // list-outlet for Klatt frames
  t_outlet         *eoq_out;         // bang-outlet on end-of-queue
} t_holmes;


/*=====================================================================
 * pd methods
 *=====================================================================*/


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *holmes_new(t_symbol *sel, int argc, t_atom *argv)
{
  t_holmes *x;
  x = (t_holmes *)pd_new(holmes_class);

  /* initialize Holmes structures to defaults */
  holmes_init_global(&(x->hg));
  holmes_init_state(&(x->hs));
  holmes_init_utterance(&(x->hg),&(x->hs));

  /* intialize pd-passable frame-list */
  init_klatt_frame_alist(x->aframe);

  /* arguments (any known parameters) */
  if (argc) {
    pd_holmes_set_kw("holmes_new()",
		     &(x->hg), &(x->hs),
		     atom_getsymbol(argv), argc-1, argv+1);
  }

  /* -- outlets -- */
  x->kfr_out = outlet_new(&x->x_obj, &s_list);
  x->eoq_out = outlet_new(&x->x_obj, &s_bang);

  return (void *)x;
}

void holmes_free(t_holmes *x) {
  /* free Holmes structures */
  holmes_free_global(&(x->hg));
  holmes_free_state(&(x->hs));
}

/*--------------------------------------------------------------------
 * clear : clear state data
 */
void holmes_clear(t_holmes *x) {
  holmes_clear_state(&(x->hs));
}

/*--------------------------------------------------------------------
 * add ELT DUR STRESS ... : append a new utterance
 */
void holmes_add(t_holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  t_atom a;

  if (!argc) return;
  SETFLOAT(&a,0);
  atoms_to_holmes_eltq("holmes_add()", 1, &a, x->hs.eltq);
  atoms_to_holmes_eltq("holmes_add()", argc, argv, x->hs.eltq);
}

/*--------------------------------------------------------------------
 * add2 ELT DUR STRESS ... : append elements to the current utterance
 */
void holmes_add2(t_holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  if (!argc) return;
  atoms_to_holmes_eltq("holmes_add2()", argc, argv, x->hs.eltq);
}

/*--------------------------------------------------------------------
 * set PHONES ... : set contents of utterance-buffer
 */
void holmes_set(t_holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  holmes_clear_state(&(x->hs));
  holmes_add(x,sel,argc,argv);
}

/*--------------------------------------------------------------------
 * bang : output the next klatt frame as a list
 */
void holmes_bang(t_holmes *x) {
  // -- get the next frame
  holmes_compute_next_frame(&(x->hg),&(x->hs));

  if (dsqueue_empty(x->hs.eltq)) {
    // -- report end-of-queue
    outlet_bang(x->eoq_out);
  }

  // -- reset end-of-utterance flag
  x->hs.flags &= ~(HOLMES_FLAG_EOU|HOLMES_FLAG_EOW);

  // -- finally, output the klatt frame
  klatt_frame_to_alist((long *)&(x->hs.pars), x->aframe);
  outlet_list(x->kfr_out, &s_list, NPAR, x->aframe);
}

/*--------------------------------------------------------------------
 * anything : set named parameters
 */
void holmes_anything(t_holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  pd_holmes_set_kw("holmes_set_anything()", &(x->hg), &(x->hs), sel, argc, argv);
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void holmes_setup(void) {
  /* ensure that our dependencies have been satisfied */
  klatt_frame_setup();
  pd_holmes_setup();

#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:         holmes : Holmes/Mattingly/Shearme phonetic interpreter");
#endif

  /* register class */
  holmes_class = class_new(gensym("holmes"),              // name 
			   (t_newmethod)holmes_new,       // newmethod
			   (t_method)holmes_free,         // freemethod
			   sizeof(t_holmes),              // size
			   CLASS_DEFAULT,                 // flags
			   A_GIMME,                       // args
			   0);
  /* --- methods --- */
  class_addmethod(holmes_class, (t_method)holmes_clear, gensym("clear"), 0);
  class_addmethod(holmes_class, (t_method)holmes_add,   gensym("add"),  A_GIMME, 0);
  class_addmethod(holmes_class, (t_method)holmes_add2,  gensym("add2"), A_GIMME, 0);
  class_addmethod(holmes_class, (t_method)holmes_add2,  &s_list,        A_GIMME, 0);
  class_addmethod(holmes_class, (t_method)holmes_set,   gensym("set"),  A_GIMME, 0);
  class_addbang(holmes_class,   holmes_bang);

  /* -- arbitrary parameter-setting method */
  class_addanything(holmes_class, holmes_anything);

  class_sethelpsymbol(holmes_class, gensym("holmes-help.pd"));
}
