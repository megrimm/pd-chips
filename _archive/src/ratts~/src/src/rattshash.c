/*=============================================================================
 * File: rattshash.c
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
 *   differences to maphash (2003 Bryan Jurish):
 *   + renamed to rattshash
 *   + added larger hash-sizes; put sizes into their own file 'hashsizes.def'
 *   + added "hash" configuration messages
 *   + renamed 'hashtable.[ch]' to 'alhash.[ch]'
 *     - changed conflict-resolution strategy from linear search
 *       to linked-list table-entries
 *     - added code to allow table resizing
 *     - added hash table flag to resize table automatically
 *     - in my opinion, this constitutes a whole different
 *       flavor of hash table than the one used by 'maphash' and 'mapper'.
 * 
 *   differences to mapper (2002 Orm Finnendal):
 *
 *   - renamed to maphash
 * 
 *   - also lists of any content can be mapped to symbols or
 *     floats.
 * 
 *   - Two additional objects (mapread and mapwrite) can read and
 *     write values (to access mapped values remotely), much like
 *     the tabread and tabwrite objects in pd used for
 *     table(array) access. The reference of those objects to the
 *     hashtable is by name (also like tabread and tabwrite).
 * 
 *=============================================================================*/

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "alhash.h"
#include "ratts_keyval.h"
#include "rattshash.h"

/*-------------------------------------------------------------
 * GLOBALS
 */
static ratts_hash_key_t search_key;    // -- key structure used for all lookups


/*------------------------ rattshash --------------------------*/

t_class *rattshash_class;

/*******************************************************************
*  METHOD:   output_val
*  PURPOSE:  output the val field of a t_keyval object
*  ACTION:   send either a float or symbol out the maphash's outlet
********************************************************************/
static void rattshash_output_val (t_rattshash *x, ratts_hash_value_t *val)
{
  if ( val->vec[0].a_type == A_FLOAT )
    {
      outlet_list(x->x_ob.ob_outlet, gensym("list"), val->size, val->vec);
    }
  else if ( val->vec[0].a_type == A_SYMBOL )
    {
      outlet_anything(x->x_ob.ob_outlet, 
                      val->vec->a_w.w_symbol, 
                      val->size - 1, 
                      val->vec + 1);
    }
  else
    {
      post("rattshash_output_val(): ERROR - got invalid val type");
    }
}


/*******************************************************************
*  METHOD:   float
*  PURPOSE:  handle floats in the left inlet                      
*  ACTION:   output the value associated with the incoming float  
********************************************************************/
void rattshash_float (t_rattshash *x, t_floatarg f)
{
  alhash_entry_t *found;

  //TDEBUG_IN("rattshash_float");

  /* look for the incoming key */
  ratts_keyval_setfloatkey(&search_key, f);
  found = alhash_lookup_extended(x->x_table, &search_key);
	
  /* if we find something, send the value out the outlet */
  if ( found ) {
    rattshash_output_val(x, found->val);
    /*
      outlet_float(x->x_ob.ob_outlet, found->val);
    */
  }

  //TDEBUG_OUT("rattshash_float");
}


/*******************************************************************
*  METHOD:   symbol
*  PURPOSE:  handle symbols in the left inlet                      
*  ACTION:   output the value associated with the incoming symbol  
********************************************************************/
void rattshash_symbol (t_rattshash *x, t_symbol *s)
{
  alhash_entry_t *found;

  //TDEBUG_IN("rattshash_symbol");

  /* look for the incoming key */
  ratts_keyval_setsymbolkey(&search_key, s);
  found = alhash_lookup_extended(x->x_table, &search_key);
	
  /* if we find something, send the value out the outlet */
  if ( found )
    {
      rattshash_output_val(x, found->val);
      /*
        outlet_float(x->x_ob.ob_outlet, found->val);
      */
    }

  //TDEBUG_OUT("rattshash_symbol");
}



