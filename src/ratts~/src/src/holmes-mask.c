/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: holmes-mask.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: holmes-element time quantization
 *
 *     A PD external derived from Nick Ing-Simmons'
 *     implementation of the phoneme to vocal tract parameters
 *     system described in:
 *
 *        J. N. Holmes, I. Mattingly, and J. Shearme, "Speech synthesis by rule",
 *        Language and Speech 7, pp 127--143, 1964.
 *
 *
 * Copyright (c) 2004 Bryan Jurish.  All Rights Reserved.
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

#include "proto.h"
#include "phfeat.h"
#include "elements.h"
#include "rholmes.h"
#include "pd_holmes.h"
#include "alhash.h"


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

extern alhash_table_t *symbol_to_holmes_eid;
extern alhash_table_t *symbol_to_holmes_ftrp;


/*=====================================================================
 * Structures and Types
 *=====================================================================*/

/*=====================================================================
 * holmes_mask_class
 *=====================================================================*/
static t_class *holmes_mask_class;
typedef struct _holmes_mask
{
  t_object  x_obj;             // black magic (probably inheritance-related)
  long      x_mask;            // mask of features to scale
  t_atom    x_buf[3];          // output buffer
  t_outlet *x_goodout;         // match outlet
  t_outlet *x_badout;          // no-match outlet
} t_holmes_mask;

/*=====================================================================
 * utility methods
 *=====================================================================*/

/*-------------------------------------------------------------
 * holmes_mask_name2ftr
 */
long holmes_mask_name2ftr(t_symbol *fname)
{
  alhash_entry_t *he;
  if ((he = alhash_lookup_extended(symbol_to_holmes_ftrp, fname))) {
    //post("--> got val=%ld for ftr='%s'", *((long *)(he->val)), fname->s_name);//--DEBUG
    return *((long *)(he->val));
  }
  error("holmes_mask: unknown feature name '%s'", fname->s_name);
  return 0;
}

/*-------------------------------------------------------------
 * set_bymask()
 */
static void holmes_mask_set_bymask(t_holmes_mask *x, long mask, int val)
{
  //post("--> x->x_mask called: mask=%ld, val=%d", mask, val); //-- DEBUG

  if (val) x->x_mask |= mask;
  else x->x_mask &= ~mask;
}

/*-------------------------------------------------------------
 * set_byname()
 */
static void holmes_mask_set_byname(t_holmes_mask *x, t_symbol *fs, int val)
{
  holmes_mask_set_bymask(x, holmes_mask_name2ftr(fs), val);
}

/*=====================================================================
 * pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * list : match features
 */
static void holmes_mask_list(t_holmes_mask *x, t_symbol *sel, int argc, t_atom *argv)
{
  int i;
  alhash_entry_t *he;
  t_outlet       *heout;
  for (i = 0; i < argc; i += 3) {
    //-- element
    SETSYMBOL(x->x_buf, atom_getsymbolarg(i, argc, argv));

    //-- get duration, stress
    SETFLOAT(x->x_buf+1, atom_getfloatarg(i+1, argc, argv));
    SETFLOAT(x->x_buf+2, atom_getfloatarg(i+2, argc, argv));

    //-- lookup
    he = alhash_lookup_extended(symbol_to_holmes_eid, x->x_buf->a_w.w_symbol);
    if (he) {
      if (x->x_mask & Elements[(unsigned int)(he->val)].feat) heout = x->x_goodout;
      else heout = x->x_badout;
    }
    else {
      heout = x->x_badout;
      error("holmes_mask: unknown holmes element '%s'", x->x_buf->a_w.w_symbol->s_name);
    }
    outlet_anything(heout, &s_list, 3, x->x_buf);
  }
}

/*-------------------------------------------------------------
 * anything() : match element-names only
 */
static void holmes_mask_anything(t_holmes_mask *x, t_symbol *sel, int argc, t_atom *argv)
{
  int i;
  alhash_entry_t *he;
  t_outlet       *heout;
  t_symbol       *es = sel;

  for (i = -1; i < argc; i++) {
    //-- element, lookup
    if (i >= 0) es = atom_getsymbolarg(i, argc, argv);
    he = alhash_lookup_extended(symbol_to_holmes_eid, es);

    if (he) {
      if (x->x_mask & Elements[(unsigned int)(he->val)].feat) heout = x->x_goodout;
      else heout = x->x_badout;
    }
    else {
      heout = x->x_badout;
      error("holmes_mask: unknown holmes element '%s'", es->s_name);
    }
    outlet_anything(heout, es, 0, NULL);
  }
}

