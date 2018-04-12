/*
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 * 01/2003: modified by moocow
 *   + outputs raw text of word-to-phonestring mappings
 *     rather than a gdbm db.
 *   + USAGE: mkdicttxt [DIALECT [INFILE [OUTFILE]]]
 *     - where DIALECT is either 'a' (american) or 'b' (british)
 *
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "proto.h"
#include <stdio.h>
#include <ctype.h>

#include "useconfig.h"

#include "trie.h"
#include "darray.h"
#include "phones.h"


/*
 * Globals
 */
// dialect: one of 'ph_am' or 'ph_br'
char **dialect = NULL;

trie_ptr phones;

static void enter_phones PROTO((void));
static void 
enter_phones()
{
 int i;
 char *s;
 for (i = 1; (s = ph_name[i]); i++)
  trie_insert(&phones, s, (void *) i);
}



static void enter_words PROTO((FILE *fin, FILE *fout));
static void
enter_words(fin, fout)
FILE *fin;
FILE *fout;
{
 char buf[4096];
 while (fgets(buf, sizeof(buf), fin))
  {
   char *s = buf;
   char *h = strchr(s, '#');
   if (h)
    *h = '\0';
   while (isspace(*s))
    s++;
   if (*s)
    {
     char *p = s;
     while (isalpha(*p) || *p == '-' || *p == '\'')
      {
       if (islower(*p))
        *p = toupper(*p);
       p++;
      }
     if (isspace(*p))
      {
       char codes[4096];
       char *d = codes;
       int ok = 1;
       char *key = s;
       unsigned keysize = p - s;

       while (*p && ok)
        {
         unsigned code;
         while (isspace(*p))
          p++;
         if (*p)
          {
           char *e = p;
           while (isalpha(*e) || *e == '1' || *e == '2')
            {
             if (islower(*e))
              *e = toupper(*e);
             e++;
            }
           if (*e == '0')
            *e++ = ' ';
           if (e > p && (code = (unsigned) trie_lookup(&phones, &p)))
            *d++ = code;
           else
            {
             fprintf(stderr, "Bad code %.*s>%s", (int)(p - s), s, p);
             ok = 0;
             break;
            }
          }
        }
       if (ok)
        {
	  /*
	    datum data;
	    data.dptr = codes;
	    data.dsize = d - codes;
	    gdbm_store(db, key, data, GDBM_INSERT);
	  */
	  char *val = codes;
	  unsigned valsize = d - codes;
	  fwrite(key, sizeof(char), keysize, fout);
	  fputc('\t', fout);
	  //fwrite(val, sizeof(char), valsize, fout);
	  while (valsize-- > 0) {
	    //fprintf(fout, "%d", (int)*val++);  // output numeric phone-string character values
	    fputs(dialect[(int)*val++], fout);   // output dialect-specific phone-strings
	    //fputs(ph_name[(int)*val++], fout); // output abstract phone names
	    //if (valsize) fputc(' ', fout);     // separate phones with spaces
	  }
	  fputc('\n', fout);
        }
      }
     else
      {
       if (*p != '(')
        fprintf(stderr, "Ignore (%c) %s", *p, s);
      }
    }
  }
}

/*
 * USAGE: mkdicttxt < INFILE > OUTFILE
 */
int main PROTO((int argc, char *argv[], char *env[]));

int
main(argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
  FILE *fin = stdin, *fout = stdout;
  if (argc > 1) {
    if (!strcmp(argv[1], "a")) {
      dialect = ph_am;
    } else if (!strcmp(argv[1], "b")) {
      dialect = ph_br;
    } else {
      dialect = ph_br;
      fprintf(stderr, "%s: unknown dialect '%s' -- using 'br'\n", argv[0], argv[1]);
    }
  } else {
    fprintf(stderr, "Usage: %s DIALECT [INFILE [OUTFILE]]\n", argv[0]);
    fprintf(stderr, "  where DIALECT is one of:\n");  
    fprintf(stderr, "    a : american english\n");  
    fprintf(stderr, "    b : british english\n");
    exit(0);
  }

  if (argc > 2) {
    fin = fopen(argv[2], "r");
    if (!fin) {
      fprintf(stderr, "%s: could not open input file '%s': ", argv[0], argv[2]);
      perror(argv[2]);
      exit(2);
    }
    if (argc > 3) {
      fout = fopen(argv[3], "w");
      if (!fout) {
	perror(argv[3]);
	exit(3);
      }
    }
  }
  enter_phones();
  enter_words(fin, fout);
  
  if (fin != stdin) fclose(fin);
  if (fout != stdout) fclose(fout);

 return 0;
}