/*******************************************************************
*  METHOD:   rattshash_map
*  PURPOSE:  handle 'map' messages 
*  ACTION:   create an association between a key and a value  
********************************************************************/
void rattshash_map (t_rattshash *x, t_symbol *s, int argc, t_atom *argv)
{
  ratts_hash_key_t *k; 
  ratts_hash_value_t *v;
  int i;

  t_atom *source, *dest, *in_vec = (t_atom *)getbytes(argc * sizeof(t_atom));

  if (!in_vec) {
    post("rattshash_map(): ERROR: could not allocate memory for value-vector");
    return;
  }
  
  //TDEBUG_IN("rattshash_map");
  if (argc > 1) {
    // -- setup value-vector
    for (i = 1, dest = in_vec, source = argv+1; i < argc; i++, dest++, source++) 
      {
	switch (source->a_type) {
	case A_FLOAT: 
	  { 
	    dest->a_type = A_FLOAT;
	    dest->a_w.w_float = atom_getfloat(source);
	    break;
	  }
	case A_SYMBOL: 
	  {
	    dest->a_type = A_SYMBOL;       
	    dest->a_w.w_symbol = atom_getsymbol(source);
	    break; 
	  }
	default:
	  {
	    post("rattshash_map(): can't handle data type %d", source->a_type);
	    return;
	  }
	}
      }

    // -- set up key
    k = (ratts_hash_key_t *)getbytes(sizeof(ratts_hash_key_t));
    if (!k) {
      post("rattshash_map(): ERROR: could not allocate memory for key!");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      return;
    }

    switch (argv[0].a_type) {
    case A_FLOAT:
      {
	ratts_keyval_setfloatkey(k, argv[0].a_w.w_float);
	break;
      }
    case A_SYMBOL:
      {
	ratts_keyval_setsymbolkey(k, argv[0].a_w.w_symbol);
	break;
      }
    default:
      {
	post("rattshash_map(): ERROR - got unsupported key type");
	if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
	return;
      }
    }

    v = (ratts_hash_value_t *)getbytes(sizeof(ratts_hash_value_t));
    if (!v) {
      post("rattshash_map(): ERROR - could not allocate memory for value");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      if (k) freebytes(k,sizeof(ratts_hash_key_t));
      return;
    }
    
    v->size = argc - 1;
    v->vec = in_vec;

#if 0
    {
      alhash_entry_t *he = alhash_lookup_extended(x->x_table, k);
      ratts_hash_key t *hek = he ? ((ratts_hash_key_t *)he->key) : NULL;
      char *oldkey = hek && hek->key_type == A_SYMBOL ? hek->key.s->s_name : "???";
      if (he && k->key_type == A_SYMBOL && strcmp(oldkey, k->key.s->s_name)) {
	post("rattshash_map(): [DEBUG] overwrote old key '%s' with new key '%s'",
	     oldkey, k->key.s->s_name);
      }
    }
#else
    alhash_insert(x->x_table, k, v);
#endif
  }
  else {
    post("rattshash: Too few arguments for message map");
  }
  //TDEBUG_OUT("rattshash_map");
}


/*******************************************************************
*  METHOD:   rattshash_unmap
*  PURPOSE:  handle 'unmap' messages 
*  ACTION:   remove the entry for a partiuclar key 
********************************************************************/
void rattshash_unmap (t_rattshash *x, t_symbol *s, int argc, t_atom *argv)
{
  alhash_entry_t *going = NULL;

  //TDEBUG_IN("rattshash_unmap");

  if (argc < 1) {
    post("rattshash: bad syntax, expected 'unmap <key>'");
    return;
  }

  switch (argv[0].a_type) {
  case A_FLOAT:
    {
      ratts_keyval_setfloatkey(&search_key, argv[0].a_w.w_float);
      going = alhash_remove_extended(x->x_table, &search_key);
      break;
    }
  case A_SYMBOL:
    {
      ratts_keyval_setsymbolkey(&search_key, argv[0].a_w.w_symbol);
      going = alhash_remove_extended(x->x_table, &search_key);
      break;
    }
  default:
    {
      post("rattshash: unsupported key received for 'unmap'");
      return;
    }
  }

  if (going) {
    unsigned memsize = ((ratts_hash_value_t *)going->val)->size;
    /* deallocate the storage for the value vector created in map() */
    freebytes( ((ratts_hash_value_t *)going->val)->vec, memsize * sizeof(t_atom) );
    /* deallocate the key & value storage created in map() */
    freebytes( (ratts_hash_value_t *)going->key, sizeof(ratts_hash_key_t) );
    freebytes( (ratts_hash_value_t *)going->val, sizeof(ratts_hash_value_t) );
    /* free the hash-entry we've stolen */
    free(going);
  }
  else {
    post("rattshash: no entry for key given to unmap");
  }

  //TDEBUG_OUT("rattshash_unmap");
}

/*******************************************************************
*  METHOD:   rattshash_anything
*  PURPOSE:  handle anything in the left inlet                      
*  ACTION:   output the value associated with the incoming symbol  
********************************************************************/
void rattshash_anything (t_rattshash *x, t_symbol *s, int argc, t_atom *argv)
{
  alhash_entry_t *found;
  //TDEBUG_IN("rattshash_anything");

  if (argc > 0) post("rattshash: can't handle lists (message ignored)");
  else {
    /* look for the incoming key */
    ratts_keyval_setsymbolkey(&search_key, s);
    found = alhash_lookup_extended(x->x_table, &search_key);

    /* if we find something, send the value out the outlet */
    if ( found ) {
      rattshash_output_val(x, (ratts_hash_value_t *)found->val);
      /*
	outlet_float(x->x_ob.ob_outlet, found->val);
      */
    }
  }
  //TDEBUG_OUT("rattshash_anything");
}

