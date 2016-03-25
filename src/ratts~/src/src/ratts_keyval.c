/*=============================================================================
 * File: ratts_keyval.c
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

#include "ratts_keyval.h"

/*
 * NOTE: I make the assumption that the symbol generator
 *   in PD correctly catches duplicate symbols and returns
 *   the same pointer for symbols(strings) that are the same 
 */

int ratts_keyval_hash (ratts_hash_key_t *key)
{
  switch (key->key_type) {
  case A_SYMBOL:
    return (int)(key->key.s);
  case A_FLOAT:
    return (int)(key->key.f);
  default:
    post("ratts_keyval: ERROR - ratts_keyval_hash() got invalid key type");
  }
  return 0;
}

int ratts_keyval_equal (ratts_hash_key_t *a, ratts_hash_key_t *b)
{
  if ( a->key_type != b->key_type ) return 0;

  switch (a->key_type)
    {
    case A_SYMBOL:
      return (a->key.s == b->key.s);
    case A_FLOAT:
      return (a->key.f == b->key.f);
    default:
      post("ratts_keyval: ERROR - ratts_keyval_equal() got invalid key type");
    }

  return 0;
}

void ratts_keyval_setfloatkey (ratts_hash_key_t *key, t_float f)
{
  key->key_type = A_FLOAT;
  key->key.f = f;
}

void ratts_keyval_setsymbolkey (ratts_hash_key_t *key, t_symbol *s)
{
  key->key_type = A_SYMBOL;
  key->key.s = s;
}

/*
void setfloatval_keyval (t_keyval *pair, t_float val)
{
	pair->val_type = A_FLOAT;
	pair->val.f = val;
}

void setsymbolval_keyval (t_keyval *pair, t_symbol *val)
{
	pair->val_type = A_SYMBOL;
	pair->val.s = val;
}
*/


void ratts_keyval_free(ratts_hash_key_t *key, ratts_hash_value_t *val)
{
  // -- free the key
  if (key) {
    freebytes(key, sizeof(ratts_hash_key_t));
  }
  // -- free the value
  if (val) {
    freebytes(val->vec, val->size * sizeof(t_atom));
    freebytes(val, sizeof(ratts_hash_value_t));
  }
}
