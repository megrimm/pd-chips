/************************************************************
 * File: alhash.c
 * Maintainer: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: abstract-hashing linked-list auto-growable hash table
 *
 *  Based on:
 *   object :  Mapper 
 *   version:  1.0
 *   file   :  hashtable.c
 *   author :  Travis Newhouse
 *             tnewhous@ucsd.edu
 *             http://www-cse.ucsd.edu/~newhouse
 *   date   :  09/21/99
 *
 ************************************************************/

#ifdef ALHASH_DEBUG
# include <stdio.h>
#endif

#include <stdlib.h>
#include "alhash.h"

static const int SIZES_COUNT = 26;
static const int SIZES[] =
  {
#include "hashsizes.def"
  };


/*----------------------------------------------------------------------
 * common hashing funcs
 */
/// direct hashing function.
extern int alhash_direct_hash(void *key) {
  return (int)key;
}

/// direct equality predicate.
extern int alhash_direct_equal(void *key1, void *key2) {
  return (key1 == key2);
}


/*----------------------------------------------------------------------
 * Initialization / Destruction
 */
extern alhash_table_t *alhash_new_full (unsigned size,
					unsigned flags,
					alhash_hash_func hash,
					alhash_equal_func equal,
					alhash_free_func ufree)
{
  int i=0;
  alhash_table_t *new_tab;

  while (size > SIZES[i] && i < SIZES_COUNT) {
    ++i;
  }
  size = SIZES[i];
  
  new_tab = (alhash_table_t *) malloc( sizeof(alhash_table_t) );
  if (!new_tab) return NULL;

  new_tab->table = (alhash_entry_t **)malloc( size * sizeof(alhash_entry_t *) );
  if (!new_tab->table) {
    free(new_tab);
    return NULL;
  }

  new_tab->table_size = size;
  new_tab->hash = hash;
  new_tab->equal = equal;
  new_tab->ufree = ufree;
  new_tab->slots_used = 0;
  new_tab->flags = flags;
   
  /* initialize all entries to NULL */
  for (i=0; i < size; ++i) {
    new_tab->table[i] = NULL;
  }
  
  return new_tab;
}

alhash_table_t* alhash_new (unsigned size, alhash_hash_func hash, alhash_equal_func equal)
{
  return alhash_new_full(size,0,hash,equal,NULL);
}


void alhash_clear (alhash_table_t *ht)
{
  int i;
  for (i = 0; i < ht->table_size; i++) {
    while (ht->table[i]) {
      alhash_entry_t *he_next = ht->table[i]->next;
      if (ht->ufree) ht->ufree(ht->table[i]->key, ht->table[i]->val);
      free(ht->table[i]);
      ht->table[i] = he_next;
    }
  }
  ht->slots_used = 0;
}


void alhash_destroy (alhash_table_t *ht)
{
  alhash_clear(ht);
  free(ht->table);
  free(ht);
}



int alhash_resize (alhash_table_t *ht, unsigned newsize) {
  alhash_entry_t **new_table = NULL;
  alhash_entry_t *he, *he_next;
  int i = 0;

  while (newsize > SIZES[i] && i < SIZES_COUNT) {
    ++i;
  }
  newsize = SIZES[i];

  if (newsize == ht->table_size) return 1;

#ifdef ALHASH_DEBUG
  fprintf(stderr, "alhash_resize(): old=%u ; used=%u ; new=%u\n",
	  ht->table_size, ht->slots_used, newsize);
#endif

  new_table = (alhash_entry_t **)malloc( newsize * sizeof(alhash_entry_t *) );
  if (!new_table) return 0;

  /* initialize new entries to NULL */
  for (i=0; i < newsize; ++i) {
    new_table[i] = NULL;
  }

  /* copy old entries */
  for (i=0; i < ht->table_size; i++) {
    for (he = ht->table[i]; he; he = he_next) {
      int hkey = ht->hash(he->key) % newsize;
      he_next = he->next;
      he->next = new_table[hkey];
      new_table[hkey] = he;
    }
  }

  /* free old table */
  free(ht->table);
  ht->table = new_table;
  ht->table_size = newsize;
  return 1;
}


/*----------------------------------------------------------------------
 * Insert / Lookup / Remove
 */
int alhash_insert (alhash_table_t *ht, void *key, void *val)
{
  int hkey = ht->hash(key) % ht->table_size;
  alhash_entry_t *he = ht->table[hkey];

  /* Uses linked-list collision resolution strategy */
  while ( he && !(ht->equal(he->key, key)) ) {
    he = he->next;
  }
  if (!he) {
    // -- allocate new entry
    he = (alhash_entry_t *)malloc(sizeof(alhash_entry_t));
    if (!he) return 0;
    he->key = key;
    he->next = ht->table[hkey];
    ht->table[hkey] = he;
    ht->slots_used++;
  }
  he->val = val;
  
  /* do we need to auto-grow? */
  if (ht->slots_used > ht->table_size && ht->flags & ALHASH_AUTOGROW) {
    alhash_resize(ht,ht->slots_used);
  }

  return 1;
}


alhash_entry_t *alhash_lookup_extended (alhash_table_t *ht, void *key)
{
  int hkey = ht->hash(key) % ht->table_size;
  alhash_entry_t *he = ht->table[hkey];
  /* Uses linked-list collision resolution strategy */
  while ( he && (!ht->equal(he->key, key)) ) {
    he = he->next;
  }
  return he;
}

void* alhash_lookup (alhash_table_t *ht, void *key)
{
  alhash_entry_t *he = alhash_lookup_extended(ht,key);
  return he ? he->val : NULL;
}


alhash_entry_t *alhash_remove_extended (alhash_table_t *ht, void *key)
{
  int hkey = ht->hash(key) % ht->table_size;
  alhash_entry_t *he = ht->table[hkey], *he_prev = NULL;

  /* Uses linked-list collision resolution strategy */
  while (he && (!ht->equal(he->key, key)) ) {
    he_prev = he;
    he = he->next;
  }

  if (he) {
    if (he_prev) {
      he_prev->next = he->next;
    } else {
      ht->table[hkey] = he->next;
    }
    ht->slots_used--;
  }

  return he;
}


void* alhash_remove (alhash_table_t *ht, void *key)
{
  alhash_entry_t *he = alhash_remove_extended(ht,key);
  if (he) {
    void *item = he ? he->val : NULL;
    free(he);
    return item;
  }
  return NULL;
}


/*----------------------------------------------------------------------
 * Iteration
 */

extern alhash_iter_t *alhash_iter_begin (alhash_table_t *ht, alhash_iter_t *hti) {
  for (hti->hkey = 0; hti->hkey < ht->table_size; hti->hkey++) {
    if ((hti->entry = ht->table[hti->hkey])) return hti;
  }
  hti->entry = NULL;
  return NULL;
}

// Increment a hash-table iterator.  Returns (*hti) on success.
// If end-of-hash is reached, (hti->entry) will be NULL, and NULL is returned.
extern alhash_iter_t *alhash_iter_next (alhash_table_t *ht, alhash_iter_t *hti) {
  if (!hti || !hti->entry) return NULL;
  else if ((hti->entry = hti->entry->next)) return hti;
  for (hti->hkey++; hti->hkey < ht->table_size; hti->hkey++, hti->entry++) {
    if ((hti->entry = ht->table[hti->hkey])) return hti;
  }
  hti->entry = NULL;
  return NULL;
}