/*-------------------------------------------------------------
 * symbol() : match a single element name
 */
static void holmes_mask_symbol(t_holmes_mask *x, t_symbol *sym)
{
  holmes_mask_anything(x, sym, 0, NULL);
}

/*-------------------------------------------------------------
 * values()
 */
static void holmes_mask_values(t_holmes_mask *x, t_symbol *sel, int argc, t_atom *argv)
{
  int i;
  for (i=0; i < argc; i += 2) {
    holmes_mask_set_byname(x,
			   atom_getsymbolarg(i, argc, argv),
			   atom_getintarg(i+1, argc, argv));
  }
}

/*-------------------------------------------------------------
 * clear()
 */
static void holmes_mask_clear(t_holmes_mask *x)
{
  x->x_mask = 0;
}

/*-------------------------------------------------------------
 * invert()
 */
static void holmes_mask_invert(t_holmes_mask *x)
{
  x->x_mask = ~(x->x_mask);
}

/*-------------------------------------------------------------
 * print()
 */
static void holmes_mask_print(t_holmes_mask *x)
{
  int i, j;
  for (i=0; i < 3; i++) {
    if (i == 0) startpost("holmes_mask:");
    else        startpost("            ");
    for (j=0; j < 10 && (i*10+j) < num_Features; j++) {
      startpost(" %s=%d",
		FeatureNames[i*10+j],
		((x->x_mask & Features[i*10+j]) ? 1 : 0));
    }
    endpost();
  }
  post("            = %ld", x->x_mask);
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *holmes_mask_new(t_symbol *sel, int argc, t_atom *argv)
{
  t_holmes_mask *x;
  x = (t_holmes_mask *)pd_new(holmes_mask_class);

  /* intialize expansion mask */
  x->x_mask = 0;

  /* -- outlets -- */
  x->x_goodout = outlet_new(&x->x_obj, &s_list);
  x->x_badout  = outlet_new(&x->x_obj, &s_list);

  /*-- initialization -- */
  if (argc) holmes_mask_values(x, &s_list, argc, argv);

  return (void *)x;
}

void holmes_mask_free(t_holmes_mask *x)
{
  //-- ??
  outlet_free(x->x_goodout);
  outlet_free(x->x_badout);
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void holmes_mask_setup(void) {
  /* ensure that our dependencies have been satisfied */
  pd_holmes_setup();

#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:    holmes-mask : Phonetic feature mask");
#endif

  /* register class */
  holmes_mask_class = class_new(gensym("holmes-mask"),    // name 
				(t_newmethod)holmes_mask_new,  // newmethod
				(t_method)holmes_mask_free,    // freemethod
				sizeof(t_holmes_mask),         // size
				CLASS_DEFAULT,                 // flags
				A_GIMME, 0);                   // args

  /* methods */
  class_addlist(holmes_mask_class, (t_method)holmes_mask_list);
  class_addanything(holmes_mask_class, (t_method)holmes_mask_anything);
  class_addsymbol(holmes_mask_class, (t_method)holmes_mask_symbol);

  class_addmethod(holmes_mask_class, (t_method)holmes_mask_values,
		  gensym("values"), A_GIMME, A_NULL);
  class_addmethod(holmes_mask_class, (t_method)holmes_mask_values,
		  gensym("value"), A_GIMME, A_NULL);

  class_addmethod(holmes_mask_class, (t_method)holmes_mask_clear,
		  gensym("clear"), A_NULL);
  class_addmethod(holmes_mask_class, (t_method)holmes_mask_invert,
		  gensym("invert"), A_NULL);
  class_addmethod(holmes_mask_class, (t_method)holmes_mask_print,
		  gensym("print"), A_NULL);

  class_sethelpsymbol(holmes_mask_class, gensym("holmes-mask-help.pd"));
}
