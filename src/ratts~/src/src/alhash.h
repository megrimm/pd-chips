/************************************************************
 * File: alhash.h
 * Maintainer: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: abstract-hashing linked-list auto-growable hash table
 *
 *  Based on:
 *   object :  Mapper 
 *   version:  1.0
 *   file   :  hashtable.h
 *   author :  Travis Newhouse
 *             tnewhous@ucsd.edu
 *             http://www-cse.ucsd.edu/~newhouse
 *   date   :  09/21/99
 *
 ************************************************************/

#ifndef _ALHASH_H
#define _ALHASH_H

/*----------------------------------------------------------------------
 * typedefs
 */

/// typedef for hashing functions : hashval = hash(key)
typedef int (*alhash_hash_func) (void *);

/// typedef for equality predicates : equal = equal(key1,key2) ? yup : nope
typedef int (*alhash_equal_func) (void *, void *);

/// typedef for reapers: reaper(key,val)
typedef void (*alhash_free_func) (void *, void *);


/// typedef for individual hash-table entries.
typedef struct alhash_entry
{
  void *key;    ///< key of the entry
  void *val;    ///< value of the entry
  void *next;   ///< next entry with the same hash-value
} alhash_entry_t;

/// typedef for linked-list hash table.
typedef struct alhash_table
{
  alhash_entry_t   **table;       ///< array of pointers to table-entries
  alhash_hash_func   hash;        ///< hashing function
  alhash_equal_func  equal;       ///< key-equality predicate
  alhash_free_func   ufree;       ///< key-equality predicate
  unsigned           table_size;  ///< number of allocated entry-pointers in table
  unsigned           slots_used;  ///< number of entries in the hash table (used for auto-extend)
  unsigned           flags;       ///< bitmask of flags
} alhash_table_t;

/// hash iterator structure
typedef struct alhash_iter {
  alhash_entry_t *entry;          ///< current entry, or NULL if end-of-hash
  unsigned        hkey;           ///< hash-value of current entry
} alhash_iter_t;

/*----------------------------------------------------------------------
 * Constants
 */
/// Set this flag to allow your table to auto-grow on insert().
#define ALHASH_AUTOGROW 0x01

/*----------------------------------------------------------------------
 * common hashing funcs
 */
/// direct hashing function.
extern int alhash_direct_hash(void *key);

/// direct equality predicate.
extern int alhash_direct_equal(void *key1, void *key2);


/*----------------------------------------------------------------------
 * Initialization / Destruction
 */

/// Create and return a new hash table with space for at least (size) entry-pointers.
/// (size) will be rounded up to the next pre-defined table size (see 'hashsizes.def').
extern alhash_table_t *alhash_new (unsigned size, alhash_hash_func hash, alhash_equal_func equal);

/// Create and return a new hash table with destruction callback.
extern alhash_table_t *alhash_new_full (unsigned size,
					unsigned flags,
					alhash_hash_func hash,
					alhash_equal_func equal,
					alhash_free_func ufree);

/// Clear a hash table.
extern void alhash_clear (alhash_table_t *ht);

/// Utterly destroy a hash table.
extern void alhash_destroy (alhash_table_t *ht);

/// Resize a hash table to at least (newsize).
/// You can use this to explicitly grow or to shrink a hash-table.
extern int alhash_resize (alhash_table_t *ht, unsigned newsize);

/*----------------------------------------------------------------------
 * Insert / Lookup / Remove
 */

/// Insert an entry into the hash table.  If (ht->auto_extend) is true,
/// then the table may grow.
extern int alhash_insert (alhash_table_t *ht, void *key, void *val);

/// Insert an entry into the hash table.  If (ht->auto_extend) is true,
/// then the table may grow.
extern void *alhash_lookup (alhash_table_t *ht, void *key);

/// Extended lookup function -- useful if your values may be NULL.
extern alhash_entry_t *alhash_lookup_extended (alhash_table_t *ht, void *key);

/// Remove the entry for (key) from the hash table.
extern void *alhash_remove (alhash_table_t *ht, void *key);

/// Remove the entry for (key) from the hash table -- extended version.
extern alhash_entry_t *alhash_remove_extended (alhash_table_t *ht, void *key);


/*----------------------------------------------------------------------
 * Iteration
 */

/// Initialize an iterator to point to the first entry in the hash table.
/// If end-of-hash is reached, (hti->entry) will be NULL, and NULL is returned.
/// (hti) should already have been allocated.
extern alhash_iter_t *alhash_iter_begin (alhash_table_t *ht, alhash_iter_t *hti);

/// Increment a hash-table iterator.  Returns (*hti) on success.
/// If end-of-hash is reached, (hti->entry) will be NULL, and NULL is returned.
extern alhash_iter_t *alhash_iter_next (alhash_table_t *ht, alhash_iter_t *hti);


#endif /* ALHASH_H */