/*******************************************************************
*  METHOD:   rattshash_list
*  PURPOSE:  handle list in the left inlet                      
*  ACTION:   output the value associated with the incoming symbol  
********************************************************************/

void rattshash_list (t_rattshash *x, t_symbol *s, int argc, t_atom *argv)
{
  //TDEBUG_IN("rattshash_list");

  post("rattshash: can't handle lists (message ignored)");

  //TDEBUG_OUT("rattshash_list");
}

/*----------------------------------------------------------------------
 *  METHOD:   rattshash_hash
 *  PURPOSE:  configure the hash table
 */
void rattshash_hash(t_rattshash *x, t_symbol *sel, int argc, t_atom *argv)
{
  while (argc > 0) {
    if (argv->a_type == A_SYMBOL) {
      sel = argv->a_w.w_symbol;
      argc--;
      argv++;
      if (sel == gensym("size")) {
	// -- report table size
	outlet_float(x->x_ob.ob_outlet, (t_float)x->x_table->table_size);
      }
      else if (sel == gensym("resize")) {
	// -- resize the table
	unsigned newsize;
	if (argc > 0) {
	  // -- explicit size requested
	  newsize = atom_getfloat(argv);
	} else {
	  // -- attempt to auto-size
	  newsize = x->x_table->slots_used;
	}
	alhash_resize(x->x_table, newsize);
	// -- report the new size
	outlet_float(x->x_ob.ob_outlet, (t_float)x->x_table->table_size);
      }
      else if (sel == gensym("autosize")) {
	// -- get/set autosize flag
	if (argc > 0) {
	  t_int as = atom_getint(argv);
	  if (as) {
	    x->x_table->flags |= ALHASH_AUTOGROW;
	  } else {
	    x->x_table->flags &= ~ALHASH_AUTOGROW;
	  }
	}
	outlet_float(x->x_ob.ob_outlet, (t_float)(x->x_table->flags & ALHASH_AUTOGROW));
      }
      else if (sel == gensym("clear")) {
	// -- clear the table
	alhash_clear(x->x_table);	
      }
      else if (sel == gensym("count")) {
	// -- output slots used
	outlet_float(x->x_ob.ob_outlet, (t_float)(x->x_table->slots_used));
      }
      else {
	post("rattshash: unknown configuration keyword '%s' in hash message",
	     sel->s_name);
      }
    }
    else {
      post("rattshash_hash(): cannot handle data type");
    }
    argc--;
    argv++;
  }
}

/*----------------------------------------------------------------------
 *  METHOD:   rattshash_load
 *  PURPOSE:  load table contents from a file
 */
void rattshash_load(t_rattshash *x, t_symbol *filename, t_symbol *format)
{
  // -- borrowed from x_qlist.c
  t_binbuf *binbuf = binbuf_new();
  int cr = 0;
  int count = 0;

  if (!strcmp(format->s_name, "cr"))
    cr = 1;
  else if (*format->s_name)
    error("rattshash_load: unknown flag: %s", format->s_name);

  post("rattshash: loading entries from file '%s'... ", filename->s_name);

  if (binbuf_read_via_path(binbuf,
			   filename->s_name,
			   canvas_getdir(x->x_canvas)->s_name,
			   cr))
    {
      error("%s: read failed", filename->s_name);
      binbuf_free(binbuf);
      return;
    }
  else
    {
      int
	argc = binbuf_getnatom(binbuf),
	onset = 0,
	onset2;
      t_atom *argv = binbuf_getvec(binbuf);

      while (onset < argc) {
	t_atom *ap = argv + onset;

	while (ap->a_type == A_SEMI) {
	  // -- ignore semicolons
	  onset++;
	  ap++;
	  if (onset >= argc) {
	    //post("rattshash_load: semi-check-loop: END detected");
	    goto end;
	  }
	}

	onset2 = onset;
	while (onset2 < argc && ap->a_type != A_SEMI) {
	  // -- gobble the key-value pair
#        if 0
	  if (ap->a_type == A_SYMBOL) {
	    post("rattshash_load: symbol %s", ap->a_w.w_symbol->s_name);
	  } else if (ap->a_type == A_FLOAT) {
	    post("rattshash_load: float %f", ap->a_w.w_float);
	  } else if (ap->a_type == A_SEMI) {
	    post("rattshash_load: A_SEMI");
	  } else if (ap->a_type == A_COMMA) {
	    post("rattshash_load: A_COMMA");
	  } else if (ap->a_type == A_NULL) {
	    post("rattshash_load: A_NULL");
	  } else if (ap->a_type == A_POINTER) {
	    post("rattshash_load: A_POINTER");
	  } 
	  else {
	    post("rattshash_load: unknown datatype %d", ap->a_type);
	  }
#        endif

	  onset2++;
	  ap++;
	}

	//post("rattshash_load: read message of length %d", onset2-onset);

	// -- store the loaded pair
	rattshash_map(x, 0, onset2-onset, argv+onset);
	count++;

	// pd-size : w/o  phone-spaces (slower!) = 34624k
	//         : with phone-spaces = 40764k
	//         : with numeric values = 40764k
	//         : with 200K hash defined, nothing loaded = 2144k
#      if 0
	if (count >= 524288) {
	  post("rattshash_load: count limit exceeded -- bailing out!");
	  goto end;
	}
#      endif
	
	onset = onset2;
	//post("rattshash_load: read %d binbuf entries from file %s.\n", filename->s_name);
      }
    }

 end:

  post("rattshash: %d entries loaded.", count, filename->s_name);

  binbuf_free(binbuf);
}


