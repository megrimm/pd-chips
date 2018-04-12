/*=============================================================================
 * File: rattshash.h
 * Object: rattshash
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: auto-growing linked-list hashes for pd
 *
 * Based on:
 *   object :  Maphash 
 *   version:  1.0
 *   file   :  maphash.c
 *   author :  Orm Finnendahl, based on code for mapper 
 *             by Travis Newhouse (tnewhous@ucsd.edu)
 *             http://www-cse.ucsd.edu/~newhouse
 *   date   :  02/19/2002
 *
 * See rattshash.c for details.
 *=============================================================================*/

#ifndef _RATTSHASH_H
#define _RATTSHASH_H

#ifndef _RATTS_M_PD_H
#define _RATTS_M_PD_H
#include "m_pd.h"
#endif

#include "alhash.h"
#include "ratts_keyval.h"

#define RATTSHASH_DEFAULT_SIZE   197  // default hash size
#define RATTSHASH_DEFAULT_AUTOGROW 1  // default autogrow value

typedef struct
{
  t_object x_ob;
  t_symbol *x_hashname;
} t_rattshwrite;


typedef struct
{
  t_object x_ob;
  t_symbol *x_hashname;
  t_outlet *x_passout;
} t_rattshread;

typedef struct
{
  t_object        x_ob;
  t_symbol       *x_hashname;
  t_canvas       *x_canvas;
  alhash_table_t *x_table;
} t_rattshash;

extern void rattshread_setup(void);
extern void rattshwrite_setup(void);
extern void rattshash_setup(void);

#endif /* _RATTS_HASH_H */
