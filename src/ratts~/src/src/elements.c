/* $Id: elements.c,v 1.2 2002/12/24 02:22:24 moocow Exp $
 */

/*
 * Copyright (c) 1994 Nick Ing-Simmons.  All Rights Reserved.
 */

char *elements_id = "$Id: elements.c,v 1.2 2002/12/24 02:22:24 moocow Exp $";
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <math.h>

#include "proto.h"
#include "elements.h"
#include "phfeat.h"

Elm_t Elements[] =
{
#include "Elements.def"
};

unsigned num_Elements = (sizeof(Elements) / sizeof(Elm_t));

char *Ep_name[nEparm] =
{
 "fn", "f1", "f2", "f3",
 "b1", "b2", "b3", "an",
 "a1", "a2", "a3", "a4",
 "a5", "a6", "ab", "av",
 "avc", "asp", "af"
};
