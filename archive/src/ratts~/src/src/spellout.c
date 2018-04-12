/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: spellout.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: pronounce single letters
 *
 * Based on "rsynth-2.0" by Nick Ing-Simmons.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include "ASCII.h"

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*=====================================================================
 * Globals
 *=====================================================================*/


// for non-symbol translations
#define SPELLOUT_BUFLEN 512
static char spellout_buffer[SPELLOUT_BUFLEN];

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define SPELLOUT_DEBUG 1
//#undef SPELLOUT_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/


/*=====================================================================
 * spellout_class
 *=====================================================================*/
static t_class *spellout_class;
typedef struct _spellout
{
  t_object    x_obj;       // black magic (probably inheritance-related)
  char      **chrtbl;      // character-to-word translations
  int         chrtbl_size; // size of char2word[]
  t_outlet   *x_out;       // word-outlet
} t_spellout;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * spellout_string(x,s)
 *   + actually spell-out the string 's', outletting as we go
 */
static void spellout_string(t_spellout *x, char *s)
{
  char *chname;
  while (*s) {
    if (*s >= 0 && *s < x->chrtbl_size) {
      chname = x->chrtbl[(int)(*s)];
      if (*chname) outlet_symbol(x->x_out, gensym(chname));
      // -- silently ignore empty-string table entries
    } else {
      post("spellout: warning: unknown character '%c' (ASCII=%d)", *s, *s);
    }
    s++;
  }  
}

/*--------------------------------------------------------------------
 * spellout_atoms(x,argc,argv)
 */
static void spellout_atoms(t_spellout *x, int argc, t_atom *argv)
{
  while (argc-- > 0) {
    if (argv->a_type == A_SYMBOL) {
      spellout_string(x, argv->a_w.w_symbol->s_name);
    } else {
      atom_string(argv, spellout_buffer, SPELLOUT_BUFLEN);
      spellout_string(x, spellout_buffer);
    }
    argv++;
  }
}


/*=====================================================================
 * pd methods
 *=====================================================================*/


/*--------------------------------------------------------------------
 * symbol
 */
void spellout_symbol(t_spellout *x, t_symbol *sym) {
  spellout_string(x, sym->s_name);
}

/*--------------------------------------------------------------------
 * float
 */
void spellout_float(t_spellout *x, t_floatarg f) {
  sprintf(spellout_buffer, "%g", f);
  spellout_string(x, spellout_buffer);
}

/*--------------------------------------------------------------------
 * list
 */
void spellout_list(t_spellout *x, t_symbol *sel, int argc, t_atom *argv)
{
  spellout_atoms(x,argc,argv);
}

/*--------------------------------------------------------------------
 * anything
 */
void spellout_anything(t_spellout *x, t_symbol *sel, int argc, t_atom *argv)
{
  spellout_string(x,sel->s_name);
  spellout_atoms(x, argc, argv);
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *spellout_new(void)
{
  t_spellout *x;
  x = (t_spellout *)pd_new(spellout_class);

  /* -- default character-table */
  x->chrtbl = ASCII;
  x->chrtbl_size = ASCII_size;

  /* -- outlets -- */
  x->x_out = outlet_new(&x->x_obj, &s_anything);

  return (void *)x;
}

void spellout_free(t_spellout *x) {
  /* do nothing */
  return;
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void spellout_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:       spellout : symbol to single-character-name converter");
#endif


  /* register class */
  spellout_class = class_new(gensym("spellout"),           // name 
			    (t_newmethod)spellout_new,     // newmethod
			    (t_method)spellout_free,       // freemethod
			    sizeof(t_spellout),            // size
			    CLASS_DEFAULT,                 // flags
			    0);                            // args
  /* --- methods --- */
  class_addsymbol(spellout_class, spellout_symbol);
  class_addlist(spellout_class, spellout_list);
  class_addfloat(spellout_class, spellout_float);
  class_addanything(spellout_class, spellout_anything);

  class_sethelpsymbol(spellout_class, gensym("spellout-help.pd"));
}
