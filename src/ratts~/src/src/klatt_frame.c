/*
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */

#include "klatt_frame.h"
#include <stdlib.h>


/*---------------------------------------------------------------------
 * Globals: pre-generated parameter-name symbols
 *---------------------------------------------------------------------*/
static t_symbol
  *f0,*av,*f1,*b1,*f2,*b2,*f3,*b3,*f4,*b4,*f5,*b5,*f6,*b6,*fnz,*bnz,*fnp,*bnp,
  *asp,*kopen,*aturb,*tilt,*af,*skew,*a1,*b1p,*a2,*b2p,*a3,*b3p,*a4,*b4p,
  *a5,*b5p,*a6,*b6p,*anp,*ab,*avp,*gain;

static char klatt_frame_initialized = 0;


/*---------------------------------------------------------------------
 * Functions: initialization
 *---------------------------------------------------------------------*/

// klatt_frame_setup() : setup frame-utilities
void klatt_frame_setup(void) {
  if (klatt_frame_initialized) return;

  // pre-generate symbols
  f0 = gensym("f0");
  av = gensym("av");
  f1 = gensym("f1");
  b1 = gensym("b1");
  f2 = gensym("f2");
  b2 = gensym("b2");
  f3 = gensym("f3");
  b3 = gensym("b3");
  f4 = gensym("f4");
  b4 = gensym("b4");
  f5 = gensym("f5");
  b5 = gensym("b5");
  f6 = gensym("f6");
  b6 = gensym("b6");
  fnz = gensym("fnz");
  bnz = gensym("bnz");
  fnp = gensym("fnp");
  bnp = gensym("bnp");
  asp = gensym("asp");
  kopen = gensym("kopen");
  aturb = gensym("aturb");
  tilt = gensym("tilt");
  af = gensym("af");
  skew = gensym("skew");
  a1 = gensym("a1");
  b1p = gensym("b1p");
  a2 = gensym("a2");
  b2p = gensym("b2p");
  a3 = gensym("a3");
  b3p = gensym("b3p");
  a4 = gensym("a4");
  b4p = gensym("b4p");
  a5 = gensym("a5");
  b5p = gensym("b5p");
  a6 = gensym("a6");
  b6p = gensym("b6p");
  anp = gensym("anp");
  ab = gensym("ab");
  avp = gensym("avp");
  gain = gensym("gain");

  // set initialized flag
  klatt_frame_initialized = 1;
}

/*---------------------------------------------------------------------
 * Functions: frame-constructor
 *---------------------------------------------------------------------*/
klatt_frame_t *klatt_frame_new(const char *methodName) {
  klatt_frame_t *frame = (klatt_frame_t *)malloc(sizeof(klatt_frame_t));
  if (!frame) {
    error("klatt~: frame allocation failed in %s.", methodName ? methodName : "klatt_frame_new()");
  }
  return frame;
}

/*---------------------------------------------------------------------
 * Functions: frame-assignment
 *---------------------------------------------------------------------*/

