/*
 * $Id: suspect.c,v 1.1 2003/01/10 02:29:14 moocow Exp $
 *
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 * - 01/2003 : modified by Bryan Jurish <moocow@ling.uni-potsdam.de>
 *    + removed many functions [xlate_*()]
 *    + adapted to PD
 */

#include <ctype.h>
#include <useconfig.h>

#include "suspect.h"

char
suspect_word(s, n)
char *s;
int n;
{
 int i = 0;
 char seen_lower = 0;
 char seen_upper = 0;
 char seen_vowel = 0;
 char last = 0;
 for (i = 0; i < n; i++)
  {
   char ch = *s++;
   if (i && last != '-' && isupper(ch))
    seen_upper = 1;
   if (islower(ch))
    {
     seen_lower = 1;
     ch = toupper(ch);
    }
   if (ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' || ch == 'Y')
    seen_vowel = 1;
   last = ch;
  }
 return !seen_vowel || (seen_upper && seen_lower) || !seen_lower;
}
