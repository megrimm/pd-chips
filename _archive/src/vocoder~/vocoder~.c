/* vocoder~ -- vocoder effect inspired by xvox 
 * written by Simon Morlat ( http://simon.morlat.free.fr )
 * 
 * Copyleft 2001 Yves Degoyon.
 * Permission is granted to use this software for any purpose provided you
 * keep this copyright notice intact.
 *
 * THE AUTHOR AND HIS EXPLOITERS MAKE NO WARRANTY, EXPRESS OR IMPLIED,
 * IN CONNECTION WITH THIS SOFTWARE.
 *
 */

#include "m_pd.h"

#define OUTPUT_DELAY 50
#define THRES 0.06
#define Dmin 10.81e-3

#ifndef __FILTERS_H__
#define __FILTERS_H__

void lpc_filter(double *buf_ppf, double *lpc_coef, double *buf_sy,int n);
void hp_filter(double *in,double cut,int n);

#endif

extern int dsp_fd;
extern int bsize;
extern double voix[512+256];
extern double synth[512+256];
extern double sortie[512+12];
extern double *out;
extern double BandExpTable[11];
extern double BinomialWindowTable[11];
extern double HammingWindowTable[128*3];
extern double CosineTable[257];
extern double LspDcTable[11]; 

void lev_durb(double *corr,double *lpc_coef);
void comp_lpc(double *buf_x,double *corr,double *lpc_coef,int n);
void lpc_filter(double *buf_ppf, double *lpc_coef, double *buf_sy,int n);
void lsp2lpc(double *lsp_coef,double *lpc_coef);
void lpc2lsp(double lpc_coef[],double *f1,double *f2,double lsp_coef[]);
double evalc(double cw,double *fonc);

void lpc_filter(double *buf_ppf, double *lpc_coef, double *buf_sy, int n)
{
  int i,j;
  double acc;

  for(i=0;i<n;i++)
  {
      acc=buf_ppf[i]*lpc_coef[0];
      for(j=1;j<11;j++)
      {
	  acc=acc-lpc_coef[j]*buf_sy[i-j];
      };
      buf_sy[i]=acc;
  };
}

void hp_filter(double *in,double cut,int n)
{
  int i;
  double prev_e,s_out;
  prev_e=s_out=0;
  for(i=0;i<n;i++)
  {
      s_out=in[i]-prev_e+cut*s_out;
      prev_e=in[i];
      in[i]=s_out;
  };
}



/* Levinson Durbin algorithm for computing LPC coefficients using 
autocorrelation fonction */
void lev_durb(double *corr,double *lpc_coef)
{
  double k[11],tab[11];
  double err,acc;
  int i,j;  
  double *a=tab;
  double *prev_a=lpc_coef;
  double *exch;
  
  
  /*init vectors*/
  for (i=0;i<11;i++)
  {
      prev_a[i]=0;
      a[i]=0;
  };
  err=corr[0];
  for(i=1;i<11;i++)
  {
      prev_a[0]=1;
      acc=0;
      for(j=0;j<i;j++) 
      {
	  acc=acc+prev_a[j]*corr[i-j];
      };
      a[i]=k[i]=-acc/err;
      for(j=1;j<i;j++)
      {
	  a[j]=prev_a[j]+k[i]*prev_a[i-j];
      };
      err=(1-k[i]*k[i])*err;
      exch=prev_a;
      prev_a=a;
      a=exch;
  };
}

void comp_lpc(double *buf_x,double *corr,double *lpc_coef,int n)
{
  double buffer[n*3];
  double acc,max=0;
  int i,j;
  /* computes LPC analysis for one subframe */
  /* hamming windowing*/
  acc=0;
  for(i=0;i<2*n;i++)
  {
      acc+=buf_x[i]*buf_x[i];
  };
  if (acc>THRES) 
  {
      for(i=0;i<3*n;i++)
      {
	  buffer[i]=buf_x[i-n]*HammingWindowTable[i];
      };
      /* autocorrelation computation*/
      for(i=0;i<11;i++)
      {
	  acc=0;
	  for(j=i;j<n*3;j++)	    
          {
	      acc=acc+buffer[j]*buffer[j-i];
	  };
	  /* correction with binomial coeffs */
	  corr[i]=acc;//*BinomialWindowTable[i];
      };
      corr[0]=corr[0]*(1.0+1.0/1024.0);
      lev_durb(corr,lpc_coef);
  } 
  else
  {
      for(i=0;i<11;i++)
      {
	  lpc_coef[i]=0;
      };
  }
}
  


