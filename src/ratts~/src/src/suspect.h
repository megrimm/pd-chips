/*
 * $Id: suspect.h,v 1.1 2003/01/10 02:29:14 moocow Exp $
 *
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 * - 01/2003 : modified by Bryan Jurish <moocow@ling.uni-potsdam.de>
 *    + removed many functions [xlate_*()]
 *    + adapted to PD
 */

#ifndef _RATTS_SUSPECT_H
#define _RATTS_SUSPECT_H

#include "proto.h"

/// returns true if the (n)-ary prefix of (s) is not very word-like.
extern char suspect_word PROTO((char *s,int n));

#endif /* _RATTS_SUSPECT_H */