/*******************************************************************
*  METHOD:   rattshash_free
*  PURPOSE:  handle the destruction of a rattshash object
*  ACTION:   deallocate storage associated with the hashtable
********************************************************************/

void rattshash_free (t_rattshash *x) 
{
  alhash_destroy(x->x_table);
  x->x_table = NULL;

  /* unbind the symbol of the name of hashtable in pd's global namespace
   */
  pd_unbind((t_pd*)x, x->x_hashname);

  /* TODO: check if this is necessary */
  outlet_free(x->x_ob.ob_outlet);
}


/*----------------------------------------------------------------------
 * rattshash_new
 */
void* rattshash_new (t_symbol *sel, int argc, t_atom *argv)
{
  t_rattshash *x =0;
  t_symbol *name;
  t_int size = RATTSHASH_DEFAULT_SIZE;
  t_int autosize = 1;
  
  //TDEBUG_IN("rattshash_new");

  // -- first argument MUST be the hash name
  if (argc > 0 && argv->a_type == A_SYMBOL) {
    name = argv->a_w.w_symbol;
    argc--;
    argv++;
  } else {
    name = sel;
  }

  /* make sure this hashtable doesn't exist already */
  if (!(t_rattshash*)pd_findbyclass(name, rattshash_class)) {
    //search_key.key.f = -99.0; // ???

    // -- get configuration arguments
    while (argc > 0) {
      if (argv->a_type == A_SYMBOL) {
	sel = atom_getsymbol(argv);
	argv++;
	argc--;
	if (sel == gensym("size")) {
	  size = argc > 0 ? atom_getint(argv) : RATTSHASH_DEFAULT_SIZE;
	}
	else if (sel == gensym("autosize")) {
	  autosize = argc > 0 ? atom_getint(argv) : 1;
	}
	else {
	  // -- Unknown parameter
	  post("rattshash: unknown configuration parameter '%s' -- ignored.",
	       sel ? sel->s_name : "(nil)");
	}
      }
      else if (argv->a_type == A_FLOAT) {
	// -- lone float: size
	size = atom_getint(argv);
      }
      else {
	post("rattshash: unknown configuration parameter type -- ignored.");
      }
      argc--;
      argv++;
    }

    /* make sure size is valid */
    if (size < 0 ) size = RATTSHASH_DEFAULT_SIZE;

    /* allocate the new rattshash object */
    x = (t_rattshash *)pd_new(rattshash_class);

    /* bind the symbol of the name of hashtable in pd's global namespace
     *  (for retrieval by mapread and mapwrite)
     */
    pd_bind((t_pd*)x, name);
    x->x_hashname = name;

    /* create the hashtable to store the key-value pairs */
    x->x_table = alhash_new_full((int)size,
				 0,
				 (void *) ratts_keyval_hash, 
				 (void *) ratts_keyval_equal,
				 (void *) ratts_keyval_free);

    /* initialize table flags */
    if (autosize) {
      x->x_table->flags |= ALHASH_AUTOGROW;
    } else {
      x->x_table->flags &= ~ALHASH_AUTOGROW;
    }

    /* add an outlet */
    outlet_new(&x->x_ob, &s_anything);

    /* add canvas-pointer */
    x->x_canvas = canvas_getcurrent();
  }
  else {
    post("rattshash: hashtable %s already exists (initialization failed)", name->s_name);
  }

  //TDEBUG_OUT("rattshash_new");
  return (void*)x;
}


/*----------------------------------------------------------------------
 * rattshash_setup
 */
