/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: ratts.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD : top-level library
 *
 *     A PD external derived from Nick Ing-Simmons' "rsynth" text-to-speech
 *     program.
 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/

#ifndef _RATTS_M_PD_H
# include <m_pd.h>
# define _RATTS_M_PD_H
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

#include "rattshash.h"

/*=====================================================================
 * Dummy object for global help
 *=====================================================================*/
t_class *ratts_class;
typedef struct ratts
{
  t_object t_ob;
} t_ratts;

void *ratts_new(void)
{
  t_ratts *x = (t_ratts *)pd_new(ratts_class);
  return (void *)x;
}


/*=====================================================================
 * External setup routines
 *=====================================================================*/
extern void klatt_frame_setup(void);
extern void klatt_tilde_setup(void);

extern void pd_holmes_setup(void);
extern void holmes_setup(void);
extern void holmes_feat_setup(void);
extern void holmes_mask_setup(void);

extern void pd_phtoelm_setup(void);
extern void phones2holmes_setup(void);

extern void guessphones_setup(void);
extern void number2text_setup(void);

extern void rattstok_setup(void);
extern void toupper_setup(void);

extern void spellout_setup(void);

/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void ratts_setup(void) {
  post("");
  //post("ratts: Text-to-Speech Synthesis External Library v%s by Bryan Jurish", PACKAGE_VERSION);
  post("ratts: Realtime Analog Text-To-Speech externals v%s by Bryan Jurish",
       PACKAGE_VERSION);
  post("ratts: Based on text-to-speech code by Nick Ing-Simmons and Jon Iles");
  post("ratts: and PD external code by Orm Finnendahl and Travis Newhouse");
  post("ratts: compiled by %s on %s", PACKAGE_COMPILED_BY, PACKAGE_COMPILED_ON);

  // -- setup our library's externals
  klatt_frame_setup();
  klatt_tilde_setup();
  pd_holmes_setup();
  holmes_setup();
  holmes_feat_setup();
  holmes_mask_setup();
  pd_phtoelm_setup();
  phones2holmes_setup();
  guessphones_setup();
  number2text_setup();
  rattshash_setup();
  toupper_setup();
  spellout_setup();
  rattstok_setup();

  post("");

  ratts_class = class_new(gensym("ratts"), ratts_new, 0, sizeof(t_ratts), CLASS_NOINLET, 0);

  class_sethelpsymbol(ratts_class, gensym("ratts-help.pd"));
}
