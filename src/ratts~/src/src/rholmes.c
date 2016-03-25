/* $Id: rholmes.c,v 1.13 2003/01/03 13:35:13 moocow Exp $ 
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * - 2002 : modified by Bryan Jurish <moocow@ling.uni-potsdam.de>
 *    + added dynamic queueing, removed statics and globals.
 */
char *holmes_id = "$Id: rholmes.c,v 1.13 2003/01/03 13:35:13 moocow Exp $";

#include <stdio.h>
#include <ctype.h>
#include <useconfig.h>
#include <math.h>

#include "dsqueue.h"
#include "proto.h"
#include "parwave.h"
#include "elements.h"
#include "rholmes.h"
#include "phfeat.h"

#ifdef HOLMES_DEBUG
# ifndef _RATTS_M_PD_H
#  include <m_pd.h>
# endif
#endif

#if 1
# define AMP_ADJ 14
#else
# define AMP_ADJ 0
#endif

/* -- *really* default parameters -- */
static klatt_frame_t default_klatt_parameters =
{
#include "pars.def"
};

static float filter PROTO((holmes_filter_ptr p, float v));

static float
filter(p, v)
holmes_filter_ptr p;
float v;
{
 return p->v = (p->a * v + p->b * p->v);
}

/* 'a' is dominant element, 'b' is dominated
   ext is flag to say to use external times from 'a' rather
   than internal i.e. ext != 0 if 'a' is NOT current element.

 */
static void set_trans PROTO((holmes_global_t *hg, holmes_slope_t * t, Elm_ptr a, Elm_ptr b, int ext, int e));

static void
set_trans(hg, t, a, b, ext, e)
holmes_global_t *hg;
holmes_slope_t *t;
Elm_ptr a;
Elm_ptr b;
int ext;
int e;
{
 int i;
 for (i = 0; i < nEparm; i++)
  {
   t[i].t = ((ext) ? a->p[i].ed : a->p[i].id) * hg->speed;
   if (t[i].t)
    t[i].v = a->p[i].fixd + (a->p[i].prop * b->p[i].stdy) * (float) 0.01;
   else
    t[i].v = b->p[i].stdy;
  }
}

static float linear PROTO((float a, float b, int t, int d));

/*              
   ______________ b
   /
   /
   /
   a____________/                 
   0   d
   ---------------t---------------
 */

static float
linear(a, b, t, d)
float a;
float b;
int t;
int d;
{
 if (t <= 0)
  return a;
 else if (t >= d)
  return b;
 else
  {
   float f = (float) t / (float) d;
   return a + (b - a) * f;
  }
}

static float interpolate PROTO((char *w, char *p, holmes_slope_t * s, holmes_slope_t * e, float mid, int t, int d));

static float
interpolate(w, p, s, e, mid, t, d)
char *w;
char *p;
holmes_slope_t *s;
holmes_slope_t *e;
float mid;
int t;
int d;
{
 float steady = d - (s->t + e->t);
#ifdef HOLMES_DEBUG_INTERPOLATE
 fprintf(stdout, "%4s %s s=%g,%d e=%g,%d m=%g,%g\n",
         w, p, s->v, s->t, e->v, e->t, mid, steady);
#endif
 if (steady >= 0)
  {
   /* Value reaches stready state somewhere ... */
   if (t < s->t)
    return linear(s->v, mid, t, s->t);	/* initial transition */
   else
    {
     t -= s->t;
     if (t <= steady)
      return mid;                 /* steady state */
     else
      return linear(mid, e->v, (int) (t - steady), e->t);
     /* final transition */
    }
  }
 else
  {
   float f = (float) 1.0 - ((float) t / (float) d);
   float sp = linear(s->v, mid, t, s->t);
   float ep = linear(e->v, mid, d - t, e->t);
   return f * sp + ((float) 1.0 - f) * ep;
  }
}


/*----------------------------------------------------------------------
 * Holmes module global initialization.
 */
void holmes_init_global(holmes_global_t *hg) {
  hg->speed = 1.0;
  hg->frac  = 1.0;
  memcpy(&(hg->def_pars), &default_klatt_parameters, sizeof(klatt_frame_t));

  hg->stress_st = 40;
  hg->stress_et = 40;

  // -- rsynth defaults:
  //hg->f0decl = 0.5;
  //hg->topc  = 1.1;
  //hg->basec = 0.8;

  // -- flat prosody defaults
  hg->f0decl = 0.0;
  hg->topc  = 1.0;
  hg->basec = 1.0;
}

/*----------------------------------------------------------------------
 * Once-off Holmes state initialization
 */
