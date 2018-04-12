/*
 *
 * Copyright 1994 by Nick Ing-Simmons.  All rights reserved.
 *
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if STDC_HEADERS
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#else          
#ifndef HAVE_STRCHR          
#define strchr index          
#define strrchr rindex          
#endif

#ifndef STRCHR_IS_MACRO
char *strchr (), *strrchr ();
#endif

#ifndef HAVE_MEMCPY          
#define memcpy(d, s, n) bcopy ((s), (d), (n))          
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#endif /* STDC_HEADERS */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_LIBC_H
/* From NeXT stuff */
#include <libc.h>
#endif

