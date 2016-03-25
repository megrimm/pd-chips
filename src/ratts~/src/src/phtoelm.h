/*
 * $Id: phtoelm.h,v 1.7 2003/06/21 11:24:52 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 12/2002: modified by moocow
 *  - removed phone_append() to phoneutils.h
 *  - added state type and parameter
 *  - changed element encoding
 *  - changed output type from darray to dsqueue
 *
 */

#ifndef _RATTS_PHTOELM_H
#define _RATTS_PHTOELM_H

#include "dsqueue.h"
#include "trie.h"

/**
 * \file phtoelm.h
 * \brief Phone-to-Element conversion utilities.
 */

/*----------------------------------------------------------------------
 * types
 */

/// pte_eltseq_t: strings of the form "<N><ElementIndex1>...<ElementIndexN>"
typedef char *pte_eltseq_str;

// -- Accessors for pte_eltseq_t
/// ptesGetElt(e,i) : returns i'th element in pte_eltseq_t e
#define ptesGetElt(e,i) e[i]

/// ptesSetElt(e,i,c) : set i'th element in pte_eltseq_t e to char c
#define ptesSetElt(e,i,c)  e[i] = c



/* -- UNUSED --
/// pte_phone_t: 'unsigned int's in which each byte represents
/// a single character in an arbabet-derived phone-encoding .
/// Least significant byte first, 0 byte is end-of-phone.
/// Caveat: your 'unsigned int' must be at least 16
/// bytes wide for this to work with the arbabet derivate
/// used here.
typedef unsigned int pte_phone_t;

// -- Accessors for pte_phone_t
/// ptepGetPhoneC(p,i) : returns i'th phone-char of pte_phone_t p
#define ptepGetPhoneC(p,i) ((p<<(i*8))&0xff)

// -- Manipulators for pte_phone_t
/// ptepSetPhoneC(p,i,c) : set i'th phone of p to char c.
#define ptepSetPhoneC(p,i,c) p &= ~(0xff<<(i*8)); p |= ((c&0xff)<<(i*8))

/// ptepClearPhoneC(p,i,c) : clears (zeros) the i'th phone of p.
#define ptepClearPhoneC(p,i) p &= ~(0xff<<(i*8))
*/

// -- Accessors for pte_phone_t
/// ptepGetPhoneC(p,i) : returns i'th phone-char of pte_phone_t p
#define ptepGetPhoneC(p,i) p[i]

// -- Manipulators for pte_phone_t
/// ptepSetPhoneC(p,i,c) : set i'th phone of p to char c.
#define ptepSetPhoneC(p,i,c) p[i] = c

/// ptepClearPhoneC(p,i) : clears (zeros) the i'th phone of p.
#define ptepClearPhoneC(p,i) p[i] = 0



/// State variable for phone_to_elm() conversion
typedef struct {
  char              *s;       ///< current position in current phone-string

  int              stress;  ///< current stress value (?)
  unsigned         t;       ///< duration of generated phones (in speed-adjusted frames)
} phtoelm_state_t, *phtoelm_state_ptr;


/*----------------------------------------------------------------------
 * Globals
 */
/// Lookup table for phoneme-to-Holmes-element conversion.
///    Keys: phoneme names (char *)
///  Values: pte_elt_seq
extern trie_ptr phtoelm;

/*----------------------------------------------------------------------
 * Initialization
 */

/// Initialize global 'phtoelm' lookup table
/// -- formerly 'enter_phonemes()', static to phtoelm.c
extern void phtoelm_enter_phonemes(void);

/// Create/initialize/reset a phtoelm_state_t structure to the defaults.
/// ps may be NULL, in which case a new structure will be allocated.
/// Returns a pointer to the the state structure on success, NULL on failure.
extern phtoelm_state_t *phtoelm_init_state(phtoelm_state_t *ps);

/// Initialize a new utterance.
extern void phtoelm_init_utterance(phtoelm_state_t *ps);

/// Clear a phtoelm_state_t structure (currently just calls phtoelm_init_utterance()).
extern void phtoelm_clear_state(phtoelm_state_t *ps);

/// Free a phtoelm_state_t structure.
/// Really just clears the structure -- you need to call free()
/// yourself.
extern void phtoelm_free_state(phtoelm_state_t *ps);

/*----------------------------------------------------------------------
 * Conversion
 */

/// Try and get the element-sequence for the next phone in ps->s.
/// If successful, the next element-sequence is returned.
/// Returns NULL on empty string or unknown phone.
extern pte_eltseq_str pte_get_next_eltseq(phtoelm_state_t *ps);

/// Append Holmes-elements for phones in (phonestr) to (eltq)
/// Returns ps->t.
extern unsigned phone_to_elm(phtoelm_state_t *ps, char *phonestr, dsqueue_t *eltq);


#endif /* _RATTS_PHTOELM_H */
