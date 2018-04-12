/*
 * spaces~ 	--space object for Pd
 *          --this code is a port of the space unit generator written
 *           by F. Richard Moore as part of the cmusic program in the
 *           CARL package
 *			--Originally ported as "space~" to PD by Shahrokh Yadegari March 2000.
 * 			--Forked and modified by maxus germanus 2011
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "m_pd.h"

//#if defined(PD)||defined(MSP)
#define REALTIME
//#endif 

#ifndef REALTIME
#define UNUSED	1
#define DBUF	2
#define DLEN	3
#define NOW	4
#define DARRAY	5
#define CARRAY	6
#define AARRAY	7
#define FARRAY	8
#define XS	9
#define YS	10
#define THETAS	11
#define AMPS	12
#define BACKS	13
#define X	14
#define Y	15
#define THETA   16
#define AMP     17
#define BACK    18
#else /* ifndef REALTIME */

/* UNIT GENERATOR DEFINITIONS */
#ifdef csound
#include <carl/sndio.h>
extern int sferror;
#endif /* csound */
#ifdef bicsf
#include <carl/snd.h>
extern int sferror;
#endif /* bicsf */
#ifdef REALTIME

#define	SP_DEFAULT_INNER	5.0
#define	SP_DEFAULT_OUTER	50.0
#define	SP_DEFAULT_QUAT		4
#define	SP_DEFAULT_STEREO	2
#define SPR_DEFAULT_TH		0.4
#define SPR_DEFAULT_CF		0.5

/* allow up to 64 acoustic surfaces and 64 speakers	*/
#define	SP_MAX_CHANNEL		64
#define	SP_MAX_SURFACE		4


typedef struct spaces_room {
	int		spr_NLs;				/* Number of listening surfaces */
	int   	spr_NAs;				/* Number of acoustic space surfaces */
	float	spr_Lx[SP_MAX_SURFACE];	/* Listening space x-coordinates */
	float	spr_Ly[SP_MAX_SURFACE];	/* Listening space y-coordinates */
	float	spr_Ax[SP_MAX_SURFACE];	/* Acoustic space x-coordinates */
	float	spr_Ay[SP_MAX_SURFACE];	/* Acoustic space y-coordinates */
	int		spr_ns;					/* number of speakers */
	float	spr_Sx[SP_MAX_CHANNEL];	/* Speaker x-coordinates */
	float	spr_Sy[SP_MAX_CHANNEL];	/* Speaker y-coordinates */
	float	spr_TH;					/* diffraction threshhold */
	float	spr_CF;					/* crossface factor */
	float	spr_lx1[SP_MAX_SURFACE];
	float	spr_ly1[SP_MAX_SURFACE];
	float	spr_lx2[SP_MAX_SURFACE];
	float	spr_ly2[SP_MAX_SURFACE];
	int		spr_horiz[SP_MAX_SURFACE], spr_vert[SP_MAX_SURFACE];
	float	spr_m[SP_MAX_SURFACE], spr_b[SP_MAX_SURFACE];
	int		spr_notfirst;
	double	spr_d[SP_MAX_SURFACE];
	double	spr_sint[SP_MAX_SURFACE];
	double	spr_cost[SP_MAX_SURFACE];
	double	spr_rho[SP_MAX_SURFACE];
	double	spr_sin2t[SP_MAX_SURFACE];
	double	spr_cos2t[SP_MAX_SURFACE];
	double	spr_f1[SP_MAX_SURFACE], spr_f2[SP_MAX_SURFACE];
} t_spaces_room;

typedef struct _spaces_tilde
{
	t_object sp_obj;
	t_float  sp_f;					/* save value for control to signal change */
	int	sp_off;						/* have we turned off space */
	t_float *sp_in;					/* the input buffer */
	t_float **sp_out;				/* output buffers */
	t_float *sp_grev;				/* global reverb */
	double	sp_srate;				/* sampling rate */
	int	sp_nchan;					/* number of channels */
	int	sp_vsize;					/* vector size  */
	int	sp_firstbuf;				/* processing the first buf after init? */
	int	sp_interpol;				/* to interpolate or not, that is the qstn */

	/* room information */
	t_spaces_room *sp_room;			/* room dimensions */
	int	sp_nir;						/* number of inner rooms defined */
	int	sp_curroom;					/* current room number */
	float	sp_direct;				/* direct attenuation */
	float	sp_reflect;				/* reflection attenuation */
	float	sp_innersize;			/* inner room size */
	float	sp_outersize;			/* outer room size */
	int	sp_realray;					/* if set real-time data for theta and back */
	float	sp_back;				/* back scalar value */
	float	sp_theta;				/* theta scalar value */
	t_float *DBUF;					/* delay buffer */
	long     DLEN;					/* delay buffer length */
	long     NOW;
	t_float *CARRAYO;				/* old cut values */
	t_float *DARRAYO;				/* old delay values */
	t_float *AARRAYO;				/* old atten values */
	t_float *DARRAY;				/* delay values */
	t_float *CARRAY;				/* cut values */
	t_float *AARRAY;				/* atten values */
	t_float *FARRAY;				/* fade in on cut changes */
	t_float *XS;					/* save X location */
	t_float *YS;					/* save y location */
	t_float *THETAS;				/* save THETA */
	t_float *AMPS;					/* save AMP */
	t_float *BACKS;					/* save BACK */
	t_float *X;
	t_float *Y;
	t_float *THETA;
	t_float *AMP;
	t_float *BACK;
} t_spaces_tilde;

extern void spaces_init(t_spaces_tilde *x);
extern void spaces_free(t_spaces_tilde *x);
extern void spaces (t_spaces_tilde *x);
#endif

#ifndef REALTIME
struct ug_desc{						/* unit generator table entry */
    char *ug_name;					/* ug symbolic name */
    char *ug_arglist;				/* symbolic argument list description */
    int  (*ug_call)();				/* pointer to ug code */
};

union arg_ptr{ /* args may be float or function pointers, etc. */
    float *v; 
    float **fp; 
    double **dp; 
    float (*f)();
    char  *s;
    long  *l;
    FILE **fileptr;
#ifdef csound
    struct sndesc **sfd;
#endif
#ifdef bicsf
    struct sndesc **snd;
#endif
};

#define	UGHEAD	(narg, ap, lens, incs, atypes, ugflags)\
register long narg,incs[];\
register union arg_ptr ap[];\
float lens[];\
char atypes[];\
long *ugflags;

#define	UGINIT \
 register long i,arg; extern long Ngen, Nchan; extern float *Outblock, *Outptr

#define UGLOOP  *ugflags &= ~(STARTFLAG); for (i=0; i<Ngen; i++)

#define	UGEND(n)	for(arg=n;arg<narg;arg++)ap[arg].v += incs[arg];

#define OUT	0