/* LPC to LSP coefficients conversion */

/* evaluate  function C(x) (whose roots are  LSP coeffs)*/

double evalc(double cw,double *fonc)
{
  double b[7];
  double x,res;
  int k;

  x=cw;
  b[5]=1;
  b[6]=0;
  for(k=4;k>0;k--)
  {
      b[k]=2*x*b[k+1]-b[k+2]+fonc[5-k];
  };
  res=x*b[1]-b[2]+fonc[5]/2;
  return(res);
}



/* converts LPC vector into LSP frequency vector */
/* all LSP frenquencies are in [0;PI] but are normalized to be in [0;1] */
void lpc2lsp(double lpc_coef[],double *f1,double *f2,double lsp_coef[])
{
  
  int i,k=1;
  double *fonc,*prev_f,*f_exch;
  double prev_sign1,sign,prev_sign2;
  double *s, *prev_s,*s_exch;
  double lpc_exp[11];

  /* first computes an additional bandwidth expansion on LPC coeffs*/
  for(i=1;i<11;i++)
  {
      lpc_exp[i]=lpc_coef[i]*BandExpTable[i];
  };
  /* computes the F1 and F2 coeffs*/
  f1[0]=f2[0]=1;
  for(i=0;i<5;i++)
  {
      f1[i+1]=lpc_exp[i+1]+lpc_exp[10-i]-f1[i];
      f2[i+1]=lpc_exp[i+1]-lpc_exp[10-i]+f2[i];
  }; 

   /*find the roots of C(x) alternatively for F1 and F2*/
  fonc=f1;
  prev_f=f2;
  prev_sign1=evalc(1.0,f1);
  prev_sign2=evalc(1.0,f2);
  s=&prev_sign1;
  prev_s=&prev_sign2;
  for(i=1;i<256;i++)
  {
      sign=evalc(CosineTable[i],fonc);
      if ((sign)*(*s)<0)
      {
	  /* interpolate to find root*/
	  lsp_coef[k]=((double)i-(*s)/(sign-(*s)))/256.0;
	  k++;
	  /* chek if all roots are found */
	  if (k==11) i=257;
	  (*s)=sign;
	  /* pointers exchange  */
	  s_exch=s;
	  s=prev_s;
	  prev_s=s_exch;
	  f_exch=fonc;
	  fonc=prev_f;
	  prev_f=f_exch;
       }
       else (*s)=sign;
  }
  /* if here all roots are not found , use  lspDC vector */
  if (k!=11)
  {
      for(i=1;i<11;i++)
      {
	  lsp_coef[i]=LspDcTable[i];
      };
  };
}


/* converts lsp frequencies to lpc coeffs */

void lsp2lpc(double *lsp_coef,double *lpc_coef)
{
   int i,j=0,index,ok=1;
   double lspcos[11],delta,tmp,p_avg;
   double F1[12],F2[12]; /* begin at indice two*/

   F1[0]=0;F1[1]=1;
   F2[0]=0;F2[1]=1;
   /* stability check */
   while(ok && (j<11))
   {
       ok=0;
       for(i=1;i<10;i++)
       {
	   if( (lsp_coef[i+1]-lsp_coef[i]) < Dmin)
	   {
	       ok=1;
	       p_avg=(lsp_coef[i]+lsp_coef[i+1])/2.0;
	       lsp_coef[i]=p_avg-Dmin/2.0;
	       lsp_coef[i+1]=p_avg+Dmin/2.0;
	   };
       };
       j++;
   }
   
   /* first converts lsp frequencies to lsp coefficients */
   for (i=1;i<11;i++)
   {
       /* interpolation */
       tmp=lsp_coef[i]*255.0;
       index=(int)tmp;
       delta=CosineTable[index+1]-CosineTable[index];
       lspcos[i]=CosineTable[index]+delta*(tmp-index);
   };

   for(i=2;i<7;i++)
   {
       F1[i]=-2*lspcos[2*i-3]*F1[i-1]+2*F1[i-2];
       F2[i]=-2*lspcos[2*i-2]*F2[i-1]+2*F2[i-2]; 
       for(j=i-1;j>1;j--)
       {
	   F1[j]=F1[j]-2*lspcos[2*i-3]*F1[j-1]+F1[j-2];
	   F2[j]=F2[j]-2*lspcos[2*i-2]*F2[j-1]+F2[j-2];
       };
   };
   for(i=6;i>1;i--)
   {
       F1[i]=F1[i]+F1[i-1];
       F2[i]=F2[i]-F2[i-1];
   };
   for(i=2;i<7;i++)
   {
       lpc_coef[i-1]=(F1[i]+F2[i])*0.5;
       lpc_coef[i+4]=(F1[8-i]-F2[8-i])*0.5;
   };
   lpc_coef[0]=1;
}

