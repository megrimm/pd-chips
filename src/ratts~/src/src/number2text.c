/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: number2text.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: phoneme-to-klatt-frame conversion
 *
 *     A PD external derived from Nick Ing-Simmons'
 *     'saynum.c', which produces English text from numeric values.
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

#include <stdio.h>   // for sprintf()
#include <stdlib.h>  // for malloc() and free()

#include "darray.h"
#include "phoneutils.h"
#include "saynum.h"

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
#define NUMBER2TEXT_DA_GSIZE 32

// buffer-length for float-to-char* conversion
#define NUMBER2TEXT_BUFLEN 32


/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define HOLMES_DEBUG 1
//#undef HOLMES_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/

/*=====================================================================
 * number2text_class
 *=====================================================================*/
static t_class *number2text_class;
typedef struct _number2text
{
  t_object          x_obj;                 // black magic (probably inheritance-related)
  darray_t          text;                  // text-string to output
  char              fbuf[NUMBER2TEXT_BUFLEN]; // buffer for sprintf() conversion
  t_outlet         *txt_out;               // symbol-outlet for current text-string
} t_number2text;


/*=====================================================================
 * holmes2phones: pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *number2text_new(t_symbol *sel)
{
  t_number2text *x;

  x = (t_number2text *)pd_new(number2text_class);

  /* initialize darray */
  darray_init(&x->text, sizeof(char), NUMBER2TEXT_DA_GSIZE);

  /* outlet(s) */
  x->txt_out = outlet_new(&x->x_obj, &s_symbol);

  return (void *)x;
}

void number2text_free(t_number2text *x) {
  /* free Holmes structures */
  darray_free(&x->text);
}


/*--------------------------------------------------------------------
 * FLOAT : guess phones for a float
 */
void number2text_float(t_number2text *x, t_floatarg f) {
  long l = (long)f;
  char *b, *e;
  if ((float)l == (float)f) {
    // -- do integer conversion
    xlate_cardinal(l, &x->text);
  } else {
    // -- do floating-point conversion
    sprintf(x->fbuf, "%g", (float)f);
    xlate_float_string(x->fbuf, &x->text);
  }
  // -- output the text
  phone_append(&x->text, 0);
  for (b = e = x->text.data; *e; e++) {
    if (*e == ' ' || !*e) {
      *e = 0;
      outlet_symbol(x->txt_out, gensym(b));
      e++;
      b = e;
    }
  }
  if (b != e) outlet_symbol(x->txt_out, gensym(b));
  x->text.items = 0;
}


/*--------------------------------------------------------------------
 * number2text: setup
 *--------------------------------------------------------------------*/
void number2text_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:    number2text : number to english text converter");
#endif

  /* register class */
  number2text_class = class_new(gensym("number2text"),              // name 
				(t_newmethod)number2text_new,       // newmethod
				(t_method)number2text_free,         // freemethod
				sizeof(t_number2text),              // size
				CLASS_DEFAULT,                      // flags
				//A_GIMME,                          // args
				0);

  /* --- methods --- */
  class_addfloat(number2text_class,    number2text_float);

  class_sethelpsymbol(number2text_class, gensym("number2text-help.pd"));
}
