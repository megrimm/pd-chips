/* $Id: darray.h,v 1.3 2003/01/02 00:45:28 moocow Exp $
 *
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 */

/**
 * \file darray.h
 * \brief Dynamically allocated array utilities.
 */

#if !defined(DARRAY_H)
#define DARRAY_H

#include <stdlib.h>
#include "proto.h"

/// Dynamically allocated array-like structure.
typedef struct
{
  char     *data;          ///< the items
  unsigned items;          ///< number of slots used
  unsigned alloc;          ///< number of slots allocated
  unsigned short esize;    ///< size of a single item (in bytes)
  unsigned short get;      ///< number to get (step-size)
} darray_t, *darray_ptr;

/**
 * \brief Use 'darray_find()' instead.
 *
 * Return pointer to nth item, creating it if it doesn't exist.
 * returns NULL on allocation error.
 */
extern void *Darray_find PROTO((darray_t *a,unsigned n));

/// Deletes the nth item, zeroing its contents.
extern int darray_delete PROTO((darray_t *a,unsigned n));

/// Frees the whole array.
extern void darray_free  PROTO((darray_t *a));

#if defined(__GNUC__)
/// Initialize a darray for elements of size 'size' with step-size 'get'
static inline void darray_init(darray_t *a,unsigned size,unsigned get)
{
 a->esize = size;
 a->get   = get;
 a->items = a->alloc = 0;
 a->data = NULL;
}

/// Return the nth element of 'a', creating if it doesn't yet exist.
/// Returns NULL on error.
static inline void *darray_find(darray_t *a,unsigned n)
{
 if (n < a->alloc && n < a->items)
  return (void *) (a->data + n * a->esize);
 return Darray_find(a,n);
}
#else

/* cryptic, but useful for auto-extraction --moocow */
#define darray_init(a,sz,gt) \
 ((a)->esize = (sz), (a)->get = (gt), (a)->items = (a)->alloc = 0, (a)->data = NULL)

/* ditto --moocow */
#define darray_find(a,n) \
 (((n) < (a)->alloc && (n) < (a)->items) \
  ? (void *) ((a)->data + (n) * (a)->esize) \
  : Darray_find(a,n))

#endif 
#endif