////////////////
// Begin TABLES
///////////////
double LspDcTable[11] = {
0 , 0.0955505 , 0.144073 , 0.23468 , 0.329773 , 0.42334 , 0.503387 , 0.602783 , 0.679321 , 0.77771 , 0.845886 
};

double CosineTable[257] = {
1 , 0.999939 , 0.999695 , 0.999329 , 0.998779 , 0.998108 , 0.997314 , 0.996338 , 0.995178 , 0.993896 , 
0.992493 , 0.990906 , 0.989197 , 0.987305 , 0.985291 , 0.983093 , 0.980774 , 0.978333 , 0.975708 , 
0.972961 , 0.970032 , 0.96698 , 0.963806 , 0.960449 , 0.95697 , 0.953308 , 0.949524 , 0.945618 , 0.941528 , 
0.937317 , 0.932983 , 0.928528 , 0.923889 , 0.919128 , 0.914185 , 0.90918 , 0.903992 , 0.898682 , 0.89325 , 
0.887634 , 0.881897 , 0.876099 , 0.870117 , 0.863953 , 0.857727 , 0.851379 , 0.844849 , 0.838196 , 
0.831482 , 0.824585 , 0.817566 , 0.810486 , 0.803223 , 0.795837 , 0.78833 , 0.780762 , 0.77301 , 0.765198 , 
0.757202 , 0.749146 , 0.740967 , 0.732666 , 0.724243 , 0.715759 , 0.707092 , 0.698364 , 0.689514 , 0.680603 , 
0.67157 , 0.662415 , 0.653198 , 0.64386 , 0.634399 , 0.624878 , 0.615234 , 0.60553 , 0.595703 , 0.585815 , 
0.575806 , 0.565735 , 0.555542 , 0.545349 , 0.534973 , 0.524597 , 0.514099 , 0.50354 , 0.49292 , 0.482178 , 
0.471375 , 0.46051 , 0.449585 , 0.438599 , 0.427551 , 0.416443 , 0.405212 , 0.393982 , 0.38269 , 0.371338 , 
0.359924 , 0.348389 , 0.336914 , 0.325317 , 0.31366 , 0.302002 , 0.290283 , 0.278503 , 0.266724 , 0.254883 , 
0.242981 , 0.231079 , 0.219116 , 0.207092 , 0.195068 , 0.183044 , 0.170959 , 0.158875 , 0.146729 , 0.134583 , 
0.122437 , 0.110229 , 0.0980225 , 0.0858154 , 0.0735474 , 0.0613403 , 0.0490723 , 0.0368042 , 0.0245361 , 
0.0122681 , 0 , -0.0122681 , -0.0245361 , -0.0368042 , -0.0490723 , -0.0613403 , -0.0735474 , -0.0858154 , 
-0.0980225 , -0.110229 , -0.122437 , -0.134583 , -0.146729 , -0.158875 , -0.170959 , -0.183044 , -0.195068 , 
-0.207092 , -0.219116 , -0.231079 , -0.242981 , -0.254883 , -0.266724 , -0.278503 , -0.290283 , -0.302002 , 
-0.31366 , -0.325317 , -0.336914 , -0.348389 , -0.359924 , -0.371338 , -0.38269 , -0.393982 , -0.405212 , 
-0.416443 , -0.427551 , -0.438599 , -0.449585 , -0.46051 , -0.471375 , -0.482178 , -0.49292 , -0.50354 , 
-0.514099 , -0.524597 , -0.534973 , -0.545349 , -0.555542 , -0.565735 , -0.575806 , -0.585815 , -0.595703 , 
-0.60553 , -0.615234 , -0.624878 , -0.634399 , -0.64386 , -0.653198 , -0.662415 , -0.67157 , -0.680603 , 
-0.689514 , -0.698364 , -0.707092 , -0.715759 , -0.724243 , -0.732666 , -0.740967 , -0.749146 , -0.757202 , 
-0.765198 , -0.77301 , -0.780762 , -0.78833 , -0.795837 , -0.803223 , -0.810486 , -0.817566 , -0.824585 , 
-0.831482 , -0.838196 , -0.844849 , -0.851379 , -0.857727 , -0.863953 , -0.870117 , -0.876099 , -0.881897 , 
-0.887634 , -0.89325 , -0.898682 , -0.903992 , -0.90918 , -0.914185 , -0.919128 , -0.923889 , -0.928528 , 
-0.932983 , -0.937317 , -0.941528 , -0.945618 , -0.949524 , -0.953308 , -0.95697 , -0.960449 , -0.963806 , 
-0.96698 , -0.970032 , -0.972961 , -0.975708 , -0.978333 , -0.980774 , -0.983093 , -0.985291 , -0.987305 , 
-0.989197 , -0.990906 , -0.992493 , -0.993896 , -0.995178 , -0.996338 , -0.997314 , -0.998108 , -0.998779 , 
-0.999329 , -0.999695 , -0.999939 , -1 
};