void rattshash_setup (void)
{
#ifdef RATTS_SHAMELESS
  /* post a (not-so-) little banner */
  post("ratts:      rattshash : Auto-resizing hash table");
  post("ratts:     rattshread : Auto-resizing hash table reader");
  post("ratts:    rattshwrite : Auto-resizing hash table writer");
#endif

  //TDEBUG_IN("ratts_keyval_setup");

  rattshash_class = class_new(gensym("rattshash"),
			      (t_newmethod)rattshash_new, 
			      (t_method)rattshash_free,
			      sizeof(t_rattshash),
			      CLASS_DEFAULT,
			      A_GIMME,
			      0);

  // -- Methods
  class_addfloat(rattshash_class, rattshash_float);
  class_addsymbol(rattshash_class, rattshash_symbol);
  class_addmethod(rattshash_class, (t_method)rattshash_map,    gensym("map"),   A_GIMME, 0);
  class_addmethod(rattshash_class, (t_method)rattshash_unmap,  gensym("unmap"), A_GIMME, 0);

  class_addmethod(rattshash_class, (t_method)rattshash_hash, gensym("hash"), A_GIMME, 0);
  class_addmethod(rattshash_class, (t_method)rattshash_load, gensym("load"), A_SYMBOL, A_DEFSYM, 0);

  class_addlist(rattshash_class, rattshash_list);
  class_addanything(rattshash_class, rattshash_anything);

  rattshread_setup();
  rattshwrite_setup();

  class_sethelpsymbol(rattshash_class, gensym("rattshash-help.pd"));

  //TDEBUG_OUT("ratts_keyval_setup");
}

/*------------------------ rattshread --------------------------*/

t_class *rattshread_class;

/*******************************************************************
*  METHOD:   rattshread_output_val
*  PURPOSE:  output the val field of a t_keyval object
*  ACTION:   send either a list or a anything out the rattshread's outlet
********************************************************************/
static void rattshread_output_val (t_rattshread *x, ratts_hash_value_t *val)
{
  if ( val->vec[0].a_type == A_FLOAT )
    {
      outlet_list(x->x_ob.ob_outlet, gensym("list"), val->size, val->vec);
    }
  else if ( val->vec[0].a_type == A_SYMBOL )
    {
      outlet_anything(x->x_ob.ob_outlet, 
                      val->vec->a_w.w_symbol, 
                      val->size - 1, 
                      val->vec + 1);
    }
  else
    {
      post("rattshread_output_val(): ERROR - got invalid val type");
    }
}


/*******************************************************************
*  METHOD:   rattshread_output_keyval
*  PURPOSE:  output the key and val field of a t_keyval object
*  ACTION:   send either a list or a anything out the rattshread's outlet
********************************************************************/
static void rattshread_output_keyval (t_rattshread *x, ratts_hash_key_t *k, ratts_hash_value_t *v)
{
  switch (k->key_type) {
  case A_FLOAT:
    {
      // -- this probably leaks...
      int size = (v->size + 1) * sizeof(t_atom);
      t_atom *tmp = (t_atom*)getbytes(size);
      SETFLOAT(tmp,(float) k->key.f);
      memmove(tmp+1, v->vec, (size - sizeof(t_atom)));
      outlet_list(x->x_ob.ob_outlet, gensym("list"), v->size+1, tmp);
      break;
    }
  case A_SYMBOL:
    {
      outlet_anything(x->x_ob.ob_outlet, 
                      k->key.s, 
                      v->size, 
                      v->vec);
      break;
    }
  default:
    {
      post("rattshread: ERROR - output_val() got invalid val type");
    }
  }
}


/*******************************************************************
*  METHOD:   rattshread_float
*  PURPOSE:  handle floats in the left inlet                      
*  ACTION:   output the value associated with the incoming float  
********************************************************************/
void rattshread_float (t_rattshread *x, t_floatarg f)
{
  alhash_entry_t *found;
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

  if (!ref) {
    post("rattshread: rattshash hashtable %s not found", x->x_hashname->s_name);
    return;
  }

  /* look for the incoming key */
  ratts_keyval_setfloatkey(&search_key, f);
  found = alhash_lookup_extended(ref->x_table, &search_key);
	
  /* if we find something, send the value out the first outlet */
  if ( found ) {
    rattshread_output_val(x, found->val);
  } else {
    // -- pass the unmatched key to the second outlet
    outlet_float(x->x_passout, f);
  }
}


/*******************************************************************
*  METHOD:   rattshread_symbol
*  PURPOSE:  handle symbols in the left inlet                      
*  ACTION:   output the value associated with the incoming symbol  
********************************************************************/
void rattshread_symbol (t_rattshread *x, t_symbol *s)
{
  alhash_entry_t *found;
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

  if (!ref) {
    post("rattshread: rattshash hashtable %s not found", x->x_hashname->s_name);
    return;
  }

  /* look for the incoming key */
  ratts_keyval_setsymbolkey(&search_key, s);
  found = alhash_lookup_extended(ref->x_table, &search_key);
	
  /* if we find something, send the value out the outlet */
  if ( found ) {
    rattshread_output_val(x, found->val);
  } else {
    // -- pass the unmatched key to the second outlet
    outlet_symbol(x->x_passout, s);
  }
}

