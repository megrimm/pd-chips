/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pd_holmes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD : Holmes phoneme-to-Klatt utilties
 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/

#include <stdlib.h>

#include "alhash.h"
#include "klatt_frame.h"
#include "rholmes.h"
#include "pd_holmes.h"
#include "elements.h"
#include "phtoelm.h"
#include "phfeat.h"


/*---------------------------------------------------------------------
 * Globals
 *---------------------------------------------------------------------*/

//-- element-name lookup table
alhash_table_t *symbol_to_holmes_eid = NULL;

//-- feature-name lookup table (values = long int *) : UNUSED (?)
alhash_table_t *symbol_to_holmes_ftrp = NULL;

//-- pre-computed symbols
static t_symbol *speed, *frac, *topc, *basec, *stress_st, *stress_et, *f0decl;

//-- initialization flag
static char pd_holmes_initialized = 0;

/*---------------------------------------------------------------------
 * Functions: initialization
 *---------------------------------------------------------------------*/

// fill symbol table and pre-generate parameter names
void pd_holmes_setup(void) {
  int i;
  if (pd_holmes_initialized) return;

  // -- fill Holmes element-symbol lookup table
  symbol_to_holmes_eid = alhash_new(0,alhash_direct_hash,alhash_direct_equal);
  for (i = 0; i < num_Elements; i++) {
    alhash_insert(symbol_to_holmes_eid,
		  gensym(Elements[i].name),
		  (void *)i
		  );
  }

  // -- fill Holmes feature-symbol lookup table
  symbol_to_holmes_ftrp = alhash_new(0,alhash_direct_hash,alhash_direct_equal);
  for (i = 0; i < num_Features; i++) {
    alhash_insert(symbol_to_holmes_ftrp, gensym(FeatureNames[i]), Features+i);
  }

  // -- pre-generate holmes parameter symbols
  speed = gensym("speed");
  frac = gensym("frac");
  topc = gensym("topc");
  basec = gensym("basec");
  stress_st = gensym("stress_st");
  stress_et = gensym("stress_st");
  f0decl = gensym("f0decl");

  pd_holmes_initialized = 1;
}

/*---------------------------------------------------------------------
 * Set Holmes parameters from a pd keyword list
 */
void pd_holmes_set_kw(const char *methodName,
		      holmes_global_t *hg,    holmes_state_t *hs,
		      t_symbol *sel,          int argc,           t_atom *argv)
{
# ifdef HOLMES_DEBUG
  post("pd_holmes_set_kw(): called with %d arguments", argc);
# endif

  for ( ; sel && argc >= 0; argc -= 2) {
# ifdef HOLMES_DEBUG
    char symbuf[256];
    atom_string(argv, symbuf, 256);
    post("pd_holmes_set_kw(): iter argc=%d ; sel=%s ; argv='%s'",
	 argc, sel ? sel->s_name : "NULL", symbuf);
# endif

    if (sel == f0decl) {
      // Declination of f0 envelope in Hz/cS (default=0.5)
      hg->f0decl = atom_getfloat(argv);
    }
    else if (sel == topc) {
      // frequency coefficient (for f0) for per-utterance prosodic peak (default=1.1)
      hg->topc = atom_getfloat(argv);
      if (hg->topc <= 0) { hg->topc = 1.0; }
#    ifdef HOLMES_DEBUG
      post("pd_holmes_set_kw(): set hg->topc to %f", hg->topc);
#    endif
    }
    else if (sel == basec) {
      // frequency coefficient (for f0) for per-utterance prosodic base (default=0.8)
      hg->basec = atom_getfloat(argv);
      if (hg->basec <= 0) { hg->basec = 1.0; }
    }
    else if (sel == frac) {
      // Parameter filter 'fraction' (default=1.0) : range = 0..~2.0
      hg->frac = atom_getfloat(argv);
      if (hg->frac < 0) hg->frac = 0.0;
      if (hg->frac >= 2) hg->frac = 2.0;
    }
    else if (sel == stress_st) {
      // Default transition time for stress_s (default=40)
      hg->stress_st = atom_getfloat(argv);
    }
    else if (sel == stress_et) {
      // Default transition time for stress_e (default=40);
      hg->stress_et = atom_getfloat(argv);
    }
    else if (sel == speed) {
      // Speed coefficient, 1.0 is "normal"
      hg->speed = atom_getfloat(argv);
    }
    else {
      // -- unknown: assumed to be a (default) klatt_frame parameter
      klatt_frame_set_kw(methodName ? methodName : "pd_holmes_set_kw()",
			 &(hg->def_pars),
			 sel, argc ? 1 : 0, argv);
    }
    // -- update counters
    sel = argc > 1 ? atom_getsymbol(++argv) : NULL;
    ++argv;
  }
}


/*---------------------------------------------------------------------
 * Convert & append a pd atom-list to a dsqueue of (holmes_elt_t)s
 */
dsqueue_t *atoms_to_holmes_eltq(const char *methodName, int argc, t_atom *argv, dsqueue_t *eltq) {
  if (!eltq) {
    eltq = dsqueue_new(argc);
    if (!eltq) {
      error("%s: queue allocation failed!",
	    methodName ? methodName : "atoms_to_holmes_eltq()");
      return NULL;
    }
  }
  while (argc-- > 0) {
    holmes_qelt_t qe = 1;

    // -- lookup & assign element
    switch (argv->a_type) {
    case A_SYMBOL:
      {
	alhash_entry_t *he = NULL;
	if ((he = alhash_lookup_extended(symbol_to_holmes_eid, argv->a_w.w_symbol)))
	  {
	    qe = (unsigned)he->val;
	  }
	else
	  {
	    // -- uh-oh
	    error("%s: unknown element name '%s' -- using 'Q'",
		  methodName ? methodName : "atoms_to_holmes_eltq()",
		  argv->a_w.w_symbol->s_name);
	  }
	break;
      }
    default:
      // -- anything other than a symbol is assumed to be a literal element encoding
      qe = (unsigned)atom_getint(argv);
      if (hqeGetEID(qe) >= num_Elements) {
	error("%s: (encoded) no known element at index %d -- using 'Q'",
	      methodName ? methodName : "atoms_to_holmes_eltq()", hqeGetEID(qe));
	qe = 1;
      }
    }
    argv++;

    // -- element index part of 'qe' should already have been set

    // -- set duration
    if (argc-- > 0) {
      hqeSetDur(qe, atom_getint(argv++));
    } else {
      Elm_t *e = &Elements[hqeGetEID(qe)];
      hqeSetDur(qe, e->du);
    }

    // -- set stress
    hqeSetStr(qe, (argc-- > 0 ? atom_getint(argv++) : 0));

    // -- append to the queue
    dsqueue_append(eltq, (void *)qe);
  }
  return eltq;
}
