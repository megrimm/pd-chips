/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pd_holmes.h
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

#ifndef _PD_HOLMES_H
#define _PD_HOLMES_H

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#include "rholmes.h"
#include "phtoelm.h"

/*---------------------------------------------------------------------
 * Globals
 */

/*---------------------------------------------------------------------
 * Functions: initialization
 */

// fill in symbol_to_holmes_elt table and pre-generate parameter names
extern void pd_holmes_setup(void);

/*---------------------------------------------------------------------
 * Set Holmes parameters from a pd keyword list
 */
extern void pd_holmes_set_kw(const char *methodName,
			     holmes_global_t *hg,    holmes_state_t *hs,
			     t_symbol *sel,          int argc,           t_atom *argv);

/*---------------------------------------------------------------------
 * Convert & append a pd atom-list to a dsqueue of (holmes_qelt_t)s
 *   + atoms are either literal integer-encoded (holmes_qelt_t)s or
 *     triplets 'NAME DURATION STRESS'
 *   + currently unused
 */
extern dsqueue_t *atoms_to_holmes_eltq(const char *methodName,
				       int argc, t_atom *argv,
				       dsqueue_t *eltq);


#endif /* _PD_HOLMES_H */