/*******************************************************************
*  METHOD:   rattshread_set
*  PURPOSE:  handle 'set' messages 
*  ACTION:   associate new symbol namespace to object
********************************************************************/
void rattshread_set (t_rattshread *x, t_symbol *s)
{
  x->x_hashname = s;
}




/*******************************************************************
*  METHOD:   rattshread_anything
*  PURPOSE:  handle anything in the left inlet                      
*  ACTION:   output the value associated with the incoming symbol  
********************************************************************/
void rattshread_anything (t_rattshread *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 0) {
    post("rattshread: (anything) can't handle lists (message passed to second outlet)");
    outlet_anything(x->x_passout, s, argc, argv);
  }
  else
    {
      alhash_entry_t *found;
      t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

      if (!ref) {
	post("rattshread: rattshash hashtable %s not found", x->x_hashname->s_name);
	return;
      }

      /* look for the incoming key */
      ratts_keyval_setsymbolkey(&search_key, s);
      found = alhash_lookup_extended(ref->x_table, &search_key);
	
      /* if we find something, send the value out the outlet */
      if ( found ) {
	rattshread_output_val(x, found->val);
      } else {
	// -- pass the unmatched key to the second outlet
	outlet_anything(x->x_passout, s, argc, argv);
      }
    }
}



/*******************************************************************
*  METHOD:   rattshread_list
*  PURPOSE:  handle list in the left inlet                      
*  ACTION:   no action
********************************************************************/
void rattshread_list (t_rattshread *x, t_symbol *s, int argc, t_atom *argv)
{
  //TDEBUG_IN("rattshread_list");

  post("rattshread: (list) can't handle lists (message passed to second outlet)");
  //post("          : selector='%s'", s->s_name);
  outlet_anything(x->x_passout, s, argc, argv);

  //TDEBUG_OUT("rattshread_list");
}

/*******************************************************************
*  METHOD:   rattshread_dump
*  PURPOSE:  dump all mappings
*  ACTION:   send all hashtable key/valuelists to outlet
********************************************************************/

int rattshread_dump (t_rattshread *x, t_symbol *s)
{
  alhash_table_t *ht;
  alhash_iter_t   hi;
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

  if (!ref) {
    post("rattshread: rattshash hashtable '%s' not found for dump", x->x_hashname->s_name);
    return 0;
  }

  ht = ref->x_table;

  for (alhash_iter_begin(ht, &hi); hi.entry != NULL; alhash_iter_next(ht, &hi))
    {
      rattshread_output_keyval(x,
			       (ratts_hash_key_t *)hi.entry->key,
			       (ratts_hash_value_t *)hi.entry->val);
    }


  return 0;
}


/*******************************************************************
*  METHOD:   rattshread_new
*  PURPOSE:  handle the inititalization of a rattshread object
*  ACTION:   instantiate object and set reference to named hashtable
********************************************************************/

void* rattshread_new (t_symbol *s, t_floatarg size)
{
  t_rattshread *x;

  //search_key.key.f = -99.0; // ???
  //  search_key.val.f = -9999;
  //TDEBUG_IN("rattshread_new");
		
  /* make sure size is valid */

  /* allocate the new rattshread object */
  x = (t_rattshread*)pd_new(rattshread_class);

  /* create the hashtable to store the key-value pairs */
  x->x_hashname = s;

  /* add an inlet to receive lists */
  /* TODO: would like the 'map' command to come in a second inlet */
  /*
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym(), ???);
  */

  /* add an outlet */
  outlet_new(&x->x_ob, &s_anything);
  x->x_passout = outlet_new(&x->x_ob, &s_anything);

  //TDEBUG_OUT("rattshread_new");

  return (void*)x;
}

void rattshread_setup (void)
{
  //TDEBUG_IN("rattshread_setup");

  rattshread_class = class_new(gensym("rattshread"),
			       (t_newmethod)rattshread_new, 
			       0,
			       sizeof(t_rattshread),
			       CLASS_DEFAULT,
			       A_SYMBOL,
			       A_DEFFLOAT,
			       0);

  class_addfloat(rattshread_class, rattshread_float);
  class_addsymbol(rattshread_class, rattshread_symbol);
  class_addlist(rattshread_class, rattshread_list);
  class_addanything(rattshread_class, rattshread_anything);
  class_addmethod(rattshread_class, (t_method)rattshread_dump, gensym("dump"), 0);
  class_addmethod(rattshread_class, (t_method)rattshread_set,  gensym("set"),  A_SYMBOL, 0);

  class_sethelpsymbol(rattshread_class, gensym("rattshread-help.pd"));

  //TDEBUG_OUT("rattshread_setup");
}