#define	FPTR(x) *ap[x].fp
#define	DPTR(x) *ap[x].dp
#define	FPT(x) ap[x].v
#define	VAL(x) *ap[x].v
#define	LVAL(x) *ap[x].l
#ifdef csound
#define	PVAL(x) *(*ap[x].sfd)
#define	PLOC(x) (*ap[x].sfd)
#endif /* csound */
#ifdef bicsf
#define PVAL(x) *(*ap[x].snd)
#define PLOC(x) (*ap[x].snd)
#endif  /* bicsf */
#define	FPLOC(x) (*ap[x].fileptr)
#define	SVAL(x) *ap[x].s
#define	LOC(x) ap[x].v
#define	SLOC(x) ap[x].s
#define LEN(x) lens[x]
#define INC(x) incs[x]
#define TYPE(x) atypes[x]
#define	LOOKUP(table,index)	*(LOC(table) + (long) VAL(index) )
#define STARTFLAG 1
#define ENDFLAG 2
#define	STARTNOTE (*ugflags & STARTFLAG)
#define	ENDNOTE (*ugflags & ENDFLAG)

#else /* ifndef REALTIME */

#define	UGHEAD	(t_spaces_tilde *x)

#define	UGINIT \
 register long i,arg

#define UGLOOP  for (i=0; i<Ngen; i++)

#define	UGEND(n)	;

#define	FPTR(a)	x->a
#define	VAL(a) 	x->a
#define	LVAL(a)	x->a

#endif /* ifndef REALTIME */


/*____________________________________
*/

#define	Srate		((long) x->sp_srate)	/* sampling rate */
#define	Nchan 		(x->sp_nchan) 			/* number of channels */
#define	Ngen		(x->sp_vsize) 			/* Samples to generate at 1 time (<= Lblocks) */
#define	Grevblock	(x->sp_grev)		 	/* Pointer to global reverb output block */
#define Outblock	((float *) x->sp_out)

#ifdef notdef 

/* the following is out of cmusic:m.globs.h */
float Lx[]={5.,-5.,-5.,5.}; 				/* Listening space x-coordinates */
float Ly[]={5.,5.,-5.,-5.}; 				/* Listening space y-coordinates */
int   NLs = 4;								/* Number of listening surfaces */
float Ax[]={50.,-50.,-50.,50.};				/* Acoustic space x-coordinates */
float Ay[]={50.,50.,-50.,-50};				/* Acoustic space y-coordinates */
int   NAs = 4;								/* Number of acoustic space surfaces */
float Sx[]={5.,-5.,-5.,5.}; 				/* Speaker x-coordinates */
float Sy[]={5.,5.,-5.,-5.}; 				/* Speaker y-coordinates */
float Revscale=.08;							/* Global reverb input scale */
float T60mult=.83;							/* T60 multiplier */
float Revthresh=.0001;						/* Reverb tail cancellation threshold */
float Direct = 1.;							/* Direct path distance amplifier */
float Reflect = 1.;							/* Reflected path distance amplifier */
#endif

int Spaceson;								/* Spatial processing flag */
int Spaceswason;							/* Spatial processing souvenir */
int Spacesreset;							/* Spatial processing reset flag */
float Maxecho;								/* Maximum echo amplitude generated */

int spaces_debug;
int spaces_delinterpol;	/* what type of interpolation */

#endif

#include <stdio.h>
#include <math.h>


#define CI	.00029851						/* 1/335 (meters per second) */
#define PI2	6.283185308
#ifndef MSP
#define PI	3.141592654
#endif /* !MSP */
#define IPI	.318309886
#define D270	4.712388981
#define D90	1.570796327
#define DIST(x1,y1,x2,y2) sqrt( (double) ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ) )
#define TAU	50								/* milliseconds in cross-fade */

float ***fp3alloc(int x, int y, int z); 
void fp3free(float ***fp3, int x, int y, int z);
float do_delay(float buf[],long len, long now, float tau);
float getcut(t_spaces_room *,
	float x1, float sp_y1, float x2, float y2, int fracret);
void getrefl(t_spaces_room *,
	float sx, float sy, float cx, float cy, int s,float *rx,float *ry);
int within(t_spaces_room *, float x,float y);

void
#ifndef REALTIME
spaces_init(narg, ap)
register long narg;
register union arg_ptr ap[];
#else
spaces_init(t_spaces_tilde *x)
#endif
{
	float maxdist, maxdelay, d;
	long dlen, now, j, k, nrv, c, r, s, ns;
	float RCI = CI*Srate;
	float NAs;
	t_spaces_room *rm;



	Spaceswason = Spaceson = 1;				/* Turn on global reverberator */
	maxdist = 0.;
	rm = x->sp_room;						/* the first room is the biggest */
	NAs = rm->spr_NAs;
	for(j=0; j<NAs; j++){ 					/* Find largest distance in acoustic space */
	    for(k=0; k<NAs; k++){
		if(j == k) continue;
		d = DIST(rm->spr_Ax[j], rm->spr_Ay[j],
						rm->spr_Ax[k], rm->spr_Ay[k]);
		if(d > maxdist) maxdist = d;
	    }
	}
	maxdelay = 2.*maxdist;
	dlen = LVAL(DLEN) = maxdelay*RCI + 0.5;
	now = LVAL(NOW) = 0;
	FPTR(DBUF) = (float *) calloc(dlen, sizeof(float));
#ifdef REALTIME
	x->sp_firstbuf = 1;
	nrv = 1;								/* number of radiation vectors specified */
	x->sp_interpol = 1;						/* interpolation is on by default */
#else
	nrv = (narg - X)/5;						/* number of radiation vectors specified */
#endif
	ns = NAs + 1; 							/* Extra surface is for direct path values */
#ifdef REALTIME 							/* SDY should fix for cmusic */
	FPTR(CARRAYO) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(DARRAYO) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(AARRAYO) = (float *) fp3alloc(nrv,ns,Nchan);
#endif
	FPTR(CARRAY) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(DARRAY) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(AARRAY) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(FARRAY) = (float *) fp3alloc(nrv,ns,Nchan);
	FPTR(XS) = (float *) calloc(nrv, sizeof(float));
	FPTR(YS) = (float *) calloc(nrv, sizeof(float));
	FPTR(THETAS) = (float *) calloc(nrv, sizeof(float));
	FPTR(AMPS) = (float *) calloc(nrv, sizeof(float));
	FPTR(BACKS) = (float *) calloc(nrv, sizeof(float));

}