double BandExpTable[11] = {
1 , 0.993988 , 0.988037 , 0.982117 , 0.976227 , 0.970367 , 0.964539 , 0.95874 , 0.953003 , 0.947266 , 0.941589 
};

double HammingWindowTable[128*3] = {
5.38469e-16,  6.69307e-05,  0.000267706,  0.000602271,  0.00107054,  0.00167238,  0.00240763,  0.00327611,  
0.00427757,  0.00541174,  0.00667833,  0.00807699,  0.00960736,  0.011269,  0.0130615,  0.0149844,  0.0170371,  
0.0192191,  0.0215298,  0.0239687,  0.0265349,  0.029228,  0.032047,  0.0349914,  0.0380602,  0.0412527,  
0.0445681,  0.0480053,  0.0515636,  0.055242,  0.0590394,  0.0629548,  0.0669873,  0.0711357,  0.0753989,  
0.0797758,  0.0842652,  0.0888659,  0.0935766,  0.0983962,  0.103323,  0.108357,  0.113495,  0.118736,  0.12408,  
0.129524,  0.135068,  0.140709,  0.146447,  0.152279,  0.158204,  0.164221,  0.170327,  0.176522,  0.182803,  
0.18917,  0.195619,  0.20215,  0.208761,  0.21545,  0.222215,  0.229054,  0.235966,  0.242949,  0.25,  0.257118, 
0.264302,  0.271548,  0.278856,  0.286222,  0.293646,  0.301126,  0.308658,  0.316242,  0.323875,  0.331555,  
0.33928,  0.347048,  0.354858,  0.362706,  0.37059,  0.37851,  0.386462,  0.394444,  0.402455,  0.410492,  
0.418552,  0.426635,  0.434737,  0.442857,  0.450991,  0.459139,  0.467298,  0.475466,  0.48364,  0.491819,  
0.5,  0.508181,  0.51636,  0.524534,  0.532702,  0.540861,  0.549009,  0.557143,  0.565263,  0.573365,  
0.581448,  0.589508,  0.597545,  0.605556,  0.613538,  0.62149,  0.62941,  0.637294,  0.645142,  0.652952,  
0.66072,  0.668445,  0.676125,  0.683758,  0.691342,  0.698874,  0.706354,  0.713778,  0.721144,  0.728452,  
0.735698,  0.742882,  0.75,  0.757051,  0.764034,  0.770946,  0.777785,  0.78455,  0.791239,  0.79785,  
0.804381,  0.81083,  0.817197,  0.823478,  0.829673,  0.835779,  0.841796,  0.847721,  0.853553,  0.859291,  
0.864932,  0.870476,  0.87592,  0.881264,  0.886505,  0.891643,  0.896677,  0.901604,  0.906423,  0.911134,  
0.915735,  0.920224,  0.924601,  0.928864,  0.933013,  0.937045,  0.940961,  0.944758,  0.948436,  0.951995,  
0.955432,  0.958747,  0.96194,  0.965009,  0.967953,  0.970772,  0.973465,  0.976031,  0.97847,  0.980781,  
0.982963,  0.985016,  0.986938,  0.988731,  0.990393,  0.991923,  0.993322,  0.994588,  0.995722,  0.996724,  
0.997592,  0.998328,  0.998929,  0.999398,  0.999732,  0.999933,  1,  0.999933,  0.999732,  0.999398,  0.998929,  
0.998328,  0.997592,  0.996724,  0.995722,  0.994588,  0.993322,  0.991923,  0.990393,  0.988731,  0.986938,  
0.985016,  0.982963,  0.980781,  0.97847,  0.976031,  0.973465,  0.970772,  0.967953,  0.965009,  0.96194,  
0.958747,  0.955432,  0.951995,  0.948436,  0.944758,  0.940961,  0.937045,  0.933013,  0.928864,  0.924601,  
0.920224,  0.915735,  0.911134,  0.906423,  0.901604,  0.896677,  0.891643,  0.886505,  0.881264,  0.87592,  
0.870476,  0.864932,  0.859291,  0.853553,  0.847721,  0.841796,  0.835779,  0.829673,  0.823478,  0.817197,  
0.81083,  0.804381,  0.79785,  0.791239,  0.78455,  0.777785,  0.770946,  0.764034,  0.757051,  0.75,  0.742882,  
0.735698,  0.728452,  0.721144,  0.713778,  0.706354,  0.698874,  0.691342,  0.683758,  0.676125,  0.668445,  
0.66072,  0.652952,  0.645142,  0.637294,  0.62941,  0.62149,  0.613538,  0.605556,  0.597545,  0.589508,  
0.581448,  0.573365,  0.565263,  0.557143,  0.549009,  0.540861,  0.532702,  0.524534,  0.51636,  0.508181,  
0.5,  0.491819,  0.48364,  0.475466,  0.467298,  0.459139,  0.450991,  0.442857,  0.434737,  0.426635,  
0.418552,  0.410492,  0.402455,  0.394444,  0.386462,  0.37851,  0.37059,  0.362706,  0.354858,  0.347048,  
0.33928,  0.331555,  0.323875,  0.316242,  0.308658,  0.301126,  0.293646,  0.286222,  0.278856,  0.271548,  
0.264302,  0.257118,  0.25,  0.242949,  0.235966,  0.229054,  0.222215,  0.21545,  0.208761,  0.20215,  
0.195619,  0.18917,  0.182803,  0.176522,  0.170327,  0.164221,  0.158204,  0.152279,  0.146447,  0.140709,  
0.135068,  0.129524,  0.12408,  0.118736,  0.113495,  0.108357,  0.103323,  0.0983962,  0.0935766,  0.0888659,  
0.0842652,  0.0797758,  0.0753989,  0.0711357,  0.0669873,  0.0629548,  0.0590394,  0.055242,  0.0515636,  
0.0480053,  0.0445681,  0.0412527,  0.0380602,  0.0349914,  0.032047,  0.029228,  0.0265349,  0.0239687,  
0.0215298,  0.0192191,  0.0170371,  0.0149844,  0.0130615,  0.011269,  0.00960736,  0.00807699,  0.00667833,  
0.00541174,  0.00427757,  0.00327611,  0.00240763,  0.00167238,  0.00107054,  0.000602271,  0.000267706,  
6.69307e-05,
};
/////////////
//End TABLES
////////////


