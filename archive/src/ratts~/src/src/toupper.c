/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: toupper.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD: string-converters
 *
 *     PD wrappers for the C library toupper() and tolower() calls.
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

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*=====================================================================
 * Globals
 *=====================================================================*/

// intial conversion-buffer size to use for non-destructive string conversions
#define TOUPPER_INITIAL_BUFLEN 64

// extra bytes to get on dynamic resize for string conversion
#define TOUPPER_GETLEN 32

// intial output-buffer size (number of atoms)
#define TOUPPER_INITIAL_ABUFLEN 16

// number of extra atoms to get on dynamic output-buffer resize
#define TOUPPER_AGETLEN 8

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define TOUPPER_DEBUG 1
//#undef TOUPPER_DEBUG


/*=====================================================================
 * Structures and Types
 *=====================================================================*/


/*=====================================================================
 * toupper_class
 *=====================================================================*/
static t_class *toupper_class;
typedef struct _toupper
{
  t_object   x_obj;      // black magic (probably inheritance-related)
  char      *buf;        // string-conversion buffer
  t_int      buflen;     // allocated size of string-conversion buffer
  t_atom    *abuf;       // output buffer
  t_int      abuflen;    // allocated size of t_atom output buffer
  t_outlet  *x_out;      // upper-cased outlet
} t_toupper;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * toupper_string(x,s)
 *   + put upper-cased equivalent of 's' into x->buf, reallocating if
 *     necessary.
 */
static char *toupper_string(t_toupper *x, char *s)
{
  char *b;
  size_t len = strlen(s);

  // -- re-allocate if neccessary
  if (len > x->buflen) {
    x->buf = (char *)resizebytes(x->buf, x->buflen, len + TOUPPER_GETLEN);
    if (!x->buf) {
      x->buflen = 0;
      post("toupper: error allocating memory for string '%s'", s);
      return NULL;
    }
    x->buflen = len + TOUPPER_GETLEN;
  }

  b = x->buf;
  while (*s) {
    *b++ = toupper(*s++);
  }
  *b = '\0';
  return x->buf;
}

/*--------------------------------------------------------------------
 * toupper_atoms(x,argc,argv)
 *   + put upper-cased equivalents of first (argc) atoms in vector
 *     (argv) into x->abuf.
 *   + returns x->abuf
 */
static t_atom *toupper_atoms(t_toupper *x, int argc, t_atom *argv)
{
  t_atom *a;

  // -- reallocate output-buffer if necessary
  if (argc > x->abuflen) {
    x->abuf = (t_atom *)resizebytes(x->abuf,
				    x->abuflen*sizeof(t_atom),
				    (TOUPPER_AGETLEN+argc)*sizeof(t_atom));
    if (!x->abuf) {
      x->abuflen = 0;
      post("toupper: error allocating memory for output-buffer");
      return NULL;
    }
    x->abuflen = (TOUPPER_AGETLEN+argc)*sizeof(t_atom);
  }

  a = x->abuf;
  while (argc-- > 0) {
    switch (argv->a_type)
      {
      case A_SYMBOL:
	SETSYMBOL(a, gensym(toupper_string(x, argv->a_w.w_symbol->s_name)));
	break;
      case A_FLOAT:
	SETFLOAT(a, argv->a_w.w_float);
	break;
      default:
	{
	  post("toupper: unknown data type!");
	  *a = *argv;  // -- hope this works...
	  /*
	    char *b = x->buf;
	    atom_string(argv, x->buf, x->buflen);
	    while (*b) *b++ = toupper(*b);
	    break;
	  */
	}
      }
    a++;
    argv++;
  }
  return x->abuf;
}


/*=====================================================================
 * pd methods
 *=====================================================================*/


/*--------------------------------------------------------------------
 * symbol
 */
void toupper_symbol(t_toupper *x, t_symbol *sym) {
  outlet_symbol(x->x_out, gensym(toupper_string(x,sym->s_name)));
}

/*--------------------------------------------------------------------
 * float
 */
void toupper_float(t_toupper *x, t_floatarg f) {
  outlet_float(x->x_out, f);
}

/*--------------------------------------------------------------------
 * list
 */
void toupper_list(t_toupper *x, t_symbol *sel, int argc, t_atom *argv)
{
  outlet_list(x->x_out, sel, argc, toupper_atoms(x,argc,argv));
}

/*--------------------------------------------------------------------
 * anything
 */
