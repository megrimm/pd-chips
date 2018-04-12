/*
 * $Id: saynum.c,v 1.1 2003/01/02 00:45:28 moocow Exp $
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: modified by moocow
 *   - removed numeric translators to their own file: 'saynum.h'
 *   - removed xlate_string() calls from numeric translators
 *     xlate_ordinal() and xlate_cardinal(): these now produce
 *     plain text for further processing.
 */
char *saynum_id = "$Id: saynum.c,v 1.1 2003/01/02 00:45:28 moocow Exp $";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "proto.h"
#include "darray.h"
#include "saynum.h"
#include "phoneutils.h"

/*
   **              Integer to Readable ASCII Conversion Routine.
   **
   ** Synopsis:
   **
   **      say_cardinal(value)
   **              long int     value;          -- The number to output
   **
   **      The number is translated into a string of words
   **
 */
static char *Cardinals[] =
{
 "zero", "one", "two", "three",
 "four", "five", "six", "seven",
 "eight", "nine",
 "ten", "eleven", "twelve", "thirteen",
 "fourteen", "fifteen", "sixteen", "seventeen",
 "eighteen", "nineteen"
};


static char *Twenties[] =
{
 "twenty", "thirty", "forty", "fifty",
 "sixty", "seventy", "eighty", "ninety"
};


static char *Ordinals[] =
{
 "zeroth", "first", "second", "third",
 "fourth", "fifth", "sixth", "seventh",
 "eighth", "ninth",
 "tenth", "eleventh", "twelfth", "thirteenth",
 "fourteenth", "fifteenth", "sixteenth", "seventeenth",
 "eighteenth", "nineteenth"
};


static char *Ord_twenties[] =
{
 "twentieth", "thirtieth", "fortieth", "fiftieth",
 "sixtieth", "seventieth", "eightieth", "ninetieth"
};

/*
 * Translate a number to text.  This version is for CARDINAL numbers.
 *       Note: this is recursive.
 */
unsigned xlate_cardinal(long int value, darray_ptr text)
{
 unsigned nph = 0;
 if (value < 0) {
   nph += phone_append_string(text, "minus ");
   value = (-value);
   if (value < 0) {
     // Overflow!  -32768
     nph += phone_append_string(text, "a lot");
     return nph;
   }
 }
 if (value >= 1000000000L)
  /* Billions */
  {
   nph += xlate_cardinal(value / 1000000000L, text);
   nph += phone_append_string(text, " billion");
   value = value % 1000000000;
   if (value == 0)
    return nph;                   /* Even billion */
   phone_append(text, ' ');
   nph++;
   if (value < 100)
    nph += phone_append_string(text, "and ");
   /* as in THREE BILLION AND FIVE */
  }
 if (value >= 1000000L)
  /* Millions */
  {
   nph += xlate_cardinal(value / 1000000L, text);
   nph += phone_append_string(text, " million");
   value = value % 1000000L;
   if (value == 0)
    return nph;                   /* Even million */
   phone_append(text, ' ');
   nph++;
   if (value < 100)
    nph += phone_append_string( text, "and ");
   /* as in THREE MILLION AND FIVE */
  }

 /* Thousands 1000..1099 2000..99999 */
 /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
 if ((value >= 1000L && value <= 1099L) || value >= 2000L)
  {
   nph += xlate_cardinal(value / 1000L, text);
   nph += phone_append_string(text, " thousand");
   value = value % 1000L;
   if (value == 0)
    return nph;                   /* Even thousand */
   phone_append(text, ' ');
   nph++;
   if (value < 100)
    nph += phone_append_string(text, "and ");
   /* as in THREE THOUSAND AND FIVE */
  }
 if (value >= 100L)
  {
   nph += phone_append_string(text, Cardinals[value / 100]);
   nph += phone_append_string(text, " hundred");
   value = value % 100;
   if (value == 0)
    return nph;                   /* Even hundred */
   phone_append(text, ' ');
   nph++;
  }
 if (value >= 20)
  {
   nph += phone_append_string(text, Twenties[(value - 20) / 10]);
   value = value % 10;
   if (value == 0)
    return nph;                   /* Even ten */
   phone_append(text, ' ');
   nph++;
  }
 nph += phone_append_string(text, Cardinals[value]);
 return nph;
}