void
#ifndef REALTIME
spaces_free(narg, ap)
register long narg;
register union arg_ptr ap[];
#else
spaces_free(t_spaces_tilde *x)
#endif
{
	float *dbuf;
	float ***cut, ***delay, ***atten, ***fader, delsamp;
	long dlen, now, j, nrv, c, r, s, ns;
	float *xs, *ys, *ts, *as, *bs;
	float NAs;
	t_spaces_room *rm;

	rm = x->sp_room;
	NAs = rm->spr_NAs;
	dbuf = FPTR(DBUF);
	cut = (float ***) FPTR(CARRAY);
	delay = (float ***) FPTR(DARRAY);
	atten = (float ***) FPTR(AARRAY);
	fader = (float ***) FPTR(FARRAY);
#ifdef REALTIME
	nrv = 1;								/* number of radiation vectors specified */
#else
	nrv = (narg - X)/5;						/* number of radiation vectors specified */
#endif
	xs = FPTR(XS);
	ys = FPTR(YS);
	ts = FPTR(THETAS);
	as = FPTR(AMPS);
	bs = FPTR(BACKS);
	ns = NAs + 1;

	free(dbuf);
#ifdef REALTIME 							/* SDY should fix for cmusic */
	fp3free((float ***) FPTR(CARRAYO),nrv,ns,Nchan);
	fp3free((float ***) FPTR(DARRAYO),nrv,ns,Nchan);
	fp3free((float ***) FPTR(AARRAYO),nrv,ns,Nchan);
#endif
	fp3free(cut,nrv,ns,Nchan);
	fp3free(delay,nrv,ns,Nchan);
	fp3free(atten,nrv,ns,Nchan);
	fp3free(fader,nrv,ns,Nchan);
	free(xs);
	free(ys);
	free(ts);
	free(as);
	free(bs);
}

/*
 * sp_valcopy -- copy the content of a three dimentional array
 */
void
sp_valcopy(float ***dest, float ***src, int n1, int n2, int n3)
{
	int i, j, k;
	for(i = 0; i < n1; i++)
		for(j = 0; j < n2; j++)
			for(k = 0; k < n3; k++)
				dest[i][j][k] = src[i][j][k] ;
}

#define y1	sp_y1							/* to avoid an overshadowing of the math fun y1() */

void
spaces
UGHEAD
{
    UGINIT;
    float *Out = Outblock, *Grev = Grevblock;
    float *dbuf;
    float ***cut, ***delay, ***atten, ***fader, delsamp;
    float ***oldcut, ***olddelay, ***oldatten;
    float *xs, *ys, *ts, *as, *bs;
    float x1, y1, x2, y2, t1, t2, fff;
    float d, di, chanval[4], radiant;
    float newatt; /* the new attenuation or interpolated */
    float newcut; /* the new cut value (or interpolated) */
    int place, change, inside = 0;
    long dlen, now, j, nrv, c, r, s = 0, ns;
    float fade, rx, ry, dx, dy, dt, phi, val, tsn;
    float RCI = CI*Srate;
    int t;
    t_spaces_room *cur_room;

#ifndef REALTIME
    if(STARTNOTE){
	spaces_init(narg, ap);
    }
#endif
    dbuf = FPTR(DBUF);
    dlen = LVAL(DLEN);
    now = LVAL(NOW);
    cut = (float ***) FPTR(CARRAY);
    delay = (float ***) FPTR(DARRAY);
    atten = (float ***) FPTR(AARRAY);
    fader = (float ***) FPTR(FARRAY);
#ifdef REALTIME
    oldcut = (float ***) FPTR(CARRAYO);
    olddelay = (float ***) FPTR(DARRAYO);
    oldatten = (float ***) FPTR(AARRAYO);
    nrv = 1;							/* number of radiation vectors specified */
#else
    nrv = (narg - X)/5;					/* number of radiation vectors specified */
#endif
    xs = FPTR(XS);
    ys = FPTR(YS);
    ts = FPTR(THETAS);
    as = FPTR(AMPS);
    bs = FPTR(BACKS);
    fade = 1000./(TAU*Srate);
    cur_room = &x->sp_room[x->sp_curroom];
    ns = cur_room->spr_NAs + 1;
    change = 0;
    UGLOOP{
#ifndef REALTIME
	change = (i==0) ;
	dbuf[now] = VAL(0); 				/* install current sample in delay buffer */
	for(j=0; j<nrv; j++){
	    place = j*5;
	    if(xs[j] != VAL(X+place)){change=1; xs[j]=VAL(X+place);}
	    if(ys[j] != VAL(Y+place)){change=1; ys[j]=VAL(Y+place);}
	    if(ts[j] != VAL(THETA+place)){change=1; ts[j]=VAL(THETA+place);}
	    if(as[j] != VAL(AMP+place)){change=1; as[j]=VAL(AMP+place);}
	    if(bs[j] != VAL(BACK+place)){change=1; bs[j]= VAL(BACK+place);}
	}
#else /* ifndef REALTIME */
	if (x->sp_interpol) {
		if (x->sp_firstbuf) {
			x->sp_firstbuf = 0;
			change = 1;
		}
	} else
		change = (i==0);
	/* i is the index of our pass in the buffers and is defined in UGLOOP */
	dbuf[now] = x->sp_in[i];			/* install current sample in delay buffer */
	j = 0; 								/* only one nrv */

	if (!x->sp_interpol) {
		if(xs[j] != VAL(X)[i]){change=1; xs[j]=VAL(X)[i];}
		if(ys[j] != VAL(Y)[i]){change=1; ys[j]=VAL(Y)[i];}
		if (x->sp_realray) {
			if(ts[j] != VAL(THETA)[i]) {
				change = 1;
				ts[j] = VAL(THETA)[i];
			}
			if(bs[j] != VAL(BACK)[i]) {
				change=1;
				bs[j]= VAL(BACK)[i];
			}
		} else {
			if (ts[j] != x->sp_theta) {
				change=1;
				ts[j] = x->sp_theta;
			}
			if (bs[j] != x->sp_back) {
				change=1;
				bs[j] = x->sp_back;
			}
		}
		if(as[j] != VAL(AMP)[i]){change=1; as[j]=VAL(AMP)[i];}
	} else if (i == 0) {

		if (xs[j] != VAL(X)[x->sp_vsize - 1]) {
			change = 1;
			xs[j] = VAL(X)[x->sp_vsize - 1];
		}
		if (ys[j] != VAL(Y)[x->sp_vsize - 1]) {
			change = 1;
			ys[j] = VAL(Y)[x->sp_vsize - 1];
		}
		if (x->sp_realray) {
			if (ts[j] != VAL(THETA)[x->sp_vsize - 1]) {
				change = 1;
				ts[j] = VAL(THETA)[x->sp_vsize - 1];
			}
			if (bs[j] != VAL(BACK)[x->sp_vsize - 1]) {
				change = 1;
				bs[j] =  VAL(BACK)[x->sp_vsize - 1];
			}
		} else {
			if (ts[j] != x->sp_theta) {
				change=1;
				ts[j] = x->sp_theta;
			}
			if (bs[j] != x->sp_back) {
				change=1;
				bs[j] = x->sp_back;
			}
		}
		if (as[j] != VAL(AMP)[x->sp_vsize - 1]) {
			change = 1;
			as[j] = VAL(AMP)[x->sp_vsize - 1];
		}
		if (change) {
			sp_valcopy(oldcut,   cut, j + 1, ns, Nchan);
			sp_valcopy(olddelay, delay, j + 1, ns, Nchan);
			sp_valcopy(oldatten, atten, j + 1, ns, Nchan);
		}
	}
#endif /* ifndef REALTIME */
	
	if(change && (!x->sp_interpol || i == 0)){
		for(r=0; r<nrv; r++) for(c=0; c<Nchan; c++){
			x1 = xs[r]; y1 = ys[r];
			x2 = cur_room->spr_Sx[c]; y2 = cur_room->spr_Sy[c];
			cut[r][0][c] = getcut(cur_room, x1,y1,x2,y2,1);
			dx = x2 - x1 ;
			dx *= dx ;
			dy = y2 - y1 ;
			dy *= dy ;
			d = sqrt((double)( dx + dy )) ;
			delay[r][0][c] = d*RCI;
			if ( x->sp_direct != 1. )
				d = pow( (double) d, (double) x->sp_direct ) ;
			if(bs[r] < 1.0){
				dy = y1 - y2;
				dx = x1 - x2;
				phi = dx ? 
				atan2( (double) dy, (double) dx) : 
							(dy > 0 ? D270 : D90 );
				tsn = ts[r];
				tsn = fmod(tsn, PI);
				dt = tsn - phi;
				dt = dt >= 0. ? dt : -dt;

				dt = fmod(dt, PI);
				radiant = 1. + (bs[r] - 1.) * dt* IPI;
				radiant *= radiant;
				atten[r][0][c] = as[r]*radiant/(1. + d) ;
			} else
				atten[r][0][c] = as[r]/(1. + d) ;
		}
		/* now do surface reflections */
		for(r=0; r<nrv; r++) for(s=1; s<ns; s++) for(c=0; c<Nchan; c++){
			x1 = xs[r]; y1 = ys[r];
			x2 = cur_room->spr_Sx[c]; y2 = cur_room->spr_Sy[c];
			getrefl(cur_room, x1,y1,x2,y2,s-1,&rx,&ry);
			cut[r][s][c] = 0.;
			cut[r][s][c] = getcut(cur_room, x1,y1,rx,ry,1);
			if (cut[r][s][c])
				cut[r][s][c]*=getcut(cur_room, rx,ry,x2,y2,1);
			dx = x1 - rx ;
			dx *= dx ;
			dy = y1 - ry ;
			dy *= dy ;
			d = sqrt((double) ( dx + dy )) ;
			dx = x2 - rx ;
			dx *= dx ;
			dy = y2 - ry ;
			dy *= dy ;
			d += sqrt((double) ( dx + dy )) ;
			delay[r][s][c] = d*RCI;
			if ( x->sp_reflect != 1. )
				d = pow((double) d, (double) x->sp_reflect) ;
			if(bs[r] < 1.0){
				dy = y1 - ry;
				dx = x1 - rx;
				phi = dx ? atan2(dy,dx) : (dy > 0 ? D270 : D90 );
				tsn = ts[r];
//				if(tsn > PI)tsn = PI2 - tsn;
				tsn = fmod(tsn, PI);
				dt = tsn - phi ;
				dt = ( dt >= 0. ? dt : -dt ) ;
				dt = fmod(dt, PI);
				radiant = 1. + (( (double) ( bs[r]) - 1. ) )*dt*IPI;
				radiant *= radiant;
				atten[r][s][c] = as[r]*radiant/(1. + d) ;
			} else
				atten[r][s][c] = as[r]/(1. + d) ;
	    }
	}

#ifdef REALTIME
	*Grev = 0;
#endif
	for(c=0; c<Nchan; c++){
	    chanval[c] = 0;
	    for(r=0; r<nrv; r++)
	     for(s=0; s<ns; s++){
#ifdef REALTIME
		if (change && x->sp_interpol) {
			newcut = oldcut[r][s][c] +
			 ((cut[r][s][c]-oldcut[r][s][c]) * i) / x->sp_vsize;
			d = olddelay[r][s][c] +
			 ((delay[r][s][c]-olddelay[r][s][c]) * i) / x->sp_vsize;
			newatt = oldatten[r][s][c] +
			 ((atten[r][s][c]-oldatten[r][s][c]) * i) / x->sp_vsize;
		} else {
			newcut = cut[r][s][c];
			newatt =  atten[r][s][c];
			d = delay[r][s][c];
		}
#else
		newatt =  atten[r][s][c];
		d = delay[r][s][c];
#endif
		delsamp = do_delay(dbuf,dlen,now,d);

		if(s == 0 && (c == 0 || r != 0)){
		    if(!within(cur_room, xs[r], ys[r])){  /* Outside? */
			*Grev += delsamp * newatt;
			inside = 0;
		    } else {
			inside = 1;
		    }
		} else if(!inside) 
			*Grev += delsamp * newatt;
/************************************************************************/
		chanval[c] +=  delsamp * newatt * newcut;
	    }
	    di = chanval[c];
#ifndef REALTIME
	    *Out++ += di ;
#else /* ifndef REALTIME */
	    ((float **)Out)[c][i] = di;
#endif
	    val = ( di >= 0 ? di : -di ) ;
	    if(val > Maxecho)Maxecho = di ;
	}
	Grev++;
	now -= 1;
	if(now < 0)now = dlen - 1;
	LVAL(NOW) = now;
	LVAL(DLEN) = dlen;
	UGEND(0);
    }
#ifndef REALTIME
    if(ENDNOTE){
	spaces_free(narg, ap);
    }
#endif
}


