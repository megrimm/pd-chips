/*
 * $Id: phtoelm.c,v 1.8 2003/01/03 13:35:13 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 12/2002: modified by moocow
 */
char *phtoelm_id = "$Id: phtoelm.c,v 1.8 2003/01/03 13:35:13 moocow Exp $";

#include <stdio.h>
#include <ctype.h>
#if defined (__STDC__)
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#include <errno.h>

#include "useconfig.h"
#include "proto.h"

#include "elements.h"
#include "phfeat.h"
#include "trie.h"
#include "phtoelm.h"
#include "rholmes.h"
#include "parwave.h"

#ifdef PHOLMES_DEBUG
# ifndef _RATTS_M_PD_H
#  include <m_pd.h>
# endif
#endif

/*----------------------------------------------------------------------
 * Globals & defines
 */

// Lookup table for phoneme-to-Holmes-element conversion.
//    Keys: phoneme names (char *)
//  Values: pte_eltseq_str (char *)
trie_ptr phtoelm = NULL;

/*----------------------------------------------------------------------
 * Initialization
 */

// Lookup an element in Elements[] by name.
// Returns a pointer to the element, or NULL on failure.
// Slow (linear search), but it only needs to happen once at startup.
static Elm_ptr find_elm(char *s)
{
 Elm_ptr e = Elements;
 while (e < Elements + num_Elements) {
   if (!strcmp(s, e->name)) return e;
   e++;
 }
 return NULL;
}

/// Enter a phoneme-to-element-sequence in the global lookup table 'phtoelm'
#if defined (__STDC__)
static void enter(char *p,...)
#else
static void enter(p, va_alist)
char *p;
va_dcl
#endif
{
 va_list ap;
 char *s;
 char buf[20];
 char *x = buf+1;
#if defined(__STDC__)
 va_start(ap, p);
#else
 va_start(ap);
#endif
 while ((s = va_arg(ap, char *))) {
   Elm_ptr e = find_elm(s);
   if (e) {
     *x++ = (e - Elements);
   } else {
     fprintf(stderr, "phtoelm.c: enter(): cannot find element '%s'\n", s);
   }
 }
 va_end(ap);

 buf[0] = (x - buf) - 1;
 x = malloc(buf[0] + 1);
 memcpy(x, buf, buf[0] + 1);
 trie_insert(&phtoelm, p, x);
}

/// Enter all known phonemes into the lookup table.
void phtoelm_enter_phonemes PROTO((void))
{
#include "phtoelm.def"
}


// Initialize a phtoelm_state_t structure.
phtoelm_state_t *phtoelm_init_state(phtoelm_state_t *ps) {
  if (!phtoelm) phtoelm_enter_phonemes();

  // allocated phtoelm state structure
  if (!ps) {
    ps = (phtoelm_state_t *)malloc(sizeof(phtoelm_state_t));
    //ps->phoneq = NULL;
  }
  if (!ps) {
    errno = ENOMEM;
    return NULL;
  }

  ps->s = NULL;
  phtoelm_init_utterance(ps);
  return ps;
}


// Initialize a new utterance.
void phtoelm_init_utterance(phtoelm_state_t *ps) {
  ps->stress = 0;
  ps->t = 0;
}


// Clear a phtoelm_state_t structure (reset utterance).
void phtoelm_clear_state(phtoelm_state_t *ps) {
  phtoelm_init_utterance(ps);
}

// Free a phtoelm_state_t structure.
void phtoelm_free_state(phtoelm_state_t *ps) {
  phtoelm_clear_state(ps);
}


/*----------------------------------------------------------------------
 * Conversion
 */


/// StressDur(e,st,sp) : computes stress-adjusted duration of Elm_t *elt.
/// @param e  : Elm_t *
/// @param st : stress
// @param sp : speed coefficient
#if 0
# define StressDur(e,st,sp) ((e->ud + (e->du - e->ud) * st / 3)*sp)
#else
//# define StressDur(e,st,sp) (st,((e->du + e->ud)/2)*sp)
# define StressDur(e,st) ((e->du + e->ud)/2)
#endif



/// Append Holmes-elements for phones in (phonestr) to (eltq).
/// Returns ps->t.
unsigned phone_to_elm(phtoelm_state_t *ps, char *phonestr, dsqueue_t *eltq) {

#ifdef PHOLMES_DEBUG
  post("phone_to_elm(): called with phonestr='%s'", phonestr);
#endif

  ps->s = phonestr;
  while (ps->s && *ps->s) {
    pte_eltseq_str es = trie_lookup(&phtoelm, &(ps->s));
    if (es) {
     int n = *es++;
     while (n-- > 0) {
       int eid = *es++;             // -- index of sequence-element in Elements[]
       holmes_qelt_t he;            // -- encoded Holmes-triple for output-queue
       Elm_ptr ep = &Elements[eid]; // -- pointer to actual current element
       int dur;                     // -- placeholder for element duration

       //
       // This works because only vowels have ud != du,
       //  and we set stress just before a vowel
       //
       if (!(ep->feat & vwl)) ps->stress = 0;
       dur = StressDur(ep,ps->stress);
       he = hqeNew(eid,dur,ps->stress);

       // append the encoded element to the output queue
       dsqueue_append(eltq, (void *)he);

#ifdef PHOLMES_DEBUG
       post("phone_to_elm(): enqueued Holmes-triple %s,%d,%d", ep->name,dur,ps->stress);
#endif
     }
    }
    else {
     char ch = *(ps->s++);
     switch (ch) {
     case '\'':                // Primary stress
       ps->stress = 3;
       break;
     case ',':                 // Secondary stress
       ps->stress = 2;
       break;
     case '+':                 // Tertiary stress
       ps->stress = 1;
       break;
     case '-':                 // hyphen in input
       break;
     case '.':                 // literal dot indicates end-of-utterance
       dsqueue_append(eltq, (void *)hqeNew(0,0,0));
       break;
     default:
       {
	 fprintf(stderr,
		 "phone_to_elm(): ignoring unknown character '%c'\n",
		 ch);
	 break;
       }
     }
    }
  }
  return ps->t;
}