void toupper_anything(t_toupper *x, t_symbol *sel, int argc, t_atom *argv)
{
  outlet_anything(x->x_out,
		  gensym(toupper_string(x, sel->s_name)),
		  argc,
		  toupper_atoms(x,argc,argv));
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *toupper_new(void)
{
  t_toupper *x;
  x = (t_toupper *)pd_new(toupper_class);

  /* -- conversion-buffer -- */
  x->buf = (char *)getbytes(TOUPPER_INITIAL_BUFLEN);
  if (x->buf) {
    x->buflen = TOUPPER_INITIAL_BUFLEN;
  } else {
    x->buflen = 0;
  }

  /* -- output-buffer -- */
  x->abuf = (t_atom *)getbytes(TOUPPER_INITIAL_ABUFLEN*sizeof(t_atom));
  if (x->abuf) {
    x->abuflen = TOUPPER_INITIAL_ABUFLEN;
  } else {
    x->abuflen = 0;
  }

  /* -- outlets -- */
  x->x_out = outlet_new(&x->x_obj, &s_anything);

  return (void *)x;
}

void toupper_free(t_toupper *x) {
  /* free buffers */
  if (x->buf && x->buflen > 0) {
    freebytes(x->buf,x->buflen);
  }
  if (x->abuf && x->abuflen > 0) {
    freebytes(x->abuf, x->abuflen*sizeof(t_atom));
  }
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
static void _toupper_setup(void) {
  /* register class */
  toupper_class = class_new(gensym("toupper"),              // name 
			    (t_newmethod)toupper_new,       // newmethod
			    (t_method)toupper_free,         // freemethod
			    sizeof(t_toupper),              // size
			    CLASS_DEFAULT,                  // flags
			    0);                             // args
  /* --- methods --- */
  class_addsymbol(toupper_class, toupper_symbol);
  class_addlist(toupper_class, toupper_list);
  class_addfloat(toupper_class, toupper_float);
  class_addanything(toupper_class, toupper_anything);

  class_sethelpsymbol(toupper_class, gensym("toupper-help.pd"));
}


/*=====================================================================
 * tolower_class
 *=====================================================================*/
static t_class *tolower_class;
typedef t_toupper t_tolower;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * tolower_string(x,s)
 *   + put upper-cased equivalent of 's' into x->buf, reallocating if
 *     necessary.
 */
static char *tolower_string(t_tolower *x, char *s)
{
  char *b;
  size_t len = strlen(s);

  // -- re-allocate if neccessary
  if (len > x->buflen) {
    x->buf = (char *)resizebytes(x->buf, x->buflen, len + TOUPPER_GETLEN);
    if (!x->buf) {
      x->buflen = 0;
      post("tolower: error allocating memory for string '%s'", s);
      return NULL;
    }
    x->buflen = len + TOUPPER_GETLEN;
  }

  b = x->buf;
  while (*s) {
    *b++ = tolower(*s++);
  }
  *b = '\0';
  return x->buf;
}

/*--------------------------------------------------------------------
 * tolower_atoms(x,argc,argv)
 *   + put upper-cased equivalents of first (argc) atoms in vector
 *     (argv) into x->abuf.
 *   + returns x->abuf
 */
static t_atom *tolower_atoms(t_tolower *x, int argc, t_atom *argv)
{
  t_atom *a;

  // -- reallocate output-buffer if necessary
  if (argc > x->abuflen) {
    x->abuf = (t_atom *)resizebytes(x->abuf,
				    x->abuflen*sizeof(t_atom),
				    (TOUPPER_AGETLEN+argc)*sizeof(t_atom));
    if (!x->abuf) {
      x->abuflen = 0;
      post("tolower: error allocating memory for output-buffer");
      return NULL;
    }
    x->abuflen = (TOUPPER_AGETLEN+argc)*sizeof(t_atom);
  }

  a = x->abuf;
  while (argc-- > 0) {
    switch (argv->a_type)
      {
      case A_SYMBOL:
	SETSYMBOL(a, gensym(tolower_string(x, argv->a_w.w_symbol->s_name)));
	break;
      case A_FLOAT:
	SETFLOAT(a, argv->a_w.w_float);
	break;
      default:
	{
	  post("tolower: unknown data type!");
	  *a = *argv;  // -- hope this works...
	  /*
	    char *b = x->buf;
	    atom_string(argv, x->buf, x->buflen);
	    while (*b) *b++ = tolower(*b);
	    break;
	  */
	}
      }
    a++;
    argv++;
  }
  return x->abuf;
}



/*=====================================================================
 * pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * symbol
 */
void tolower_symbol(t_tolower *x, t_symbol *sym)
{
  outlet_symbol(x->x_out, gensym(tolower_string(x, sym->s_name)));
}

/*--------------------------------------------------------------------
 * list
 */
void tolower_list(t_tolower *x, t_symbol *sel, int argc, t_atom *argv)
{
  outlet_list(x->x_out, sel, argc, tolower_atoms(x, argc, argv));
}

/*--------------------------------------------------------------------
 * anything
 */
void tolower_anything(t_tolower *x, t_symbol *sel, int argc, t_atom *argv)
{
  outlet_anything(x->x_out,
		  gensym(tolower_string(x, sel->s_name)),
		  argc,
		  tolower_atoms(x,argc,argv));
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *tolower_new(void)
{
  t_tolower *x;
  x = (t_tolower *)pd_new(tolower_class);

  /* -- conversion buffer -- */
  x->buf = (char *)getbytes(TOUPPER_INITIAL_BUFLEN);
  if (x->buf) {
    x->buflen = TOUPPER_INITIAL_BUFLEN;
  } else {
    x->buflen = 0;
  }

  /* -- output-buffer -- */
  x->abuf = (t_atom *)getbytes(TOUPPER_INITIAL_ABUFLEN*sizeof(t_atom));
  if (x->abuf) {
    x->abuflen = TOUPPER_INITIAL_ABUFLEN;
  } else {
    x->abuflen = 0;
  }

  /* -- outlets -- */
  x->x_out = outlet_new(&x->x_obj, &s_anything);

  return (void *)x;
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
static void _tolower_setup(void) {
  /* register class */
  tolower_class = class_new(gensym("tolower"),              // name 
			    (t_newmethod)tolower_new,       // newmethod
			    (t_method)toupper_free,         // freemethod
			    sizeof(t_tolower),              // size
			    CLASS_DEFAULT,                  // flags
			    0);                             // args
  /* --- methods --- */
  class_addsymbol(tolower_class, tolower_symbol);
  class_addlist(tolower_class, tolower_list);
  class_addfloat(tolower_class, toupper_float);
  class_addanything(tolower_class, tolower_anything);

  class_sethelpsymbol(tolower_class, gensym("toupper-help.pd"));
}


/*--------------------------------------------------------------------
 * global setup
 *--------------------------------------------------------------------*/
void toupper_setup(void) {
#ifdef RATTS_SHAMELESS
  /* post a little banner */
  post("ratts:        toupper : upper-case symbol normalizer");
#endif

  _toupper_setup();
  _tolower_setup();
}
