/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: klatt_frame.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD : Klatt synthesier frame utilities for PD
 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/

#ifndef _KLATT_FRAME_H
#define _KLATT_FRAME_H

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#include "proto.h"
#include "parwave.h"

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define KLATT_DEBUG 1
//#undef KLATT_DEBUG

/*---------------------------------------------------------------------
 * Functions: initialization
 */

// pre-generate symbol names
extern void klatt_frame_setup(void);

/*---------------------------------------------------------------------
 * Functions: frame-constructor
 */
extern klatt_frame_t *klatt_frame_new(const char *methodName);


/*---------------------------------------------------------------------
 * Functions: frame-assignment
 */

// set a klatt_frame_t's contents from keyword parameters
extern void klatt_frame_set_kw(const char *methodName,
			       klatt_frame_t *frame,
			       t_symbol *sel,
			       int argc,
			       t_atom *argv);


// set a klatt_frame_t's contents from a list of floats
//   + 'frame' is really a 'klatt_frame_t *'
void klatt_frame_set_l(const char *methodName, long *frame, int argc, t_atom *argv);

/*---------------------------------------------------------------------
 * Functions: klatt-frame to pd-list
 */

/// Initialize a Klatt-frame atom-list.
/// The atom list should already be allocated!
void init_klatt_frame_alist(t_atom *argv);

/// Set a pd atom list to the contents of a Klatt-frame.
/// The atom list should already be allocated!
void klatt_frame_to_alist(long *frame, t_atom *argv);

#endif /* KLATT_FRAME_H */
