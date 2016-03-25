/* $Id: elements.h,v 1.4 2002/12/24 11:23:31 moocow Exp $
*/

/*
 * Copyright (c) 1994 Nick Ing-Simmons.  All Rights Reserved.
 *
 * 12/2002 : modified by moocow
 *   - added some doxygen comments
 */

/**
 * \file elements.h
 * \brief Part of Nick's implementation of a phoneme to "vocal tract parameters" system
 * described by Holmes et. al. (1964).
 */

#ifndef _RATTS_ELEMENTS_H
#define _RATTS_ELEMENTS_H

/// The guts of the "input tables" from Holmes et. al. (1964).
typedef struct
{
  /// steady-state frequency (?)
  float stdy;

  /// "fixed contribution" to the boundary value (?)
  float fixd;

  /// proportion of the steady-state value of the adjacent element
  /// which is added to the fixed contribution to derive the boundary value (?)
  char  prop;

  /// Duration of the external transition (?)
  char  ed;

  /// Duration of the internal transition (?)
  char  id;
 } interp_t, *interp_ptr;


/// Element-parameter enum (?)
enum Eparm_e
 {
  fn, f1, f2, f3, b1, b2, b3, an, a1, a2, a3, a4, a5, a6, ab, av, avc, asp, af,
  nEparm
 };


/// Element-parameter identifiers (?)
extern char *Ep_name[nEparm];

/// Element structure
typedef struct Elm_s
{
  char *name;         ///< Element name.
  char rk;            ///< Element rank: used to determine dominance for transitions.
  char du;            ///< Standard duration of the element (frames)
  char ud;            ///< Duration of the element (frames) in unstressed position (vowels only)
  unsigned char font; ///< what the?
  char  *dict;        ///< probably "dictionary", but why?
  char  *ipa;         ///< probably ascii-fied phonetic represenation, but why?
  long  feat;         ///< bitmask of phonetic features
  interp_t p[nEparm]; ///< the guts: parameter delta-specifications.
 } Elm_t, *Elm_ptr;

/// global: known elements (see Elements.def).
extern Elm_t Elements[];

/// global: number of known elements.
extern unsigned num_Elements;

#endif /* _RATTS_ELEMENTS_H */
