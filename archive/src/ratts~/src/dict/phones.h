/*
 *
 * Copyright (c) 1994 by Nick Ing-Simmons.  All Rights Reserved.
 *
 */

#ifndef _RATTS_PHONES_H
#define _RATTS_PHONES_H


/// PHONE(NAME, BritishSpelling, AmericanSpelling, Examples)
/// used in enum phone_e to enumerate all known phones.
#define PHONE(nm,br,am,ex) nm,
enum phone_e {
  SIL,
#include "phones.def"
  END
};
#undef PHONE

/// phoneset variable: this should be structure-ized.
extern char *ph_name[];

/// phoneset variable (british): this should be structure-ized.
extern char *ph_br[];

/// phoneset variable (american): this should be structure-ized.
extern char *ph_am[];

#endif /* _RATTS_PHONES_H */
