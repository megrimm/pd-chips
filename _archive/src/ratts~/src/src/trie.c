/*
 * $Id: trie.c,v 1.2 2003/01/01 17:55:36 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

char *trie_id = "$Id: trie.c,v 1.2 2003/01/01 17:55:36 moocow Exp $";

#include <useconfig.h>
#include <stdio.h>
#include "proto.h"
#include "trie.h"

void
trie_insert(r, s, value)
trie_ptr *r;
char *s;
void *value;
{
 trie_ptr p = NULL;
 char ch;
 while ((ch = *s++))
  {
   while ((p = *r))
    {
     if (p->ch == ch)
      break;
     else
      r = (trie_ptr *)&p->otherwise;
    }
   if (!p)
    {
     p = (trie_ptr) malloc(sizeof(*p));
     memset(p, 0, sizeof(*p));
     p->ch = ch;
     *r = p;
    }
   r = (trie_ptr *)&p->more;
  }
 p->value = value;
}

void *
trie_lookup(r, sp)
trie_ptr *r;
char **sp;
{
 char *s = *sp;
 char *value = NULL;
 char ch;
 while ((ch = *s))
  {
   trie_ptr *l = r;
   trie_ptr p;
   while ((p = *l))
    {
     if (p->ch == ch)
      break;
     else
      l = (trie_ptr *)&p->otherwise;
    }
   if (p)
    {
     *l = (trie_ptr)p->otherwise;
     p->otherwise = *((struct trie_s **)r);
     *r = p;
     r = (trie_ptr *)&p->more;
     value = (char *) p->value;
     s++;
    }
   else
    break;
  }
 *sp = s;
 return value;
}
