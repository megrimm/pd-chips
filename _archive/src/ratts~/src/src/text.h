/* $Id: text.h,v 1.1 2003/01/01 17:55:36 moocow Exp $
 *
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 */

/**
 * \file text.h
 * \brief Nick's implementation of US Naval Research Laboratory rules
 * for converting english (american?) text to phonemes, based on
 * the version on the comp.speech archives.
 */

#ifndef _RATTS_TEXT_H
#define _RATTS_TEXT_H

/// Predicate.  Assumes upper-case \b chr.
extern int isvowel PROTO((int chr));

/// Predicate.  Assumes upper-case \b chr.
extern int isconsonant PROTO((int chr));

/// concatenation function: instance = void phone_cat(void *arg, char *s)
///  + concatenates phones in (s) onto the end of (arg) -- for
///    phone_cat(), (arg) is a darray_ptr.
typedef void (*out_p) PROTO((void *arg,char *s));

/// Do text-to-phoneme conversion.
extern int NRL PROTO((char *s,unsigned n,darray_ptr phone));


#endif /* _RATTS_TEXT_H */
