/* $Id: rholmes.h,v 1.11 2003/01/03 13:35:13 moocow Exp $
 */

/*
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 * - 12/2002 : modified by Bryan Jurish <moocow@ling.uni-potsdam.de>
 *    + added dynamic queueing, removed statics and globals.
 */

#ifndef _RATTS_RHOLMES_H
#define _RATTS_RHOLMES_H

#include "dsqueue.h"
#include "elements.h"
#include "parwave.h"

/**
 * \file rholmes.h
 * \brief Part of Nick's implementation of a phoneme to "vocal tract parameters" system
 * described by J. N. Holmes, I. Mattingly, and J. Shearme, "Speech synthesis by rule".
 * Language and Speech 7, pp 127--143. 1964.
 */

/* -- default block size for element-queue -- */
#define HOLMES_QUEUE_BLOCKSIZE 32

/*-------------------------------------------------------------------
 * typedefs
 *-------------------------------------------------------------------*/

/// Slope structure for Holmes transition computation.
typedef struct
{
  float v;                        ///< boundary value
  int t;                          ///< transition time
} holmes_slope_t;


/// Parameter-transition structure.
typedef struct
{
  holmes_slope_t p[nEparm];   ///< Transition slopes for each element-parameter (see enum Eparm_e).
} holmes_trans_t;

/// "filter" structure (I don't believe Holmes mentions this...)
typedef struct
{
  float a;
  float b;
  float v;
} holmes_filter_t, *holmes_filter_ptr;


/// Global Holmes-parameter structure
typedef struct {
  double         speed;                  ///< Speed coefficient, 1.0 is "normal"
  double         frac;                   ///< Parameter filter 'fraction' (default=1.0)
  int            stress_st;              ///< Default transition time for stress_s
  int            stress_et;              ///< Default transition time for stress_e
  klatt_frame_t  def_pars;               ///< Default initial Klatt-frame
  float          f0decl;                 ///< Declination of f0 envelope in Hz/cS (default=0.5)
  float          topc;                   ///< moo: coefficient used to prosodic peak from f0
  float          basec;                  ///< moo: coefficient used to prosodic base from top
  //long           f0offset;               ///< moo: adjustment offset for f0 in Hz
} holmes_global_t, *holmes_global_ptr;


/**
 * Holmes element encoding for queueing.
 * A Holmes-element is a triple (ElementID,Duration,Stress).
 *  + "ElementID" is the index of the element in Elements[].
 *  + "Duration" is the element's duration in frames.
 *  + "Stress" is a relative stress-marker (for vowels).
 * Each component is encoded into one byte of an 'unsigned int':
 * we use the least significant byte for 'ElementID', the
 * 2nd least significant byte for 'Duration', and the 3rd
 * least significant byte for 'Stress' -- your 'unsigned int's
 * must therefore be at least 24 bits wide in order for this
 * to work...
 */
typedef unsigned int holmes_qelt_t;
typedef unsigned int *holmes_qelt_ptr;

/// Holmes-element constructor
#define hqeNew(eid,dur,stress) ((eid&0xff)|((dur&0xff)<<8)|((stress&0xff)<<16))

/// elementID accessor for holmes_qelt_t encoding
#define hqeGetEID(e) (e&0xff)

/// Duration accessor for holmes_qelt_t encoding
#define hqeGetDur(e) ((e>>8)&0xff)

/// Stress accessor for holmes_qelt_t encoding
#define hqeGetStr(e) ((e>>16)&0xff)

/// elementID lvalue workaround for holmes_qelt_t encoding
#define hqeSetEID(e,v) e &= ~0xff; e |= (v&0xff)

/// Duration lvalue workaround for holmes_qelt_t encoding
#define hqeSetDur(e,v) e &= ~0xff00; e |= (v&0xff)<<8;

/// Stress lvalue workaround for holmes_qelt_t encoding
#define hqeSetStr(e,v) e &= ~0xff0000; e |= (v&0xff)<<16;


/// constants for holmes_state_t 'flags'

// -- end-of-utterance detected
#define HOLMES_FLAG_EOU  1

// -- end-of-word detected (unused)
#define HOLMES_FLAG_EOW  2



/// State variables for Holmes algorithm
typedef struct {
  dsqueue_t        *eltq;          ///< Queue of waiting 'holmes_qelt_t's
  holmes_filter_t   flt[nEparm];   ///< filters (???)
  klatt_frame_t     pars;          ///< last computed Klatt-frame
  Elm_ptr           le;            ///< last element
  unsigned          tstress;       ///< ???
  unsigned          ntstress;      ///< ???
  holmes_slope_t    stress_s;      ///< ???
  holmes_slope_t    stress_e;      ///< ???
  
  float             top;           ///< moo: top of f0-prosodic countour
  unsigned          flags;         ///< moo: state flags (HOLMES_FLAG_*) above

  // --- below here, the state variables don't need initialization
  Elm_ptr           ce;            ///< current element
  Elm_ptr           ne;            ///< next element
  unsigned          dur;           ///< duration of current element (frames)
  holmes_slope_t    start[nEparm]; ///< filter (?)
  holmes_slope_t    end[nEparm];   ///< filter (?)
  unsigned          t;             ///< time-counter for current element (frames)
  float             base;          ///< prosodic base freq (?)
  float             tp[nEparm];    ///< ??
} holmes_state_t, *holmes_state;


/*-------------------------------------------------------------------
 * functions : initialization / destruction
 *-------------------------------------------------------------------*/
/// Holmes module global initialization.
extern void holmes_init_global(holmes_global_t *hg);


/// Holmes module global destruction (currently does nothing)
extern void holmes_free_global(holmes_global_t *hg);

/// Holmes module per-utterance state destruction.
/// Frees element-queue, but not 'hs' itself.
extern void holmes_free_state(holmes_state_t *hs);


/// Holmes module once-off state initialization.
extern void holmes_init_state(holmes_state_t *hs);

/// Holmes module per-utterance state initialization.
extern void holmes_init_utterance(holmes_global_t *hg, holmes_state_t *hs);

/// Holmes module : clear state data (can be called more than once).
void holmes_clear_state(holmes_state_t *hs);



/*-------------------------------------------------------------------
 * functions : guts
 *-------------------------------------------------------------------*/

/**
 * \brief Phoneme to "vocal tract parameters" conversion: compute
 *        the next klatt_frame_t in hs->pars.
 * @param hg Holds global flags and structures for conversion.
 * @param hs Holds conversion state.
 */
extern void holmes_compute_next_frame(holmes_global_t *hg, holmes_state_t *hs);

/// Get the next element from the queue
extern void holmes_get_next_elt(holmes_state_t *hs);


#endif /* _RATTS_RHOLMES_H */
