/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pd_phtoelm.c
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

#include "elements.h"
#include "rholmes.h"
#include "phtoelm.h"
#include "pd_phtoelm.h"

/*---------------------------------------------------------------------
 * Globals: element-name lookup table
 *---------------------------------------------------------------------*/
static char pd_phtoelm_initialized = 0;

/*---------------------------------------------------------------------
 * Functions: initialization
 *---------------------------------------------------------------------*/

// fill symbol table and pre-generate parameter names
void pd_phtoelm_setup(void) {
  if (pd_phtoelm_initialized) return;

  // -- initialize phone-to-element lookup table
  phtoelm_enter_phonemes();

  pd_phtoelm_initialized = 1;
}


/*---------------------------------------------------------------------
 * Convert & append a pd atom-list of phonetic strings to a Holmes
 * element queue.
 */
extern dsqueue_t *atom_phones_to_eltq(const char *methodName,
				      int argc, t_atom *argv,
				      phtoelm_state_t *ps, dsqueue_t *eltq)
{
  if (!eltq) {
    eltq = dsqueue_new(argc);
    if (!eltq) {
      error("%s: queue allocation failed!",
	    methodName ? methodName : "atom_phones_to_eltq()");
      return NULL;
    }
  }
  while (argc-- > 0) {
    // -- lookup & assign element
    if (argv->a_type == A_SYMBOL) {
      phone_to_elm(ps, argv->a_w.w_symbol->s_name, eltq);
    } else {
      char buf[128];
      atom_string(argv,buf,128);
      phone_to_elm(ps, buf, eltq);
    }
    // -- atom borders are interpreted as inter-word silence
    dsqueue_append(eltq,(void *)hqeNew(1,6,0));
    argv++;
  }
  return eltq;
}
