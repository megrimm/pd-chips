/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: guessphones.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: phoneme-to-klatt-frame conversion
 *
 *     A PD external derived from Nick Ing-Simmons'
 *     implementation of US Naval Research Laboratory rules
 *     for converting english text to phonemes, based on
 *     the version on the comp.speech archives.
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

#include "darray.h"
#include "phoneutils.h"
#include "text.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*=====================================================================
 * Globals
 *=====================================================================*/

// grow-by size for phones array (chars)
#define GUESSPHONES_DA_GSIZE 32

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define HOLMES_DEBUG 1
//#undef HOLMES_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/

/*=====================================================================
 * guessphones_class
 *=====================================================================*/
static t_class *guessphones_class;
typedef struct _guessphones
{
  t_object          x_obj;            // black magic (probably inheritance-related)
  darray_t          phones;           // output phone-string
  t_outlet         *pho_out;          // symbol-outlet for current phone-string
} t_guessphones;


/*=====================================================================
 * holmes2phones: pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *guessphones_new(t_symbol *sel)
{
  t_guessphones *x;

  x = (t_guessphones *)pd_new(guessphones_class);

  /* initialize darray */
  darray_init(&x->phones, sizeof(char), GUESSPHONES_DA_GSIZE);

  /* outlet(s) */
  x->pho_out = outlet_new(&x->x_obj, &s_symbol);

  return (void *)x;
}

void guessphones_free(t_guessphones *x) {
  /* free Holmes structures */
  darray_free(&x->phones);
}


/*--------------------------------------------------------------------
 * SYMBOL ... : guess phones for a SYMBOL
 */
void guessphones_symbol(t_guessphones *x, t_symbol *sym) {
  char *s = sym->s_name;
  NRL(s, strlen(s), &x->phones);
  phone_append(&x->phones, 0);
  outlet_symbol(x->pho_out, gensym(x->phones.data));
  x->phones.items = 0;
}

/*--------------------------------------------------------------------
 * guessphones_atom ATOM : guess phones for an atom
 */
void guessphones_atom(t_guessphones *x, t_atom *a) {
  switch (a->a_type) {
  case A_SYMBOL:
    {
      char *s = a->a_w.w_symbol->s_name;
      NRL(s, strlen(s), &x->phones);
      break;
    }
  default:
    {
      char buf[128];
      atom_string(a, buf, 128);
      NRL(buf, strlen(buf), &x->phones);
    }
  }
  // -- output the phones
  phone_append(&x->phones, 0);
  outlet_symbol(x->pho_out, gensym(x->phones.data));
  x->phones.items = 0;
}


/*--------------------------------------------------------------------
 * WORDS ... : guess phones for WORDS
 */
void guessphones_anything(t_guessphones *x, t_symbol *sel, int argc, t_atom *argv) {
  guessphones_symbol(x,sel);
  while (argc-- > 0) {
    guessphones_atom(x,argv++);
  }
}


/*--------------------------------------------------------------------
 * list WORDS ... : guess phones for WORDS
 */
void guessphones_list(t_guessphones *x, t_symbol *sel, int argc, t_atom *argv) {
  while (argc-- > 0) {
    guessphones_atom(x,argv++);
  }
}



/*--------------------------------------------------------------------
 * guessphones: setup
 *--------------------------------------------------------------------*/
void guessphones_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:    guessphones : NRL english to phone-string converter");
#endif

  /* register class */
  guessphones_class = class_new(gensym("guessphones"),              // name 
				(t_newmethod)guessphones_new,       // newmethod
				(t_method)guessphones_free,         // freemethod
				sizeof(t_guessphones),              // size
				CLASS_DEFAULT,                      // flags
				//A_GIMME,                          // args
				0);

  /* --- methods --- */
  class_addlist(guessphones_class,     guessphones_list);
  class_addsymbol(guessphones_class,   guessphones_symbol);
  class_addanything(guessphones_class, guessphones_anything);

  class_sethelpsymbol(guessphones_class, gensym("guessphones-help.pd"));
}