void holmes_init_state(holmes_state_t *hs) {
  hs->flags = 0;
  hs->eltq = dsqueue_new(HOLMES_QUEUE_BLOCKSIZE);
}


/*----------------------------------------------------------------------
 * Holmes module per-utterance state initialization
 */
void holmes_init_utterance(holmes_global_t *hg, holmes_state_t *hs) {
  int j;
  hs->le = &Elements[0];  // literal insertion of "END" element
  hs->le = &Elements[1];  // literal insertion of "Q" element
  hs->tstress = 0;
  hs->ntstress = 0;
  hs->top = hg->topc * hg->def_pars.F0hz10;
  hs->pars = hg->def_pars;
  hs->pars.FNPhz = hs->le->p[fn].stdy;
#if 0
 hs->pars.F4hz = 3500;
#endif
 hs->pars.B4phz = hg->def_pars.B4phz;

 // Set stress attack/decay slope
 hs->stress_s.t = hg->stress_st;
 hs->stress_e.t = hg->stress_et;
 hs->stress_e.v = 0.0;

 for (j = 0; j < nEparm; j++) {
   hs->flt[j].v = hs->le->p[j].stdy;
   hs->flt[j].a = hg->frac;
   hs->flt[j].b = (float) 1.0 - (float) hg->frac;
 }

 hs->ce = &Elements[0];
 hs->t = 0;
}

/*----------------------------------------------------------------------
 * Holmes module global destruction (currently does nothing)
 */
void holmes_free_global(holmes_global_t *hg) {
  return;
}

/*----------------------------------------------------------------------
 * Holmes module : clear state data.
 */
void holmes_clear_state(holmes_state_t *hs) {
  while (!dsqueue_empty(hs->eltq)) {
    dsqueue_pop(hs->eltq);
  }
}

/*----------------------------------------------------------------------
 * Holmes module state destruction (once-off)
 */
void holmes_free_state(holmes_state_t *hs) {
  holmes_clear_state(hs);
  dsqueue_destroy(hs->eltq);
  hs->eltq = NULL;
}


/*----------------------------------------------------------------------
 * Pop the next elment from the queue onto the state-struct.
 */
void holmes_pop_next_elt(holmes_state_t *hs) {
  hs->t = 0;  // reset time counter on new element
  if (dsqueue_empty(hs->eltq)) {
    // -- empty queue : set 'ce' to "Q"
    if (hs->ce) hs->le = hs->ce;
    hs->ce  = &Elements[1];
    hs->dur = hs->ce->du;
  } else {
    holmes_qelt_t c_elt = (holmes_qelt_t)dsqueue_shift(hs->eltq);
    if (hs->ce) hs->le = hs->ce;
    hs->ce  = &Elements[hqeGetEID(c_elt)];
    hs->dur = hqeGetDur(c_elt);
    // -- ignore stress
  }
}


/*----------------------------------------------------------------------
 * Compute the next klatt_frame_t in hs->pars.
 */
