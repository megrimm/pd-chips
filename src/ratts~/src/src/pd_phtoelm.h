/*
 * File: pd_phtoelm.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD : phones-to-Holmes-element utilties
 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/

#ifndef _PD_PHTOELM_H
#define _PD_PHTOELM_H

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#include "rholmes.h"
#include "phtoelm.h"
#include "dsqueue.h"

/*---------------------------------------------------------------------
 * Globals
 */

/*---------------------------------------------------------------------
 * Functions: initialization
 */

// fill in symbol_to_holmes_elt table and pre-generate parameter names
extern void pd_phtoelm_setup(void);

/*---------------------------------------------------------------------
 * Convert & append a pd atom-list of phones to a dsqueue of (char *)s
 *  + strings for the queue-elements are newly allocated with malloc()
 */
extern dsqueue_t *atom_phones_to_eltq(const char *methodName,
				      int argc, t_atom *argv,
				      phtoelm_state_t *ps, dsqueue_t *eltq);


#endif /* _PD_HOLMES_H */