// klatt_frame_set_kw() : set a klatt_frame_t's contents from keyword parameters
void klatt_frame_set_kw(const char *methodName,
			klatt_frame_t *frame,
			t_symbol *sel,
			int argc,
			t_atom *argv)
{
  int i;
  if (!frame) {
    error("%s: got NULL frame.", methodName ? methodName : "klatt_frame_set_kw()");
    return;
  }
  for (i = 0; i < argc; i += 2) {
    if (sel == f0) frame->F0hz10 = (long)atom_getfloat(argv);
    else if (sel == av) frame->AVdb = (long)atom_getfloat(argv);
    else if (sel == f1) frame->F1hz = (long)atom_getfloat(argv);
    else if (sel == b1) frame->B1hz = (long)atom_getfloat(argv);
    else if (sel == f2) frame->F2hz = (long)atom_getfloat(argv);
    else if (sel == b2) frame->B2hz = (long)atom_getfloat(argv);
    else if (sel == f3) frame->F3hz = (long)atom_getfloat(argv);
    else if (sel == b3) frame->B3hz = (long)atom_getfloat(argv);
    else if (sel == f4) frame->F4hz = (long)atom_getfloat(argv);
    else if (sel == b4) frame->B4hz = (long)atom_getfloat(argv);
    else if (sel == f5) frame->F5hz = (long)atom_getfloat(argv);
    else if (sel == b5) frame->B5hz = (long)atom_getfloat(argv);
    else if (sel == f6) frame->F6hz = (long)atom_getfloat(argv);
    else if (sel == b6) frame->B6hz = (long)atom_getfloat(argv);
    else if (sel == fnz) frame->FNZhz = (long)atom_getfloat(argv);
    else if (sel == bnz) frame->BNZhz = (long)atom_getfloat(argv);
    else if (sel == fnp) frame->FNPhz = (long)atom_getfloat(argv);
    else if (sel == bnp) frame->BNPhz = (long)atom_getfloat(argv);
    else if (sel == asp) frame->ASP = (long)atom_getfloat(argv);
    else if (sel == kopen) frame->Kopen = (long)atom_getfloat(argv);
    else if (sel == aturb) frame->Aturb = (long)atom_getfloat(argv);
    else if (sel == tilt) frame->TLTdb = (long)atom_getfloat(argv);
    else if (sel == af) frame->AF = (long)atom_getfloat(argv);
    else if (sel == skew) frame->Kskew = (long)atom_getfloat(argv);
    else if (sel == a1) frame->A1 = (long)atom_getfloat(argv);
    else if (sel == b1p) frame->B1phz = (long)atom_getfloat(argv);
    else if (sel == a2) frame->A2 = (long)atom_getfloat(argv);
    else if (sel == b2p) frame->B2phz = (long)atom_getfloat(argv);
    else if (sel == a3) frame->A3 = (long)atom_getfloat(argv);
    else if (sel == b3p) frame->B3phz = (long)atom_getfloat(argv);
    else if (sel == a4) frame->A4 = (long)atom_getfloat(argv);
    else if (sel == b4p) frame->B4phz = (long)atom_getfloat(argv);
    else if (sel == a5) frame->A5 = (long)atom_getfloat(argv);
    else if (sel == b5p) frame->B5phz = (long)atom_getfloat(argv);
    else if (sel == a6) frame->A6 = (long)atom_getfloat(argv);
    else if (sel == b6p) frame->B6phz = (long)atom_getfloat(argv);
    else if (sel == anp) frame->ANP = (long)atom_getfloat(argv);
    else if (sel == ab) frame->AB = (long)atom_getfloat(argv);
    else if (sel == avp) frame->AVpdb = (long)atom_getfloat(argv);
    else if (sel == gain) frame->Gain0 = (long)atom_getfloat(argv);
    else {
      char parsym[256];
      atom_string(argv,parsym,256);
      post("%s: unknown parameter name '%s' -- ignored.",
	   methodName ? methodName : "klatt_frame_set_kw()", parsym);
    }
    /* update pointers */
    sel = atom_getsymbol(++argv);
    ++argv;
  }
}

// klatt_frame_set_l(): set whole frames from a list of floats
//   + NOTE: the 'frame' argument is really a 'klatt_frame_t *'
void klatt_frame_set_l(const char *methodName, long *frame, int argc, t_atom *argv)
{
  int i;
  for (i = 0; i < argc; i++) {
    if (i >= NPAR) {
      post("%s: extra frame parameters dropped.",
	   methodName ? methodName : "pdklatt_set_frame_l()");
      break;
    }
    *frame++ = (long)atom_getfloat(argv);
    argv++;
  }
}

/*---------------------------------------------------------------------
 * Functions: klatt-frame to pd-list
 *---------------------------------------------------------------------*/

/// Initialize a Klatt-frame atom-list.
/// The atom list should already be allocated!
void init_klatt_frame_alist(t_atom *argv) {
  int i;
  for (i = 0; i < NPAR; i++) {
    SETFLOAT(argv,0);
    argv++;
  }
}


/// Set a pd atom list to the contents of a Klatt-frame.
/// The atom list should already be allocated!
void klatt_frame_to_alist(long *frame, t_atom *argv) {
  int i;
  for (i = 0; i < NPAR; i++) {
    argv++->a_w.w_float = (float)(*frame++);
  }
}
