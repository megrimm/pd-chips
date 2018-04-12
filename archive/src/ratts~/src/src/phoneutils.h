/*
 * $Id: phoneutils.h,v 1.1 2003/01/02 00:45:28 moocow Exp $
 *
 * Based on code from 'rsynth', which is:
 * 
 *   Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: file created by moocow
 *   + shared phonetic utilities
 *
 */
#ifndef _RATTS_PHONEUTILS_H
#define _RATTS_PHONEUTILS_H

#include "darray.h"

/// Append (ch) to the darray (*p) as a literal : returns (ch)
extern int phone_append PROTO((darray_ptr p,int ch));

/// Append (s) to the darray (*p) as a literal : returns number of characters appended
extern unsigned phone_append_string PROTO((darray_ptr p, char *s));

#endif /* _RATTS_PHONEUTILS_H */
