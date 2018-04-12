/*
 * $Id: trie.h,v 1.2 2003/01/01 17:42:04 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 */

/**
 * \file trie.h
 * \brief Trie structure using linked lists.
 */

#ifndef _RATTS_TRIE_H
#define _RATTS_TRIE_H

#include "proto.h"

/// Trie structure using linked lists.
typedef struct
{
  struct trie_s *otherwise; ///< Trie for non-matches
  struct trie_s *more;      ///< Trie for match continuation
  void *value;              ///< Node value for terminal match
  char ch;                  ///< Node key
} trie_s, *trie_ptr;

/// Insert a value into a trie.
extern void trie_insert PROTO((trie_ptr *r,char *s,void *value));

/// Lookup the longest match for (*sp) from a trie.
/// *sp is set to the longest matched prefix on return.
extern void *trie_lookup PROTO((trie_ptr *r,char **sp));

#endif /* _RATTS_TRIE_H */