/*------------------------ rattshwrite --------------------------*/

t_class *rattshwrite_class;

/*******************************************************************
*  METHOD:   rattshwrite_anything
*  PURPOSE:  handle arbitrary lists
*  ACTION:   create an association between a key and a value  
********************************************************************/
void rattshwrite_anything (t_rattshwrite *x, t_symbol *s, int argc, t_atom *argv)
{
  ratts_hash_key_t *k; 
  ratts_hash_value_t *v;
  t_atom *source, *dest, *in_vec;
  int i;
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

  if (!ref) {
    post("rattshwrite_anything(): rattshash hashtable '%s' not found", x->x_hashname->s_name);
    return;
  }

  in_vec = (t_atom *)getbytes(argc * sizeof(t_atom));
  if (!in_vec) {
    post("rattshwrite_anything(): ERROR: could not allocate memory for value-vector");
    return;
  }
  
  if (argc > 0) {
    // -- setup value-vector
    for (i = 0, dest = in_vec, source = argv; i < argc; i++, dest++, source++) 
      {
	switch (source->a_type) {
	case A_FLOAT: 
	  { 
	    dest->a_type = A_FLOAT;
	    dest->a_w.w_float = atom_getfloat(source);
	    break;
	  }
	case A_SYMBOL: 
	  {
	    dest->a_type = A_SYMBOL;       
	    dest->a_w.w_symbol = atom_getsymbol(source);
	    break; 
	  }
	default:
	  {
	    post("rattshwrite_anything(): can't handle data type");
	    return;
	  }
	}
      }

    // -- setup key
    k = (ratts_hash_key_t *)getbytes(sizeof(ratts_hash_key_t));
    if (!k) {
      post("rattshwrite_anything(): ERROR: could not allocate memory for key!");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      return;
    }
    ratts_keyval_setsymbolkey(k, s);

    // -- setup value
    v = (ratts_hash_value_t *)getbytes(sizeof(ratts_hash_value_t));
    if (!v) {
      post("rattshwrite_anything(): ERROR - could not allocate memory for value");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      if (k) freebytes(k,sizeof(ratts_hash_key_t));
      return;
    }
    
    v->size = argc;
    v->vec = in_vec;
    alhash_insert(ref->x_table, k, v);
  }
  else {
    post("rattshwrite_anything(): just key received (ignored)");
  }
  //TDEBUG_OUT("rattshash_map");
}


/*******************************************************************
*  METHOD:   rattshwrite_list
*  PURPOSE:  handle lists
*  ACTION:   create an association between a key and a value  
********************************************************************/
void rattshwrite_list (t_rattshwrite *x, t_symbol *s, int argc, t_atom *argv)
{
  ratts_hash_key_t *k; 
  ratts_hash_value_t *v;
  int i;
  t_atom *source, *dest, *in_vec;
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);

  if (!ref) {
    post("rattshwrite_list(): rattshash hashtable '%s' not found", x->x_hashname->s_name);
    return;
  }

  in_vec = (t_atom *)getbytes(argc * sizeof(t_atom));
  if (!in_vec) {
    post("rattshwrite_list(): ERROR: could not allocate memory for value-vector");
    return;
  }
  
  if (argc > 1) {
    // -- setup value-vector
    for (i = 1, dest = in_vec, source = argv+1; i < argc; i++, dest++, source++) 
      {
	switch (source->a_type) {
	case A_FLOAT: 
	  { 
	    dest->a_type = A_FLOAT;
	    dest->a_w.w_float = atom_getfloat(source);
	    break;
	  }
	case A_SYMBOL: 
	  {
	    dest->a_type = A_SYMBOL;       
	    dest->a_w.w_symbol = atom_getsymbol(source);
	    break; 
	  }
	default:
	  {
	    post("rattshwrite_list(): can't handle data type");
	    return;
	  }
	}
      }

    // -- set up key
    k = (ratts_hash_key_t *)getbytes(sizeof(ratts_hash_key_t));
    if (!k) {
      post("rattshwrite_list(): ERROR: could not allocate memory for key!");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      return;
    }

    switch (argv[0].a_type) {
    case A_FLOAT:
      {
	ratts_keyval_setfloatkey(k, argv[0].a_w.w_float);
	break;
      }
    case A_SYMBOL:
      {
	ratts_keyval_setsymbolkey(k, argv[0].a_w.w_symbol);
	break;
      }
    default:
      {
	post("rattshwrite_list(): ERROR - got unsupported key type");
	if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
	return;
      }
    }

    v = (ratts_hash_value_t *)getbytes(sizeof(ratts_hash_value_t));
    if (!v) {
      post("rattshwrite_list(): ERROR - could not allocate memory for value");
      if (in_vec) freebytes(in_vec,argc * sizeof(t_atom));
      if (k) freebytes(k,sizeof(ratts_hash_key_t));
      return;
    }
    
    v->size = argc - 1;
    v->vec = in_vec;
    alhash_insert(ref->x_table, k, v);
  }
  else {
    post("rattshwrite_list(): just key received (ignored)");
  }
}

