/*
 * Copyright (c) 1994 Nick Ing-Simmons.  All Rights Reserved.
 */

#ifndef _RATTS_PHFEAT_H
#define _RATTS_PHFEAT_H

/**
 * \file phfeat.h
 * \brief Phonetic feature definitions.
 * \todo It would be really nice to know how these are used.
 */

/// alveolar (ex: T,TY,TZ,D)
#define alv 0x00000001

/// approximate (ex: H,R,W,Y)
#define apr 0x00000002

/// back [vowels] (ex: U,O,OO)
#define bck 0x00000004

/// bilabial (ex: P,PY,PZ,B,M)
#define blb 0x00000008

/// central [vowels] (ex: A,ER,IE,OU,OA)
#define cnt 0x00000010

/// dental (ex: TH,DH,DI)
#define dnt 0x00000020

/// front [vowels] (ex: I,E,AA,EE,AI,IA,IB,AIR)
#define fnt 0x00000040

/// fricative (ex: F,S,SH,X,V,QQ,DH,DI)
#define frc 0x00000080

/// glottal (ex: H)
#define glt 0x00000100

/// high [vowels] (ex: EE,UU)
#define hgh 0x00000200

/// lateral (ex: L,LL)
#define lat 0x00000400

/// labiodental (ex: F,V)
#define lbd 0x00000800

/// labiovelar (?) (ex: W)
#define lbv 0x00001000

/// low-mid [vowels] (ex: E,U,ER,AW,AI,AIR,OR)
#define lmd 0x00002000

/// low [vowels] (ex: AA,O,AR,IE,OU,IB)
#define low 0x00004000

/// middle [vowels] (ex: A,OA)
#define mdl 0x00008000

/// nasal (ex: M,N,NG)
#define nas 0x00010000

/// palatal (ex: Y)
#define pal 0x00020000

/// ??? (ex: SH,ZH,CI,JY)
#define pla 0x00040000

/// round [vowels] (ex: O,OO,AW,UU,OI,OV,OOR,OR)
#define rnd 0x00080000

/// retroflex (?) (ex: RX)
#define rzd 0x00100000

/// ??? [vowels] (ex: I,OO,OV,IA,OOR)
#define smh 0x00200000

/// stop (ex: P,PY,PZ,T,TY,TZ,K,KY,KZ,B,BY,BZ,D,DY,DZ,G,GY,GZ,CH,J)
#define stp 0x00400000

/// ??? [vowels] (ex: OI)
#define umd 0x00800000

/// ??? [vowels] (ex: I,E,AA,U,A,EE,ER,AR,AI,IE,OU,OA,IA,IB,AIR)
#define unr 0x01000000

/// voiced (ex: B,BY,BZ,D,DY,DZ,G,GY,GZ,V,QQ,DH,DI,Z,ZZ,ZH,J,JY,L,LL,W,Y)
#define vcd 0x02000000

/// velar (ex: K,KY,KZ,G,GY,GZ,NG,X)
#define vel 0x04000000

/// ??? voiceless ??? (ex: P,PY,PZ,T,TY,TZ,K,KY,KZ,F,TH,S,SH,X,CH,CI)
#define vls 0x08000000

/// vowel (ex: I,E,AA,U,O,OO,A,EE,ER,AR,AW,UU,AI,IE,OI,OU,OV,OA,IA,IB,AIR,OOR,OR)
#define vwl 0x10000000


/// phonetic-feature names
#define Fnames {\
  "alv","apr","bck","blb","cnt","dnt","fnt","frc", \
  "glt","hgh","lat","lbd","lbv","lmd","low","mdl", \
  "nas","pal","pla","rnd","rzd","smh","stp","umd", \
  "unr","vcd","vel","vls","vwl",NULL}

/// number of features
#define num_Features 29

/// feature values by feature-index
extern long unsigned int Features[];

/// feature names by feature index
extern char *FeatureNames[];

#endif /* _RATTS_PHFEAT_H */

