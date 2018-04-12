/*
 * $Id: phoneutils.c,v 1.1 2003/01/02 00:45:28 moocow Exp $
 *
 * Based on code from 'rsynth', which is:
 * 
 *   Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: file created by moocow
 *   + shared phonetic utilities
 *
 */

#include "darray.h"
#include "phoneutils.h"


int phone_append(darray_ptr p, int ch)
{
 char *s = (char *) darray_find(p, p->items);
 *s = ch;
 return ch;
}

unsigned phone_append_string(darray_ptr p, char *s)
{
  unsigned nph = 0;
  while (*s) {
    phone_append(p, *s++);
    nph++;
  }
 return nph;
}