/*******************************************************************
*  METHOD:   rattshwrite_set
*  PURPOSE:  handle 'set' messages 
*  ACTION:   associate new symbol namespace to object
********************************************************************/
void rattshwrite_set (t_rattshash *x, t_symbol *s)
{
  //TDEBUG_IN("rattshwrite_set");
  x->x_hashname = s;
  //TDEBUG_OUT("rattshwrite_set");
}



/*******************************************************************
*  METHOD:   rattshwrite_unmap
*  PURPOSE:  handle 'del' messages 
*  ACTION:   remove the entry for a partiuclar key 
********************************************************************/
void rattshwrite_unmap (t_rattshwrite *x, t_symbol *s, int argc, t_atom *argv)
{
  t_rattshash *ref = (t_rattshash*)pd_findbyclass(x->x_hashname, rattshash_class);
  alhash_entry_t *going = NULL;

  if (!ref) {
    post("rattshread: hashtable %s not found", x->x_hashname->s_name);
    return;
  }
  if (argc < 1) {
    post("rattshash: bad syntax, expected 'unmap <key>'");
    return;
  }


  switch (argv[0].a_type) {
  case A_FLOAT:
    {
      ratts_keyval_setfloatkey(&search_key, argv[0].a_w.w_float);
      going = alhash_remove_extended(ref->x_table, &search_key);
      break;
    }
  case A_SYMBOL:
    {
      ratts_keyval_setsymbolkey(&search_key, argv[0].a_w.w_symbol);
      going = alhash_remove_extended(ref->x_table, &search_key);
      break;
    }
  default:
    {
      post("rattshash: unsupported key received for 'unmap'");
      return;
    }
  }

  if (going) {
    unsigned memsize = ((ratts_hash_value_t *)going->val)->size;
    /* deallocate the storage for the value vector created in map() */
    freebytes( ((ratts_hash_value_t *)going->val)->vec, memsize * sizeof(t_atom) );
    /* deallocate the key & value storage created in map() */
    freebytes( (ratts_hash_value_t *)going->key, sizeof(ratts_hash_key_t) );
    freebytes( (ratts_hash_value_t *)going->val, sizeof(ratts_hash_value_t) );
    /* free the hash-entry we've stolen */
    free(going);
  }
  else {
    post("rattshwrite: no entry for key given to unmap");
  }
}


/*----------------------------------------------------------------------
 * rattshwrite_new
 */
void* rattshwrite_new (t_symbol *s, t_floatarg size)
{
  t_rattshwrite *x;

  //TDEBUG_IN("rattshwrite_new");
		
  /* allocate the new rattshwrite object */
  x = (t_rattshwrite*)pd_new(rattshwrite_class);

  /* add an outlet */
  outlet_new(&x->x_ob, &s_anything);

  /* save Name of hashtable */
  x->x_hashname = s;

  //TDEBUG_OUT("rattshwrite_new");

  return (void*)x;
}


/*----------------------------------------------------------------------
 * rattshwrite_setup
 */
void rattshwrite_setup (void)
{
  //TDEBUG_IN("rattshwrite_setup");

  rattshwrite_class = class_new(gensym("rattshwrite"),
				(t_newmethod)rattshwrite_new, 
				0,
				sizeof(t_rattshwrite),
				CLASS_DEFAULT,
				A_SYMBOL,
				A_DEFFLOAT,
				0);

  class_addlist(rattshwrite_class, (t_method)rattshwrite_list);
  class_addanything(rattshwrite_class, (t_method)rattshwrite_anything);
  class_addmethod(rattshwrite_class, (t_method)rattshwrite_set, gensym("set"), A_SYMBOL, 0);
  class_addmethod(rattshwrite_class, (t_method)rattshwrite_unmap, gensym("del"), A_GIMME, 0);
  class_addmethod(rattshwrite_class, (t_method)rattshwrite_unmap, gensym("delete"), A_GIMME, 0);

  class_sethelpsymbol(rattshwrite_class, gensym("rattshwrite-help.pd"));

  //TDEBUG_OUT("rattshwrite_setup");
}