/*
   ** Translate a number to phonemes.  This version is for ORDINAL numbers.
   **       Note: this is recursive.
 */
unsigned xlate_ordinal(long int value, darray_ptr text)
{
 unsigned nph = 0;
 if (value < 0)
  {
   nph += phone_append_string(text, "minus ");
   value = (-value);
   if (value < 0)                 /* Overflow!  -32768 */
    {
     nph += phone_append_string(text, "a lot");
     return nph;
    }
  }
 if (value >= 1000000000L)
  /* Billions */
  {
   nph += xlate_cardinal(value / 1000000000L, text);
   value = value % 1000000000;
   if (value == 0)
    {
     nph += phone_append_string(text, " billionth");
     return nph;                  /* Even billion */
    }
   nph += phone_append_string(text, " billion");
   phone_append(text, ' '); nph++;
   if (value < 100)
    nph += phone_append_string(text, "and ");
   /* as in THREE BILLION AND FIVE */
  }

 if (value >= 1000000L)
  /* Millions */
  {
   nph += xlate_cardinal(value / 1000000L, text);
   value = value % 1000000L;
   if (value == 0)
    {
     nph += phone_append_string(text, " millionth");
     return nph;                  /* Even million */
    }
   nph += phone_append_string(text, " million");
   phone_append(text, ' '); nph++;
   if (value < 100)
    nph += phone_append_string(text, "and ");
   /* as in THREE MILLION AND FIVE */
  }

 /* Thousands 1000..1099 2000..99999 */
 /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
 if ((value >= 1000L && value <= 1099L) || value >= 2000L)
  {
   nph += xlate_cardinal(value / 1000L, text);
   value = value % 1000L;
   if (value == 0)
    {
     nph += phone_append_string(text, " thousandth");
     return nph;                  /* Even thousand */
    }
   nph += phone_append_string(text, " thousand");
   phone_append(text, ' '); nph++;
   if (value < 100)
    nph += phone_append_string(text, "and ");
   /* as in THREE THOUSAND AND FIVE */
  }
 if (value >= 100L)
  {
   nph += phone_append_string(text, Cardinals[value / 100]);
   value = value % 100;
   if (value == 0)
    {
     nph += phone_append_string(text, " hundredth");
     return nph;                  /* Even hundred */
    }
   nph += phone_append_string(text, " hundred ");
  }
 if (value >= 20)
  {
   if ((value % 10) == 0)
    {
     nph += phone_append_string(text, Ord_twenties[(value - 20) / 10]);
     return nph;                  /* Even ten */
    }
   phone_append(text, ' '); nph++;
   nph += phone_append_string(text, Twenties[(value - 20) / 10]);
   phone_append(text, ' '); nph++;
   value = value % 10;
  }
 nph += phone_append_string(text, Ordinals[value]);
 return nph;
}


unsigned xlate_float(float value, darray_ptr text) {
  char buf[32];
  sprintf(buf,"%g",value);
  return xlate_float_string(buf,text);
}

unsigned xlate_float_string(char *value, darray_ptr text) {
  char ch, *s;
  unsigned nph = 0;
  for (s = value; (ch = *s) && ch != '.' && ch != 'e'; s++) ;
  if (s != value) {
    *s = 0;
    nph += xlate_cardinal(atol(value), text);
    *s = ch;
  }

  if (ch == '.') {
    nph += phone_append_string(text, " point");
    s++;
  }
  while ((ch = *s) && ch >= '0' && ch <= '9') {
    ch = s[1];
    s[1] = 0;
    phone_append(text, ' '); nph++;
    nph += xlate_cardinal(atol(s), text);
    *++s = ch;
  }
  if (ch == 'e') {
    s++;
    nph += phone_append_string(text, " times ten to the ");
    nph += xlate_ordinal(atol(s), text);
  }
  return nph;
}