void holmes_compute_next_frame(holmes_global_t *hg, holmes_state_t *hs)
{
  holmes_qelt_t n_elt;
  dsqueue_iter_t dsqi;
  int j;

  while (hs->t >= hs->dur || !hs->ce || hs->ce == &Elements[0]) {
    //
    // try and pop the next element from the queue
    //  + skip zero length elements which are only there to affect
    //    boundary values of adjacent elements
    //
    holmes_pop_next_elt(hs);
    if (!(hs->flags&HOLMES_FLAG_EOU) && (!hs->ce || hs->ce == &Elements[0])) {
      // -- 'END' element: it's a whole new utterance
      hs->flags |= HOLMES_FLAG_EOU;
#ifdef HOLMES_DEBUG
      post("holmes_compute_next_frame(): initializing new utterance.");
#endif
      holmes_init_utterance(hg,hs);
    }
  }

#ifdef HOLMES_DEBUG
  post("holmes_compute_next_frame(): got le=%s, ce=%s.",
       hs->le ? hs->le->name : "NULL",
       hs->ce ? hs->ce->name : "NULL");
#endif

  dsqi = dsqueue_iter_first(hs->eltq);
  n_elt = (holmes_qelt_t)dsqueue_iter_data(dsqi);
  hs->ne = &Elements[hqeGetEID(n_elt)]; // -- 'ne' will be (END,0,0) on empty queue
  
  if (hs->ce->rk > hs->le->rk) {
    // we dominate last
    set_trans(hg, hs->start, hs->ce, hs->le, 0, 's');
  } else {
    // last dominates us
    set_trans(hg, hs->start, hs->le, hs->ce, 1, 's');
  }
       
  if (hs->ne->rk > hs->ce->rk) {
    // next dominates us
    set_trans(hg, hs->end, hs->ne, hs->ce, 1, 'e');
  } else {
    // we dominate next
    set_trans(hg, hs->end, hs->ce, hs->ne, 0, 'e');
  }


  // -- [begin t++,tstress++ for-loop]
  //for (hs->t = 0; hs->t < dur; t++, tstress++) {
  if (hs->t == 0) {
    hs->base = hs->top * hg->basec; // 3 * top / 5
  }
    
  if (hs->tstress == hs->ntstress) {
    //unsigned j = i;
    dsqi = dsqueue_iter_next(hs->eltq, dsqi);
    hs->stress_s = hs->stress_e;
    hs->tstress = 0;
    hs->ntstress = hs->dur;
#  ifdef DEBUG_STRESS
    printf("Stress %g -> ", hs->stress_s.v);
#  endif
    do {
      holmes_qelt_t ep = (holmes_qelt_t)dsqueue_iter_data(dsqi);
      Elm_ptr  e;
      unsigned du = 0;
      unsigned s = 0;

      e  = &Elements[hqeGetEID(ep)];
      du = hqeGetDur(ep);
      s  = ep ? hqeGetStr(ep) : 3;

      if (s || e->feat & vwl) {
	unsigned d = 0;
	if (s)
	  hs->stress_e.v = (float) s / 3;
	else
	  hs->stress_e.v = (float) 0.1;
	do {
	  d += du;
#        ifdef DEBUG_STRESS
	  printf("%s", (e && e->dict) ? e->dict : "");
#        endif
	  dsqi = dsqueue_iter_next(hs->eltq,dsqi);
	  ep = (holmes_qelt_t)dsqueue_iter_data(dsqi);

	  e = &Elements[hqeGetEID(ep)];
	  du = hqeGetDur(ep);

	} while ((e->feat & vwl) && ep && s == hqeGetStr(ep));
	hs->ntstress += d / 2;
	break;
      }
      hs->ntstress += du;
    } while (dsqueue_iter_valid(hs->eltq, (dsqi = dsqueue_iter_next(hs->eltq,dsqi))));
#  ifdef DEBUG_STRESS
    printf(" %g @ %d\n", hs->stress_e.v, hs->ntstress);
#  endif
  }

  for (j = 0; j < nEparm; j++) {
    hs->tp[j] = filter(hs->flt + j, interpolate(hs->ce->name,
						Ep_name[j],
						&(hs->start[j]),
						&(hs->end[j]),
						(float) hs->ce->p[j].stdy,
						hs->t,
						hs->dur));
  }

  // Now generate a frame for the synth (*finally*)
  hs->pars.F0hz10 =
    hs->base + (hs->top - hs->base) *
    interpolate("",
		"f0",
		&(hs->stress_s),
		&(hs->stress_e),
		(float) 0,
		hs->tstress,
		hs->ntstress);
  
  hs->pars.AVdb = hs->pars.AVpdb = hs->tp[av];
  hs->pars.AF = hs->tp[af];
  hs->pars.FNZhz = hs->tp[fn];
  hs->pars.ASP = hs->tp[asp];
  hs->pars.Aturb = hs->tp[avc];
  hs->pars.B1phz = hs->pars.B1hz = hs->tp[b1];
  hs->pars.B2phz = hs->pars.B2hz = hs->tp[b2];
  hs->pars.B3phz = hs->pars.B3hz = hs->tp[b3];
  hs->pars.F1hz = hs->tp[f1];
  hs->pars.F2hz = hs->tp[f2];
  hs->pars.F3hz = hs->tp[f3];
  // AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
  //  Needs to be fixed properly in tables
  
  //hs->pars.ANP  = AMP_ADJ + hs->tp[an];
  hs->pars.AB = AMP_ADJ + hs->tp[ab];
  hs->pars.A5 = AMP_ADJ + hs->tp[a5];
  hs->pars.A6 = AMP_ADJ + hs->tp[a6];
  hs->pars.A1 = AMP_ADJ + hs->tp[a1];
  hs->pars.A2 = AMP_ADJ + hs->tp[a2];
  hs->pars.A3 = AMP_ADJ + hs->tp[a3];
  hs->pars.A4 = AMP_ADJ + hs->tp[a4];
  
  //parwave(&(synth->klatt_global), &pars, samp);
  //samp += synth->klatt_global.nspfr;
  
  if (hs->ce != &Elements[1] || hs->le != &Elements[1]) {
    // -- only alter f0 declination if we actually have something to say...
    // Declination of f0 envelope
    hs->top -= hg->f0decl;
  }

  // -- [end t++,tstress++ for-loop]
  hs->t++;
  hs->tstress++;
  //le = ce;
}
