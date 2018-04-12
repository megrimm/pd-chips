/*
 * $Id: darray.c,v 1.1 2003/01/01 17:55:36 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: modified by moocow
 *   + removed calls to 'memset' on initialization and extension
 *     (faster but more dangerous)
 */
char *darray_id = "$Id: darray.c,v 1.1 2003/01/01 17:55:36 moocow Exp $";

#include <config.h>
#include <useconfig.h>
#include "proto.h"
#include "darray.h"

void
darray_free(a)
darray_t *a;
{
 if (a->data)
  {
   free(a->data);
   a->data = NULL;
  }
 a->items = a->alloc = 0;
}

void *
Darray_find(a, n)
darray_t *a;
unsigned n;
{
 if (n >= a->alloc || n >= a->items)
  {
   unsigned osize = a->items * a->esize;
   unsigned nsize;
   if (!a->esize)
    abort();
   if (n >= a->alloc)
    {
     unsigned add = (a->get) ? a->get : 1;
     char *ndata = (char *) malloc(nsize = (n + add) * a->esize);
     if (ndata)
      {
       if (osize)
        memcpy(ndata, a->data, osize);
       if (a->data)
        free(a->data);
       a->data = ndata;
       a->alloc = n + add;
      }
     else
      return NULL;
    }
   else
    nsize = (n + 1) * a->esize;
   if (n >= a->items)
    {
      //memset(a->data + osize, 0, nsize - osize);
     a->items = n + 1;
    }
  }
 return (void *) (a->data + n * a->esize);
}

int
darray_delete(a, n)
darray_t *a;
unsigned n;
{
 char *p = (char *) darray_find(a, n);
 if (p)
  {
   if (a->items)
    {
     a->items--;
     while (n++ < a->items)
      {
       memcpy(p, p + a->esize, a->esize);
       p += a->esize;
      }
     //memset(p, 0, a->esize);
     return 1;
    }
   else
    abort();
  }
 else
  return 0;
}