typedef struct _vocoder
{
    t_object x_obj;
    t_float x_f;
    t_float x_cutoff;
    t_int   x_vfeedback;
    double  *x_in1buf;
    double  *x_in2buf;
    double  *x_outbuf;
    t_int   x_blocksize;
    t_int   x_process;
} t_vocoder;

static t_class *vocoder_class;

static char   *vocoder_version = "vocoder~: version 0.1, written by ydegoyon@free.fr, inspired by xvox (Simon Morlat)";

static void vocoder_cutoff(t_vocoder *x, t_floatarg fcutoff )
{
    if ( fcutoff > 128.0 )
    {
       fcutoff = 128.0;
    }
    if ( fcutoff < 0.0 )
    {
       fcutoff = 0.0;
    }
    x->x_cutoff = fcutoff;
}

static void vocoder_vfeedback(t_vocoder *x, t_floatarg fvfeedback )
{
    if ( fvfeedback > 100.0 )
    {
       fvfeedback = 100.0;
    }
    if ( fvfeedback < 0.0 )
    {
       fvfeedback = 0.0;
    }
    x->x_vfeedback = fvfeedback;
}

static t_int *vocoder_perform(t_int *w)
{
    t_vocoder *x = (t_vocoder *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *fin1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    double  correls[12], lpc_coef[11], f1[12], f2[12], lsp_coef[11];
    int offset=0;
    int n = (int)(w[5]), i;

    if ( !x->x_process ) return (w+6);
    if ( x->x_blocksize != n ) 
    {
       if ( x->x_in1buf ) freebytes( x->x_in1buf, 3*x->x_blocksize/2*sizeof( double ) );
       if ( x->x_in2buf ) freebytes( x->x_in2buf, 3*x->x_blocksize/2*sizeof( double ) );
       if ( x->x_outbuf ) freebytes( x->x_outbuf, (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
       x->x_blocksize = n;
       x->x_in1buf = (double*) getbytes( 3*x->x_blocksize/2*sizeof( double ) );
       x->x_in2buf = (double*) getbytes( 3*x->x_blocksize/2*sizeof( double ) );
       x->x_outbuf = (double*) getbytes( (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
       if ( !x->x_in1buf || !x->x_in2buf || !x->x_outbuf ) 
       {
          post( "vocoder~ : allocations failed : stop processing" );
          x->x_process = 0;
       }
    } 

    for(i=0;i<x->x_blocksize/2;i++)
    {
      x->x_in1buf[i]=x->x_in1buf[i+x->x_blocksize];
    };
    for(i=0;i<OUTPUT_DELAY;i++) x->x_outbuf[i]=x->x_outbuf[i+x->x_blocksize];
    for(i=0;i<x->x_blocksize;i++)
    {
      x->x_in1buf[x->x_blocksize/2+i]=(double)(*(in1++));
      x->x_in2buf[x->x_blocksize/2+i]=(double)(*(in2++));
    }

    hp_filter(x->x_in2buf,x->x_cutoff/128.,n);
    for(i=0;i<4;i++)
    {
      comp_lpc(x->x_in1buf+offset,correls,lpc_coef,x->x_blocksize/4);
      if (lpc_coef[0]!=0)
      {
          lpc2lsp(lpc_coef,f1,f2,lsp_coef);
          lsp2lpc(lsp_coef,lpc_coef);
      };
      lpc_filter(x->x_in2buf+offset,lpc_coef,x->x_outbuf+OUTPUT_DELAY+offset,x->x_blocksize/4);
      offset+=x->x_blocksize/4;
    };
    for(i=0;i<x->x_blocksize;i++)
    {
      if ( x->x_outbuf[OUTPUT_DELAY+i] > 1.0 ) x->x_outbuf[OUTPUT_DELAY+i]=1.0;
      if ( x->x_outbuf[OUTPUT_DELAY+i] < -1.0 ) x->x_outbuf[OUTPUT_DELAY+i]=-1.0;
      *(out1++)=(t_float)(((100-x->x_vfeedback)*x->x_outbuf[OUTPUT_DELAY+i]
                           + x->x_vfeedback*(*fin1++))/100.0);
    };
    
    return (w+6);
}

static void vocoder_dsp(t_vocoder *x, t_signal **sp)
{
    dsp_add(vocoder_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void *vocoder_new(void)
{
    t_vocoder *x = (t_vocoder *)pd_new(vocoder_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("cutoff"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("vfeedback"));
    outlet_new(&x->x_obj, &s_signal);
    x->x_cutoff = 60.;
    x->x_vfeedback = 50;
    x->x_blocksize=-1;
    x->x_in1buf = NULL;
    x->x_in2buf = NULL;
    x->x_outbuf = NULL;
    x->x_process = 1;
    return (x);
}

  /* clean up */
static void vocoder_free(t_vocoder *x)
{
    if ( x->x_in1buf ) freebytes( x->x_in1buf, 3*x->x_blocksize/2*sizeof( double ) );
    if ( x->x_in2buf ) freebytes( x->x_in2buf, 3*x->x_blocksize/2*sizeof( double ) );
    if ( x->x_outbuf ) freebytes( x->x_outbuf, (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
}

void vocoder_tilde_setup(void)
{
    post(vocoder_version);
    vocoder_class = class_new(gensym("vocoder~"), (t_newmethod)vocoder_new, (t_method)vocoder_free,
    	sizeof(t_vocoder), 0, 0);
    CLASS_MAINSIGNALIN( vocoder_class, t_vocoder, x_f );
    class_addmethod(vocoder_class, (t_method)vocoder_dsp, gensym("dsp"), 0);
    class_addmethod(vocoder_class, (t_method)vocoder_cutoff, gensym("cutoff"), A_FLOAT, 0);
    class_addmethod(vocoder_class, (t_method)vocoder_vfeedback, gensym("vfeedback"), A_FLOAT, 0);
}
