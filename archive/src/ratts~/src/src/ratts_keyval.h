/*=============================================================================
 * File: ratts_keyval.h
 * Maintainer: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: Hashing typedefs and routines for ratts_hash.
 *
 * Based on:
 *  object :  Mapper 
 *  version:  1.0
 *  file   :  keyval.h
 *  author :  Travis Newhouse
 *            tnewhous@ucsd.edu
 *            http://www-cse.ucsd.edu/~newhouse
 *  date   :  09/21/99
 *=============================================================================*/


#ifndef _RATTS_KEYVAL_H
#define _RATTS_KEYVAL_H

#ifndef _RATTS_M_PD_H
#define _RATTS_M_PD_H
#include "m_pd.h"
#endif

/*******************************************************
 * keyval - defines a datatype which can be stored in 
 *          a hashtable.  The type associates a key to
 *          a float value.  The key may be either a PD
 *          symbol or a float value.
 *******************************************************/

typedef struct {
  int size;
  t_atom *vec;
} ratts_hash_value_t;

typedef struct {
  t_atomtype key_type;
  union {
    t_float f;
    t_symbol *s;
  } key;
} ratts_hash_key_t;


extern int  ratts_keyval_hash (ratts_hash_key_t *key);
extern int  ratts_keyval_equal (ratts_hash_key_t *a, ratts_hash_key_t *b);
extern void ratts_keyval_setfloatkey (ratts_hash_key_t *key, t_float f);
extern void ratts_keyval_setsymbolkey (ratts_hash_key_t *key, t_symbol *s);

// functions removed as we only store pointers to the actual lists
// in the value field anymore
//void setfloatval_keyval (t_keyval *pair, t_float val);
//void setsymbolval_keyval (t_keyval *pair, t_symbol *val);

// callbacks for alhash
extern void ratts_keyval_free(ratts_hash_key_t *key, ratts_hash_value_t *val);

#endif /* _RATTS_KEYVAL_H */

