/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: rattstok.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: raw text tokenizer
 *
 *     A PD external derived from Nick Ing-Simmons' "rsynth"
 *     program.
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

#include <ctype.h>
#include <stdlib.h>
#include "suspect.h"

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*=====================================================================
 * Globals
 *=====================================================================*/

// buffer-size to use for atom_string() calls for unknown pd atom-types
#define RATTSTOK_BUFSIZE 64

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define RATTSTOK_DEBUG 1
//#undef RATTSTOK_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/


/*=====================================================================
 * rattstok_class
 *=====================================================================*/
static t_class *rattstok_class;
typedef struct _rattstok
{
  t_object x_obj;              // black magic (probably inheritance-related)
  t_outlet *w_out;             // word outlet
  t_outlet *f_out;             // number outlet
  t_outlet *s_out;             // suspect word-outlet
  t_outlet *p_out;             // punctuation outlet
} t_rattstok;


/*=====================================================================
 * Utilities
 *=====================================================================*/
static void rattstok_tokenize(t_rattstok *x, char *s) {
  char ch;
  char *word;
  while (isspace(ch = *s)) s++; // skip leading spaces
  while ((ch = *s)) {
    word = s;
    if (isalpha(ch)) {
      while (isalpha(ch = *s) || ((ch == '\'' || ch == '-' || ch == '.') && isalpha(s[1])))
	s++;
      if (!ch || isspace(ch) || ispunct(ch) || (isdigit(ch) && !suspect_word(word, s-word)))
	{
	  if (ch) *s = '\0';
	  if (suspect_word(word, s-word)) {
	    // -- weird-looking little bugger
	    outlet_symbol(x->s_out, gensym(word));
	  } else {
	    // -- looks like a bona-fide lexeme...
	    outlet_symbol(x->w_out, gensym(word));
	  }
	  if (ch) *s = ch;
	}
      else
	{
	  // -- highly suspect... migh be an acronym
	  *s = '\0';
	  outlet_symbol(x->s_out, gensym(word));
	  *s = ch;
	}
    }
    else if (isdigit(ch)
	     || (ch == '-' && (isdigit(s[1]) || (s[1] == '.' && isdigit(s[2]))))
	     || (ch == '.' && isdigit(s[1])))
      {
	s++;
	while (isdigit(ch = *s) || (ch == '.' && isdigit(s[1])))
	  s++;
	*s = '\0';
	outlet_float(x->f_out, atof(word));
	*s = ch;
      }
    else if (ispunct(ch))
      {
	// -- punctuation
	ch = s[1];
	s[1] = '\0';
	outlet_symbol(x->p_out, gensym(s));
	s++;
	*s = ch;
      }
    else
      {
	// -- last resort: spell-out
	while ((ch = *s) && !isspace(ch)) s++;
	*s = '\0';
	outlet_symbol(x->s_out, gensym(word));
	*s = ch;
      }
    // -- trim trailing spaces
    while (isspace(ch = *s))
      s++;
  }
}

/*=====================================================================
 * pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * symbol
 */
void rattstok_symbol(t_rattstok *x, t_symbol *s) {
  rattstok_tokenize(x, s->s_name);
}

/*--------------------------------------------------------------------
 * float
 */
void rattstok_float(t_rattstok *x, t_floatarg f) {
  outlet_float(x->f_out, f);
}

/*--------------------------------------------------------------------
 * list
 */
void rattstok_list(t_rattstok *x, t_symbol *sel, int argc, t_atom *argv)
{
  while (argc-- > 0) {
    switch (argv->a_type)
      {
      case A_SYMBOL:
	rattstok_tokenize(x, argv->a_w.w_symbol->s_name);
	break;
      case A_FLOAT:
	outlet_float(x->f_out, argv->a_w.w_float);
	break;
      default:
	{
	  char buf[RATTSTOK_BUFSIZE];
	  atom_string(argv, buf, RATTSTOK_BUFSIZE);
	  rattstok_tokenize(x, buf);
	  break;
	}
      }
    argv++;
  }
}

/*--------------------------------------------------------------------
 * anything
 */
void rattstok_anything(t_rattstok *x, t_symbol *sel, int argc, t_atom *argv)
{
  rattstok_tokenize(x, sel->s_name);
  rattstok_list(x, &s_list, argc, argv);
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *rattstok_new(void)
{
  t_rattstok *x;
  x = (t_rattstok *)pd_new(rattstok_class);

  /* -- outlets -- */
  x->w_out = outlet_new(&x->x_obj, &s_symbol);
  x->f_out = outlet_new(&x->x_obj, &s_float);
  x->s_out = outlet_new(&x->x_obj, &s_symbol);
  x->p_out = outlet_new(&x->x_obj, &s_symbol);

  return (void *)x;
}

void rattstok_free(t_rattstok *x) {
  /* free rattstok structures */
  ;
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void rattstok_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:       rattstok : raw text tokenizer");
#endif

  /* register class */
  rattstok_class = class_new(gensym("rattstok"),              // name 
			     (t_newmethod)rattstok_new,       // newmethod
			     (t_method)rattstok_free,         // freemethod
			     sizeof(t_rattstok),              // size
			     CLASS_DEFAULT,                   // flags
			     0);                              // args
  /* --- methods --- */
  class_addsymbol(rattstok_class, rattstok_symbol);
  class_addlist(rattstok_class, rattstok_list);
  class_addfloat(rattstok_class, rattstok_float);
  class_addanything(rattstok_class, rattstok_anything);

  class_sethelpsymbol(rattstok_class, gensym("rattstok-help.pd"));
}