#define	DIFFRACTION(x);							\
		if (x < rm->spr_TH)					\
			newx = (pow((rm->spr_TH-x)/rm->spr_TH,1./rm->spr_CF));	\
		else							\
			newx = 0;					\

float
getcut(t_spaces_room *rm, float x1, float y1, float x2, float y2, int retfrac)
{
	register int i;
	int test1, test2 ;
	float t1, t2 ;
	float m_ray = 0, b_ray = 0;
	int horiz_ray=0, vert_ray=0, first_ray=1;
	int cutthrough;	/* if we cut through speaker locations */
	float *lx1, *ly1;
	float *lx2, *ly2;
	int *horiz, *vert;
	float *m, *b;
	float x, newx = 0;
	float ref_factor;
	float y;
	float xsection;
	float ysection;

	lx1 = rm->spr_lx1;
	lx2 = rm->spr_lx2;
	ly1 = rm->spr_ly1;
	ly2 = rm->spr_ly2;
	horiz = rm->spr_vert;
	vert = rm->spr_horiz;
	m = rm->spr_m;
	b = rm->spr_b;


    if (!rm->spr_notfirst) {
	if (SP_MAX_SURFACE != 4)
		post("spaces: getcut() internal error, this routine only works for square rooms");

	rm->spr_notfirst = 1 ;
	for( i=0; i<rm->spr_NLs; i++ ) {
	    lx1[i] = rm->spr_Lx[i];
	    ly1[i] = rm->spr_Ly[i];
	    lx2[i] = rm->spr_Lx[(i+1)%rm->spr_NLs] ;
	    ly2[i] = rm->spr_Ly[(i+1)%rm->spr_NLs] ;
		if (ly1[i] == ly2[i]) {
			horiz[i] = 1;
			continue;
		} else
			horiz[i] = 0;
		if (lx1[i] == lx2[i]) {
			vert[i] = 1;
			continue;
		} else
			vert[i] = 0 ;
		
		m[i] = (ly2[i] - ly1[i]) / (lx2[i] - lx1[i]);
		b[i] = ly1[i] - (m[i] * lx1[i]);
	}
    }


    cutthrough = 0;
    ref_factor = 1.0;
    for (i=0; i<rm->spr_NLs; i++) {
	if( horiz[i] ) {

		if (y1 == ly1[i])
			continue;
		if (y2 == ly2[i])
			continue;
		test1 = y1 > ly1[i];
		test2 = y2 > ly2[i];
	} else if( vert[i] ) {
		if (x1 == lx1[i])
			continue;
		if (x2 == lx2[i])
			continue;
		test1 = x1 > lx1[i];
		test2 = x2 > lx2[i];
	} else {

		t1 = (m[i] * x1) + b[i];
		if( t1 == y1 )
			continue;
		test1 = t1 > y1;
		t2 = (m[i] * x2) + b[i];
		if( t2 == y2 )
			continue;
		test2 = t2 > y2;
	}

	if( test1 == test2 ) continue;

	if(first_ray)
	{
		first_ray=0;
		if (y1 == y2)
			horiz_ray = 1;
		else if (x1 == x2)
			vert_ray = 1;
		else {
			m_ray = (y2 - y1) / (x2 - x1);
			b_ray = y1 - (m_ray * x1);
		}
	}

	if(horiz_ray) {
		if (y1 == ly1[i])
			continue;
		if (y2 == ly2[i])
			continue;
		test1 = ly1[i] > y1;
		test2 = ly2[i] > y1;
	} else if(vert_ray) {
		if (x1 == lx1[i])
			continue;
		if (x2 == lx2[i])
			continue;
		test1 = lx1[i] > x1;
		test2 = lx2[i] > x1;
	} else {
		t1 = (m_ray * lx1[i]) + b_ray;

		if (t1 == ly1[i]) {
			if (cutthrough)
				return (0.);
			cutthrough = 1;
			continue;
		}
		test1 = t1 > ly1[i];
		t2 = (m_ray * lx2[i]) + b_ray;

		if (t2 == ly2[i]) {
			if (cutthrough)
				return (0.);
			cutthrough = 1;
			continue;
		}
		test2 = t2 > ly2[i];
	}
/*************************************************************************/
	if( test1 != test2) {
		if (horiz[i]) {
			if (m_ray < 0) {
				if (m_ray >= -10)
					x = 0;
				else {
					x =  1 - 1/pow(-(m_ray + 9), 1./3);
				}
				xsection = (ly1[i] - b_ray) / m_ray;
				t2 = fabs(lx1[i] - xsection);
				t1 = fabs(xsection - lx2[i]);
				DIFFRACTION(t2);
			} else if (m_ray > 0) {
				if (m_ray <= 10)
					x = 0;
				else
					x =  1 - 1/pow(m_ray - 9, 1./3);
				xsection = (ly1[i] - b_ray) / m_ray;
				t2 = fabs(lx1[i] - xsection);
				t1 = fabs(xsection - lx2[i]);
				DIFFRACTION(t1);
			} else {
				x = 1;
		printf("bad case %3.3f,%3.3f - %3.3f,%3.3f -- i = %d mray = %3.3f\n",
								x1, y1, x2, y2, i, m_ray);
			}
		} else if (vert[i]) {
			if (m_ray < 0) {
				if (m_ray <= -.1)
					x = 0;
				else
					x = 1 - pow(-(m_ray * 10), 1./3);
				ysection = (lx1[i] * m_ray) + b_ray;
				t2 = fabs(ly1[i] - ysection);
				t1 = fabs(ysection - ly2[i]);
				DIFFRACTION(t1);
			} else if (m_ray > 0) {
				if (m_ray >= .1)
					x = 0;
				else
					x = 1 - pow(10 * m_ray, 1./3);
				ysection = (lx1[i] * m_ray) + b_ray;
				t2 = fabs(ly1[i] - ysection);
				t1 = fabs(ysection - ly2[i]);
				DIFFRACTION(t2);
			} else {
				x = 1;
				printf("bad case2 %3.3f,%3.3f - %3.3f,%3.3f -- i = %d mray = %3.3f\n",
								x1, y1, x2, y2, i, m_ray);
			}
		} else {
			printf("bad case Neither vertical nor horizontal\n");
			printf("bad case %3.3f,%3.3f - %3.3f,%3.3f -- i = %d mray = %3.3f\n",
								x1, y1, x2, y2, i, m_ray);
		}

		if (newx < ref_factor) {
			ref_factor = newx;
		}
		if (ref_factor < 0.00001)
			return (ref_factor);
	}
	continue;
		
/*************************************************************************/

	if( test1 != test2 )
	    return( 0. );		/* ray path cut */
    }
    return (ref_factor);	/* ray path not cut */
}

