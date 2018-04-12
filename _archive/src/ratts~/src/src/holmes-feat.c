/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: holmes-feat.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: phonetic feature lookup
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


/*=====================================================================
 * Structures and Types
 *=====================================================================*/

/// max size of feature list
#define NFEAT 29

/*=====================================================================
 * holmes_class
 *=====================================================================*/
static t_class *holmes_feat_class;
typedef struct _holmes_feat
{
  t_object  x_obj;             // black magic (probably inheritance-related)
  t_atom    x_mask[NFEAT];     // pd-friendly feature-mask data
  t_outlet *mask_out;          // list-outlet for feature-masks
} t_holmes_feat;


/*=====================================================================
 * pd methods
 *=====================================================================*/


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *holmes_feat_new(void)
{
  t_holmes_feat *x;
  int i;
  x = (t_holmes_feat *)pd_new(holmes_feat_class);

  /* intialize pd-passable frame-list */
  for (i = 0; i < NFEAT; i++) {
    SETFLOAT(&x->x_mask[i], 0);
  }

  /* -- outlets -- */
  x->mask_out = outlet_new(&x->x_obj, &s_list);

  return (void *)x;
}

void holmes_feat_free(t_holmes_feat *x)
{

}

/*--------------------------------------------------------------------
 * get_features(x,eid)
 *   + utility
 */
static void get_features(t_holmes_feat *x, unsigned eid)
{
  unsigned i;
  if (eid >= num_Elements) eid = 0;
  for (i = 0; i < NFEAT; i++) {
    x->x_mask[i].a_w.w_float = Elements[eid].feat & (long)(1<<i) ? 1 : 0;
  }
  outlet_list(x->mask_out, &s_list, NFEAT, x->x_mask);
}

/*--------------------------------------------------------------------
 * symbol : get features for given symbol
 */
void holmes_feat_symbol(t_holmes_feat *x, t_symbol *elt) {
  alhash_entry_t *he = alhash_lookup_extended(symbol_to_holmes_eid, elt);
  get_features(x, (unsigned)he->val);
}

/*--------------------------------------------------------------------
 * float : get features for element encoding
 */
void holmes_feat_float(t_holmes_feat *x, t_floatarg f) {
  get_features(x, hqeGetEID((unsigned)f));
}

/*--------------------------------------------------------------------
 * list : get features for first list element
 */
void holmes_feat_list(t_holmes_feat *x, t_symbol *sel, int argc, t_atom *argv)
{
  switch (argv->a_type) {
  case A_SYMBOL:
    holmes_feat_symbol(x, argv->a_w.w_symbol);
    break;
  case A_FLOAT:
    holmes_feat_float(x, argv->a_w.w_float);
    break;
  default:
    post("holmes_feat_list(): warning: unknown atom type!");
    get_features(x,0);
  }
}

/*--------------------------------------------------------------------
 * anything : get features for selector
 */
void holmes_feat_anything(t_holmes_feat *x, t_symbol *sel, int argc, t_atom *argv)
{
  holmes_feat_symbol(x,sel);
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void holmes_feat_setup(void) {
  /* ensure that our dependencies have been satisfied */
  pd_holmes_setup();

#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:    holmes-feat : Phonetic feature mask extractor");
#endif

  /* register class */
  holmes_feat_class = class_new(gensym("holmes-feat"),    // name 
			   (t_newmethod)holmes_feat_new,  // newmethod
			   (t_method)holmes_feat_free,    // freemethod
			   sizeof(t_holmes_feat),         // size
			   CLASS_DEFAULT,                 // flags
			   0);                            // args

  /* --- methods --- */
  class_addsymbol(holmes_feat_class, (t_method)holmes_feat_symbol);
  class_addfloat(holmes_feat_class, (t_method)holmes_feat_float);
  class_addlist(holmes_feat_class, (t_method)holmes_feat_list);
  class_addanything(holmes_feat_class, (t_method)holmes_feat_anything);

  class_sethelpsymbol(holmes_feat_class, gensym("holmes-feat-help.pd"));
}
