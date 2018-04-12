/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: phones2holmes.c
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
 * Copyright (c) 2002 Bryan Jurish.
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

#include <stdlib.h>  // for malloc() and free()

#include "dsqueue.h"
#include "proto.h"
#include "rholmes.h"
#include "pd_holmes.h"
#include "phtoelm.h"
#include "pd_phtoelm.h"

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
 * holmes_phones_class : dummy object for phone-string help
 *=====================================================================*/
static t_class *holmes_phones_class;
typedef struct _holmes_phones
{
  t_object x_obj;
} t_holmes_phones;

void *holmes_phones_new(void)
{
  t_holmes_phones *x = (t_holmes_phones *)pd_new(holmes_phones_class);
  return (void *)x;
}

void holmes_phones_setup(void) {
  holmes_phones_class = class_new(gensym("holmes-phones"),
				  holmes_phones_new,
				  0,
				  sizeof(t_holmes_phones),
				  CLASS_NOINLET,
				  0);
  class_sethelpsymbol(holmes_phones_class, gensym("holmes-phones.pd"));
}


/*=====================================================================
 * phones2holmes_class
 *=====================================================================*/
static t_class *phones2holmes_class;
typedef struct _phones2holmes
{
  t_object         x_obj;            // black magic (probably inheritance-related)
  phtoelm_state_t  ps;               // state data for phtoelm conversion
  dsqueue_t        *eltq;            // queue of encoded holmes-elements
  t_atom           *aelts;           // pd-friendly atom-list
  t_outlet         *elt_out;         // list-outlet for current element
} t_phones2holmes;


/*=====================================================================
 * holmes2phones: pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *phones2holmes_new(t_symbol *sel)
{
  t_phones2holmes *x;

  x = (t_phones2holmes *)pd_new(phones2holmes_class);

  /* initialize Holmes structures to defaults */
  phtoelm_init_state(&(x->ps));

  /* initialize queue */
  x->eltq = dsqueue_new(HOLMES_QUEUE_BLOCKSIZE);

  /* initialize atom-list */
  x->aelts = (t_atom *)malloc(3*sizeof(t_atom));
  SETSYMBOL(&x->aelts[0],gensym("END"));
  SETFLOAT(&x->aelts[1],0);
  SETFLOAT(&x->aelts[2],0);

  /* outlet(s) */
  x->elt_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void phones2holmes_free(t_phones2holmes *x) {
  /* free Holmes structures */
  phtoelm_free_state(&(x->ps));
  if (x->aelts) free(x->aelts);
}

/*--------------------------------------------------------------------
 * clear : clear state data
 */
void phones2holmes_clear(t_phones2holmes *x) {
  phtoelm_clear_state(&(x->ps));
}

/*--------------------------------------------------------------------
 * phones2holmes_output_elts() : output the current element-queue
 */
void phones2holmes_output_elts(t_phones2holmes *x) {
  while (!dsqueue_empty(x->eltq)) {
    holmes_qelt_t hqe = (holmes_qelt_t)dsqueue_shift(x->eltq);
    x->aelts[0].a_w.w_symbol = gensym(Elements[hqeGetEID(hqe)].name);
    x->aelts[1].a_w.w_float  = (t_float)hqeGetDur(hqe);
    x->aelts[2].a_w.w_float  = (t_float)hqeGetStr(hqe);
    outlet_list(x->elt_out, &s_list, 3, x->aelts);
  }
}

/*--------------------------------------------------------------------
 * add PHONES ... : append a new utterance
 */
void phones2holmes_add(t_phones2holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  t_atom a;

  if (!argc) return;
  SETFLOAT(&a,0);
  atoms_to_holmes_eltq("phones2holmes_add()", 1, &a, x->eltq);
  atom_phones_to_eltq("phones2holmes_add()", argc, argv, &x->ps, x->eltq);

  phones2holmes_output_elts(x);
}

/*--------------------------------------------------------------------
 * add2 PHONES ... : append elements to the current utterance
 */
void phones2holmes_add2(t_phones2holmes *x, t_symbol *sel, int argc, t_atom *argv) {
  if (!argc) return;
  atom_phones_to_eltq("phones2holmes_add2()", argc, argv, &x->ps, x->eltq);
  phones2holmes_output_elts(x);
}


/*--------------------------------------------------------------------
 * phones2holmes: setup
 *--------------------------------------------------------------------*/
void phones2holmes_setup(void) {
  /* ensure that our dependencies have been satisfied */
  pd_holmes_setup();
  pd_phtoelm_setup();
  holmes_phones_setup();

#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:  phones2holmes : phone-string to Holmes-element converter");
#endif

  /* register class */
  phones2holmes_class = class_new(gensym("phones2holmes"),              // name 
				  (t_newmethod)phones2holmes_new,       // newmethod
				  (t_method)phones2holmes_free,         // freemethod
				  sizeof(t_phones2holmes),              // size
				  CLASS_DEFAULT,                        // flags
				  //A_GIMME,                             // args
				  0);

  /* --- methods --- */
  class_addmethod(phones2holmes_class, (t_method)phones2holmes_clear, gensym("clear"), 0);
  class_addmethod(phones2holmes_class, (t_method)phones2holmes_add,   gensym("add"),  A_GIMME, 0);
  class_addmethod(phones2holmes_class, (t_method)phones2holmes_add2,  gensym("add2"), A_GIMME, 0);
  class_addmethod(phones2holmes_class, (t_method)phones2holmes_add2,  &s_list,        A_GIMME, 0);

  /* -- arbitrary parameter-setting method */
  //class_addanything(phones2holmes_class, phones2holmes_anything);

  class_sethelpsymbol(phones2holmes_class, gensym("phones2holmes-help.pd"));
}