int
within(t_spaces_room *rm, float x,float y)
{
	if(x < rm->spr_Lx[0] && y < rm->spr_Ly[0])
		if(x > rm->spr_Lx[1] && y < rm->spr_Ly[1])
			if(x > rm->spr_Lx[2] && y > rm->spr_Ly[2])
				if(x < rm->spr_Lx[3] && y > rm->spr_Ly[3])
					return(1);
	return(0);
}

void
getrefl(t_spaces_room *rm, float sx, float sy, float cx, float cy,
						int s, float *rx, float *ry)
{
	double lx1,lx2,ly1,ly2,d2,s2,c2,r2,x,y,a;
	double dx1, dy1, dx2, dy2, m1 = 0, m2 = 0, b1 = 0, b2 = 0;
	double *d, *sint, *cost, *rho, *sin2t, *cos2t, *f1, *f2;

	if (SP_MAX_SURFACE != 4)
		post("spaces: getrefl() internal error, this routine only works for square rooms");
	d = rm->spr_d;
	sint = rm->spr_sint;
	cost = rm->spr_cost;
	rho = rm->spr_rho;
	sin2t = rm->spr_sin2t;
	cos2t = rm->spr_cos2t;
	f1 = rm->spr_f1;
	f2 = rm->spr_f2;

	s = s%rm->spr_NAs;
	lx1 = rm->spr_Ax[s];
	ly1 = rm->spr_Ay[s];
	lx2 = rm->spr_Ax[(s+1)%rm->spr_NAs];
	ly2 = rm->spr_Ay[(s+1)%rm->spr_NAs];
	/*
	* find normal equations for walls (needs to be done only once)
	*/
	if(d[s] == 0.){	
		d[s] = 1./sqrt((double) ((lx2-lx1)*(lx2-lx1) +
							(ly2-ly1)*(ly2-ly1)));
		sint[s] = (ly2-ly1)*d[s];
		cost[s] = (lx2-lx1)*d[s];
		rho[s] = ly1*cost[s] - lx1*sint[s];
		if(rho[s]>0.){
			sint[s]= -sint[s];
			cost[s]= -cost[s];
			rho[s]= -rho[s];
		}
		sin2t[s] = 2.*sint[s]*cost[s];
		cos2t[s] = 2.*cost[s]*cost[s] - 1.;
		f1[s] = -2.*rho[s]*sint[s];
		f2[s] =  2.*rho[s]*cost[s];
	}

	if (lx1 > 0 && lx2 > 0) {
		if (cx > lx1) {
			*rx = cx;
			*ry = cy;
			return;
		}
	} else if (lx1 < 0 && lx2 < 0) {
		if (cx < lx1) {
			*rx = cx;
			*ry = cy;
			return;
		}
	} else if (ly1 > 0 && ly2 > 0) {
		if (cy > ly1) {
			*rx = cx;
			*ry = cy;
			return;
		}
	} else if (ly1 < 0 && ly2 < 0) {
		if (cy < ly1) {
			*rx = cx;
			*ry = cy;
			return;
		}
	} else {

post("what is this case?");
	}

	x = f1[s] + sy*sin2t[s] + sx*cos2t[s];
	y = f2[s] - sy*cos2t[s] + sx*sin2t[s];

	if( (dx1 = x - cx) != 0.){ 
		m1 = (y - cy)/dx1; 
		b1 = y - m1*x; 
	}
	if( (dx2 = lx2 - lx1) != 0.){ 
		m2 = (ly2 - ly1)/dx2; 
		b2 = ly1 - m2*lx1; 
	} else { 
		*ry = m1*lx1 + b1; 
		*rx = lx1; 
		return; 
	}
	if(dx1 == 0.){ 
		*ry = m2*x + b2; 
		*rx = cx; 
		return; 
	}
	*rx = (b2 - b1)/(m1 - m2); 
	*ry = m1*(*rx) + b1; 
	return;

}

