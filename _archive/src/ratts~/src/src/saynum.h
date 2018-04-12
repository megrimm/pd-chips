/* 
 * $Id: saynum.h,v 1.1 2003/01/02 00:45:28 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: modified by moocow
 *   - removed numeric translators to their own file: 'saynum.h'
 *   - removed xlate_string() calls from numeric translators
 *     xlate_ordinal() and xlate_cardinal(): these now produce
 *     plain text for further processing.
 *
 */

#ifndef _RATTS_SAYNUM_H
#define _RATTS_SAYNUM_H

/// Pronounce a floating-point number.
extern unsigned xlate_float PROTO((float value, darray_ptr text));

/// Pronounce a floating-point number (low-level).
extern unsigned xlate_float_string PROTO((char *value, darray_ptr text));


/// Pronounce an ordinal number.
extern unsigned xlate_ordinal PROTO((long int value, darray_ptr text));

/// Pronounce a cardinal number.
extern unsigned xlate_cardinal PROTO((long int value, darray_ptr text));


#endif /* _RATTS_SAYNUM_H */