float
do_delay(buf,len,now,tau)
float buf[]; float tau; long len, now;
{
	register long t1, t2 ;
	float frac,  a,  b,  c,  d, cminusb;
	float *fp;


	if (!len) {
		fprintf(stderr,"\nspace unit: do_delay internal error.\n");
		return (0.0);
	}
	t1 = now + tau;
	if (t1 >= len)
		t1 -=len;

	while(t1 >= len)t1 -= len; /* old code --SDY */
	t2 = t1 + 1;
	if (t2 >= len)
		t2 -=len;
	if (t2 >= len) {
		fprintf(stderr,"\nspace unit: do_delay internal error.\n");
		fprintf(stderr,"\nspace unit: t2 = %d, len = %d\n",
							(int) t2, (int) len);
		return (0.0);
	}
	while(t2 >= len)t2 -= len; 		/* old code --SDY */

	switch (spaces_delinterpol) {
	case 0: 						/* linear interpolation */
		break;
	case 1:
		if (t1 + 2 > len || t1 < 1)
			break;
		frac = t1 - ((int)t1);
		fp = buf + t1;
		a = fp[-1];
		b = fp[0];
		c = fp[1];
		d = fp[2];
		cminusb = c-b;
		return (b + frac * (
			    cminusb - 0.5f * (frac-1.) * (
			(a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
			)));
	default:
		fprintf(stderr,"\nspace: do_delay unknown interpolation.\n");
		break;
	}
	return(buf[t1] + (tau - ( (int) tau ))*(buf[t2] - buf[t1]));
}

float *** fp3alloc(x,y,z) register int x,y,z; {
 register float ***fp3; register int i,j;
    if ((fp3 = (float ***) calloc(x,sizeof(float **))) == NULL){
	fprintf(stderr,"\nCMUSIC: space generator: fp3alloc failed.\n");
	exit (-1);
    }
    for (i = 0; i < x; i++) {
	if ((fp3[i] = (float **) calloc(y, sizeof(float *))) == NULL){
	    fprintf(stderr,"\nCMUSIC: space generator: fp3alloc failed.\n");
	    exit (-1);
	}
	for (j = 0; j < y; j++) {
	    if ((fp3[i][j] = (float *) calloc(z, sizeof(float))) == NULL){
		fprintf(stderr,"\nCMUSIC: space generator: fp3alloc failed.\n");
		exit (-1);
	    }
	}
    }
    return(fp3);
}

void
fp3free(fp3,x,y,z) register float ***fp3; register int x,y,z; {
 register int i,j;
    for (i = 0; i < x; i++) {
	for (j = 0; j < y; j++) {
	    free(fp3[i][j]);
	}
	free(fp3[i]);
    }
    free(fp3);
}

#ifdef notdef /*old code from cmusic */

float oldgetcut(x1,y1,x2,y2, fracret)
float x1,y1,x2,y2; int fracret; {
 register int i;
 int test1, test2 ;
 float dx, dy, dinv, t1, t2, lsintheta, lcostheta, lrho ;
 static float costheta[4], sintheta[4], rho[4] ;
 static float lx1[4], ly1[4], lx2[4], ly2[4] ;
 static int horiz[4], vert[4] ;
 static int first = 1 ;

 extern int Newgetcut;
if (Newgetcut)
	return (ngetcut(x1,y1,x2,y2, fracret));

    if( first ) {
	first = 0 ;
	for( i=0; i<NLs; i++ ) {
	    lx1[i] = Lx[i] ;
	    ly1[i] = Ly[i] ;
	    lx2[i] = Lx[(i+1)%NLs] ;
	    ly2[i] = Ly[(i+1)%NLs] ;
	    if( ly1[i] == ly2[i] ) horiz[i] = 1 ; else horiz[i] = 0 ;
	    if( lx1[i] == lx2[i] ) vert[i] = 1 ; else vert[i] = 0 ;
	    dx = lx2[i] - lx1[i] ;
	    dy = ly2[i] - ly1[i] ;
	    dinv = 1./sqrt( (double) ( dx*dx + dy*dy ) ) ;
	    sintheta[i] = dy*dinv ;
	    costheta[i] = dx*dinv ;
	    rho[i] = ( ly1[i]*lx2[i] - lx1[i]*ly2[i] )*dinv ;
	}
    }
    for( i=0; i<NLs; i++ ) {
	if( horiz[i] ) {

	    if( y1 == ly1[i] ) continue ;
	    if( y2 == ly2[i] ) continue ;
	    test1 = y1 > ly1[i] ;
	    test2 = y2 > ly2[i] ;
	} else if( vert[i] ) {
	    if( x1 == lx1[i] ) continue ;
	    if( x2 == lx2[i] ) continue ;
	    test1 = x1 > lx1[i] ;
	    test2 = x2 > lx2[i] ;
	} else {
	    t1 = y1*costheta[i] - x1*sintheta[i] ;
	    if( t1 == rho[i] ) continue ;
	    test1 = t1 > rho[i] ;
	    t2 = y2*costheta[i] - x2*sintheta[i] ;
	    if( t2 == rho[i] ) continue ;
	    test2 = t2 > rho[i] ;
	}

	if( test1 == test2 ) continue ;

	dx = x2 - x1 ;
	dy = y2 - y1 ;
	dinv = sqrt( (double) ( dx*dx + dy*dy ) ) ;
	if( dinv == 0. ) continue ;
	dinv = 1./dinv ;
	lsintheta = dy*dinv ;
	lcostheta = dx*dinv ;
	lrho = ( y1*x2 - x1*y2 )*dinv ;
	t1 = ly1[i]*lcostheta - lx1[i]*lsintheta ;

	if( t1 == lrho ) continue ;
	test1 = t1 > lrho ;
	t2 = ly2[i]*lcostheta - lx2[i]*lsintheta ;
	if( t2 == lrho ) continue ;
	test2 = t2 > lrho ;

	if( test1 != test2 )
	    return( 0. ) ;			/* ray path cut */
    }
    return( 1. ) ;				/* ray path not cut */
}

#endif

/*___________________________________________________________
*/



static char *spaces_version = "Spaces~ :: Space Is the Place :: Forked by maxum germanus 2011";
static t_class *spaces_tilde_class;
static char usage[] = "spaces~ [card] [stereo|quad] inner-size outersize";
int spaces_mkroom(t_spaces_tilde *x);
int spaces_mkroom(t_spaces_tilde *x)
{
	int i;
	float innersize;

	if (!x->sp_nir) {
		post("spaces_mkroom: number of rooms not set");
		return (1);
	}
	
	if (x->sp_outersize < x->sp_innersize) {
		post("spaces_mkroom: (%f > %f) inner room cannot be larger than outer room", x->sp_innersize, x->sp_outersize);
		return (1);
	}
	
	x->sp_room = (t_spaces_room *) calloc(x->sp_nir, sizeof (t_spaces_room));
	if (!x->sp_room) {
		post("spaces_mkroom: no memory, calloc() failed");
		return (1);
	}
	x->sp_curroom = 0;

	for (i = 0; i < x->sp_nir; i++) {
		innersize = x->sp_innersize * (x->sp_nir - i) / x->sp_nir;
		x->sp_room[i].spr_TH = innersize / 4.0;
		x->sp_room[i].spr_CF = SPR_DEFAULT_CF;
		x->sp_room[i].spr_NLs = 4;
		x->sp_room[i].spr_NAs = 4;
		x->sp_room[i].spr_Lx[0] =   innersize / 2;
		x->sp_room[i].spr_Ly[0] =   innersize / 2;
		x->sp_room[i].spr_Lx[1] = - innersize / 2;
		x->sp_room[i].spr_Ly[1] =   innersize / 2;
		x->sp_room[i].spr_Lx[2] = - innersize / 2;
		x->sp_room[i].spr_Ly[2] = - innersize / 2;
		x->sp_room[i].spr_Lx[3] =   innersize / 2;
		x->sp_room[i].spr_Ly[3] = - innersize / 2;

		x->sp_room[i].spr_Ax[0] =   x->sp_outersize / 2;
		x->sp_room[i].spr_Ay[0] =   x->sp_outersize / 2;
		x->sp_room[i].spr_Ax[1] = - x->sp_outersize / 2;
		x->sp_room[i].spr_Ay[1] =   x->sp_outersize / 2;
		x->sp_room[i].spr_Ax[2] = - x->sp_outersize / 2;
		x->sp_room[i].spr_Ay[2] = - x->sp_outersize / 2;
		x->sp_room[i].spr_Ax[3] =   x->sp_outersize / 2;
		x->sp_room[i].spr_Ay[3] = - x->sp_outersize / 2;
		
		switch (x->sp_nchan) {
		case 1:
			x->sp_room[i].spr_ns = 1;
			x->sp_room[i].spr_Sx[0] = 0;
			x->sp_room[i].spr_Sy[0] = innersize / 2;
			break;
		case 2:
			x->sp_room[i].spr_ns = 2;
			x->sp_room[i].spr_Sx[0] = - innersize / 2;
			x->sp_room[i].spr_Sy[0] =   innersize / 2;
			x->sp_room[i].spr_Sx[1] =   innersize / 2;
			x->sp_room[i].spr_Sy[1] =   innersize / 2;
			break;
		case 4:
			x->sp_room[i].spr_ns = 4;
			x->sp_room[i].spr_Sx[0] = - innersize / 2;
			x->sp_room[i].spr_Sy[0] =   innersize / 2;
			x->sp_room[i].spr_Sx[1] =   innersize / 2;
			x->sp_room[i].spr_Sy[1] =   innersize / 2;
			x->sp_room[i].spr_Sx[2] = - innersize / 2;
			x->sp_room[i].spr_Sy[2] = - innersize / 2;
			x->sp_room[i].spr_Sx[3] =   innersize / 2;
			x->sp_room[i].spr_Sy[3] = - innersize / 2;
			break;
		defautl:
			free (x->sp_room);
			post ("spaces: only mono, stereo and quad configuration supported for now");
			return (1);
		}
	}
	return (0);
}


static void *
spaces_tilde_new(t_symbol *s, int ac, t_atom*av)
{
	int i;
	t_spaces_tilde *x = (t_spaces_tilde *) pd_new(spaces_tilde_class);
	x->sp_nir = 1; 			/* for now we have only one inner room */
	x->sp_realray = 0;		/* assume no real-time radiance info unless noted */
	x->sp_back = 1;			/* assume a ful back radiance */
	x->sp_theta = 0;
	x->sp_nchan = 0;
	x->sp_direct = 1.;		/* Direct path distance amplifier */
	x->sp_reflect = 1.;		/* Reflected path distance amplifier */

def:
	if (!ac) {
		/* no arguments so all  set to default quad */
		x->sp_innersize = SP_DEFAULT_INNER;
		x->sp_outersize = SP_DEFAULT_OUTER;
		if (!x->sp_nchan)
			x->sp_nchan = 4;

		goto configdone;
	}

	if (av->a_type == A_FLOAT) {
		if (ac != 2 || av[1].a_type != A_FLOAT) {
			post(usage);
			return ((void *) NULL);
		}
		x->sp_innersize = atom_getfloat(av);
		x->sp_outersize = atom_getfloat(++av);
		if (!x->sp_nchan)
			x->sp_nchan = 4;
		goto configdone;
	}
	if (!strcmp (atom_getsymbol(av)->s_name, "card")) {
		x->sp_realray = 1; /* we will have real-time radiance info */
		ac--; av++;
		goto def;
	}
	if (!strcmp (atom_getsymbol(av)->s_name, "stereo")) {
			x->sp_nchan = 2;
			ac--; av++;
			goto def;
	}
	if (!strcmp (atom_getsymbol(av)->s_name, "quad")) {
			x->sp_nchan = 4;
			ac--; av++;
			goto def;
	}
	post(usage);
	return ((void *) NULL);
	
configdone:
	if (spaces_mkroom(x))
		return ((void *) NULL);
	inlet_new(&x->sp_obj, &x->sp_obj.ob_pd, &s_signal, &s_signal); /*X loc*/
	inlet_new(&x->sp_obj, &x->sp_obj.ob_pd, &s_signal, &s_signal); /*Y loc*/
	if (x->sp_realray) /*THETA*/
		inlet_new(&x->sp_obj, &x->sp_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->sp_obj, &x->sp_obj.ob_pd, &s_signal, &s_signal); /*AMP*/
	if (x->sp_realray) /* BACK */
		inlet_new(&x->sp_obj, &x->sp_obj.ob_pd, &s_signal, &s_signal);
	x->sp_srate = 44100;
	x->sp_grev = (t_float *) 0;
	x->sp_out = (t_float **) malloc(sizeof(t_float *) * (x->sp_nchan + 1));
	if (!x->sp_out) {
		post("spaces~: malloc failed");
		return ((void *) NULL);
	}
	for (i = 0; i < x->sp_nchan; i++)
		outlet_new(&x->sp_obj, &s_signal);
	outlet_new(&x->sp_obj, &s_signal);	/* global reverb */

	spaces_init(x);

	return (x);
}

static t_int *
spaces_tilde_perform(t_int *w)
{
	t_spaces_tilde *x = (t_spaces_tilde *)(w[1]);

	if (!x->sp_off)
		spaces(x);

	return (w+2);
}

static void
spaces_tilde_dsp(t_spaces_tilde *x, t_signal **sp)
{
	int i;
	int j = 0;

	dsp_add(spaces_tilde_perform, 1, x);
	x->sp_srate = sp[0]->s_sr;
	x->sp_vsize = sp[0]->s_n;
	x->sp_in = sp[j++]->s_vec;
	x->X = sp[j++]->s_vec;
	x->Y = sp[j++]->s_vec;
	x->AMP = sp[j++]->s_vec;
	if (x->sp_realray) /* THETA */
		x->THETA = sp[j++]->s_vec;
	if (x->sp_realray) /* BACK */
		x->BACK = sp[j++]->s_vec;
	for (i = 0; i < x->sp_nchan; i++)
		x->sp_out[i] = sp[i + j]->s_vec;
	x->sp_grev = sp[i + j]->s_vec;
}

static void
spaces_tilde_free(t_spaces_tilde *x)
{
	spaces_free(x);
	free(x->sp_out);
}

static void
spaces_tilde_interpol(t_spaces_tilde *x, t_float f)
{
	x->sp_interpol = (f != 0);
	if (x->sp_interpol)
		post("spaces interpolation on");
	else
		post("spaces interpolation off");
}


static void
spaces_tilde_direct(t_spaces_tilde *x, t_float f)
{
	/* SDY fix the code in ug.spaces.c to use this. */
	x->sp_direct = f;
printf("spaces~: Direct = %3.3f\n", f);
}

static void spaces_tilde_reflect(t_spaces_tilde *x, t_float f)
{
	/* SDY fix the code in ug.spaces.c to use this. */
	x->sp_reflect = f;
printf("spaces~: Reflect = %3.3f\n", f);
}

static void
spaces_tilde_difth(t_spaces_tilde *x, t_float f)
{
	int i;

	/* set the diffraction threshold to be the same for all inner rooms */
	for (i = 0; i < x->sp_nir; i++)
		x->sp_room[i].spr_TH = f;
printf("spaces~: Difraction threshold = %3.3f\n", f);
}

static void
spaces_tilde_back(t_spaces_tilde *x, t_float f)
{
	x->sp_back = f;
	post ("set back to %f", f);
}

static void
spaces_tilde_theta(t_spaces_tilde *x, t_float f)
{
	post ("set theta to %f", f);
	x->sp_theta = f;
}

static void
spaces_tilde_difcf(t_spaces_tilde *x, t_float f)
{
	int i;
	
	/* set the diffraction crossfade to be the same for all inner rooms */
	for (i = 0; i < x->sp_nir; i++)
		x->sp_room[i].spr_CF = f;
		
printf("spaces~: Difraction curve power = %3.3f\n", f);
}

static void
spaces_tilde_print(t_spaces_tilde *x, t_float f)
{
	post("Spaces:");
	post("	threshhold = %f", x->sp_room[0].spr_TH);
	post("	crossface = %f", x->sp_room[0].spr_CF);
}

static void
spaces_tilde_debug(t_spaces_tilde *x, t_float f)
{
	extern int spaces_debug;


        spaces_debug = !spaces_debug;
	printf("spaces~: debug= %s\n",spaces_debug ?  "on" : "off");
}

static void
spaces_tilde_delinterpol(t_spaces_tilde *x, t_float f)
{
	extern int spaces_delinterpol;

	spaces_delinterpol = (int) f;
printf("spaces~: delinterpol = %d\n", spaces_delinterpol);
}

static void
spaces_tilde_stop(t_spaces_tilde *x)
{
	x->sp_off = 1;
}

static void
spaces_tilde_start(t_spaces_tilde *x)
{
	x->sp_off = 0;
}

static void
spaces_tilde_stereo(t_spaces_tilde *x, t_float inner, t_float outer)
{
/* SDY */
}

static void
spaces_tilde_quad(t_spaces_tilde *x, t_float inner, t_float outer)
{
/* SDY */
}

static void
spaces_tilde_five_1(t_spaces_tilde *x, t_float inner, t_float outer)
{
	post("5.1 diffusion have not been implemented yet");
/* SDY */
}

void
spaces_tilde_setup(void)
{
	spaces_tilde_class = class_new(gensym("spaces~"),
		(t_newmethod)spaces_tilde_new, (t_method)spaces_tilde_free,
     	                              sizeof(t_spaces_tilde), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(spaces_tilde_class, t_spaces_tilde, sp_f);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_dsp,
    							gensym("dsp"), 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_interpol,
	        gensym("interpolate"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_direct,
	        gensym("direct"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_reflect,
	        gensym("reflect"), A_FLOAT, 0);
//	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_quad,
//	        gensym("quad"), A_FLOAT, A_FLOAT, 0);
//	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_stereo,
//	        gensym("stereo"), A_FLOAT, A_FLOAT, 0);
//	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_five_1,
//	        gensym("five.1"), A_FLOAT, A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_theta,
	        gensym("theta"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_theta,
	        gensym("Theta"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_back,
	        gensym("Back"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_back,
	        gensym("back"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difth,
	        gensym("TH"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difth,
	        gensym("th"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difth,
	        gensym("threshold"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difcf,
	        gensym("CF"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difcf,
	        gensym("cf"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_difcf,
	        gensym("crossface"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_delinterpol,
	        gensym("delinterpol"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_print,
	        gensym("print"), A_FLOAT, 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_stop,
	        gensym("stop"), 0);
	class_addmethod(spaces_tilde_class, (t_method)spaces_tilde_start,
	        gensym("start"), 0);

	post(spaces_version);
}