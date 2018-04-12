#include <stdlib.h>
#include "m_pd.h"

static t_class *atari5200_class;  

#ifdef WIN32
#define int8  char
#define int16 short
#define int32 int
#else
#define int8  char
#define int16 int
#define int32 long
#endif

#define uint8  unsigned int8 
#define uint16 unsigned int16
#define uint32 unsigned int32

#define AUDF1_C     0xd200   
#define AUDC1_C     0xd201
#define AUDF2_C     0xd202
#define AUDC2_C     0xd203
#define AUDF3_C     0xd204
#define AUDC3_C     0xd205
#define AUDF4_C     0xd206
#define AUDC4_C     0xd207
#define AUDCTL_C    0xd208


/* CONSTANT DEFINITIONS */

/* As an alternative to using the exact frequencies, selecting a playback
   frequency that is an exact division of the main clock provides a higher
   quality output due to less aliasing.  For best results, a value of 
   1787520 MHz is used for the main clock.  With this value, both the 
   64 kHz and 15 kHz clocks are evenly divisible.  Selecting a playback
   frequency that is also a division of the clock provides the best 
   results.  The best options are FREQ_64 divided by either 2, 3, or 4.
   The best selection is based on a trade off between performance and
   sound quality. 
   
   Of course, using a main clock frequency that is not exact will affect
   the pitch of the output.  With these numbers, the pitch will be low
   by 0.127%.  (More than likely, an actual unit will vary by this much!) */

#define FREQ_17_EXACT     1789790  /* exact 1.79 MHz clock freq */
#define FREQ_17_APPROX    1787520  /* approximate 1.79 MHz clock freq */

/* definitions for AUDCx (D201, D203, D205, D207) */
#define NOTPOLY5    0x80     /* selects POLY5 or direct CLOCK */
#define POLY4       0x40     /* selects POLY4 or POLY17 */
#define PURE        0x20     /* selects POLY4/17 or PURE tone */
#define VOL_ONLY    0x10     /* selects VOLUME OUTPUT ONLY */
#define VOLUME_MASK 0x0f     /* volume mask */

/* definitions for AUDCTL (D208) */
#define POLY9       0x80     /* selects POLY9 or POLY17 */
#define CH1_179     0x40     /* selects 1.78979 MHz for Ch 1 */
#define CH3_179     0x20     /* selects 1.78979 MHz for Ch 3 */
#define CH1_CH2     0x10     /* clocks channel 1 w/channel 2 */
#define CH3_CH4     0x08     /* clocks channel 3 w/channel 4 */
#define CH1_FILTER  0x04     /* selects channel 1 high pass filter */
#define CH2_FILTER  0x02     /* selects channel 2 high pass filter */
#define CLOCK_15    0x01     /* selects 15.6999kHz or 63.9210kHz */

/* for accuracy, the 64kHz and 15kHz clocks are exact divisions of
   the 1.79MHz clock */
#define DIV_64      28       /* divisor for 1.79MHz clock to 64 kHz */
#define DIV_15      114      /* divisor for 1.79MHz clock to 15 kHz */

/* the size (in entries) of the 4 polynomial tables */
#define POLY4_SIZE  0x000f
#define POLY5_SIZE  0x001f
#define POLY9_SIZE  0x01ff

#define POLY17_SIZE 0x00007fffL    /* reduced to 15 bits for simplicity */

/* channel definitions */
#define CHAN1       0
#define CHAN2       1
#define CHAN3       2
#define CHAN4       3
#define SAMPLE      4

#define FALSE       0
#define TRUE        1

/* Initialze the bit patterns for the polynomials. */

/* The 4bit and 5bit patterns are the identical ones used in the pokey chip. */
/* Though the patterns could be packed with 8 bits per byte, using only a */
/* single bit per byte keeps the math simple, which is important for */
/* efficient processing. */

static uint8 bit4[POLY4_SIZE] = 
      { 1,1,0,1,1,1,0,0,0,0,1,0,1,0,0 };

static uint8 bit5[POLY5_SIZE] = 
      { 0,0,1,1,0,0,0,1,1,1,1,0,0,1,0,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,1 };
	  
static uint8 bit17[POLY17_SIZE];

typedef struct _atari5200
{ 
  t_object x_obj;
  t_float value1;
  t_float value2;
  t_float value3;
  t_float value4;
  t_float value5;
  t_float value6;
  t_float value7;
  t_float value8;
  t_float value9;
  t_float value1_old;
  t_float value2_old;
  t_float value3_old;
  t_float value4_old;
  t_float value5_old;
  t_float value6_old;
  t_float value7_old;
  t_float value8_old;
  t_float value9_old;
  long Div_n_cnt[4];
  long Div_n_max[4];
  long Base_mult;
  long Samp_n_max;
  long Samp_n_cnt[2];
  long Poly_adjust;
  long Poly17_size;
  long P4;
  long P5;
  long P17;
  long AUDF[4];
  long AUDC[4];
  long AUDCTL;
  long Outvol[4];
  long chan1Nonzero;
  long chan1lastFreq;
  long chan2Nonzero;
  long chan2lastFreq;
  long chan3Nonzero;
  long chan3lastFreq;
  long chan4Nonzero;
  long chan4lastFreq;
} t_atari5200;  

void Pokey_sound_init(t_atari5200 *x, uint32 freq17, uint16 playback_freq);
void Update_pokey_sound (t_atari5200 *x, uint16 addr, long val);
void Pokey_process(t_atari5200 *x, t_float *buffer, register uint16 n);

void *atari5200_new(void);  
t_int *atari5200_perform(t_int *w); 
void atari5200_dsp(t_atari5200 *x, t_signal **sp, short *count);  

int filterPokeyValue(long value);


void atari5200_tilde_setup(void)
{
	atari5200_class = class_new(gensym("atari5200~"),
			(t_newmethod)atari5200_new, 0, sizeof(t_atari5200), CLASS_DEFAULT, A_DEFFLOAT, 0);
	class_addmethod(atari5200_class, (t_method)atari5200_dsp, gensym("dsp"), 0);
}

/*
void main(void) {    
  setup((t_messlist **)&atari5200_class, (method)atari5200_new, (method)dsp_free, (short)sizeof(t_atari5200), 0L, 0);
  addinx((method)atari5200_in1, 1);
  addinx((method)atari5200_in2, 2);
  addinx((method)atari5200_in3, 3);
  addinx((method)atari5200_in4, 4);
  addinx((method)atari5200_in5, 5);
  addinx((method)atari5200_in6, 6);
  addinx((method)atari5200_in7, 7);
  addinx((method)atari5200_in8, 8);
  addinx((method)atari5200_in9, 9);

  addmess((method)atari5200_dsp, "dsp", A_CANT, 0);   
  finder_addclass("All Objects", "atari5200~");
  addmess((method)atari5200_assist, "assist", A_CANT, 0); 
  dsp_initclass();
} 
*/

void *atari5200_new(void)
{  
  //t_atari5200 *x = (t_atari5200 *)newobject(atari5200_class);  
  t_atari5200 *x = (t_atari5200 *)pd_new(atari5200_class);

  x->chan1Nonzero = 0;
  x->chan2Nonzero = 0;
  x->chan3Nonzero = 0;
  x->chan4Nonzero = 0;
  
  x->chan1lastFreq = 2;
  x->chan2lastFreq = 2;
  x->chan3lastFreq = 2;
  x->chan4lastFreq = 2;

  x->value1 = x->value1_old = 0.0;
  x->value2 = x->value2_old = 0.0;
  x->value3 = x->value3_old = 0.0;
  x->value4 = x->value4_old = 0.0;
  x->value5 = x->value5_old = 0.0;
  x->value6 = x->value6_old = 0.0;
  x->value7 = x->value7_old = 0.0;
  x->value8 = x->value8_old = 0.0;
  x->value9 = x->value9_old = 0.0;
  
  floatinlet_new(&x->x_obj, &x->value1);
  floatinlet_new(&x->x_obj, &x->value2);
  floatinlet_new(&x->x_obj, &x->value3);
  floatinlet_new(&x->x_obj, &x->value4);
  floatinlet_new(&x->x_obj, &x->value5);
  floatinlet_new(&x->x_obj, &x->value6);
  floatinlet_new(&x->x_obj, &x->value7);
  floatinlet_new(&x->x_obj, &x->value8);
  floatinlet_new(&x->x_obj, &x->value9);

  outlet_new(&x->x_obj, &s_signal);

/*
  dsp_setup((t_pxobject *)x, 0);          // no inlet 
  outlet_new((t_pxobject *)x, "signal");  // outlet
  intin(x, 1);
  intin(x, 2);
  intin(x, 3);
  intin(x, 4);
  intin(x, 5);
  intin(x, 6);
  intin(x, 7);
  intin(x, 8);
  intin(x, 9);
  
  x->chan1Nonzero = 0;
  x->chan2Nonzero = 0;
  x->chan3Nonzero = 0;
  x->chan4Nonzero = 0;
  
  x->chan1lastFreq = 2;
  x->chan2lastFreq = 2;
  x->chan3lastFreq = 2;
  x->chan4lastFreq = 2;
  
  //Pokey_sound_init(x, FREQ_17_APPROX,  22050);
  */
  Pokey_sound_init(x, FREQ_17_APPROX,  FREQ_17_APPROX/56);
  return (x); 
}   

void atari5200_dsp(t_atari5200 *x, t_signal **sp, short *count) 
{   
  dsp_add(atari5200_perform, 3, sp[0]->s_vec, sp[0]->s_n, x); 
}  

/*

void atari5200_in1(t_atari5200 *x, long value) 
{
  Update_pokey_sound(x, AUDCTL_C, value);
}

void atari5200_in2(t_atari5200 *x, long value) 
{
  if (value < 2) value = 2;
  Update_pokey_sound(x, AUDF4_C, value);
  x->chan4lastFreq = value;
}

void atari5200_in3(t_atari5200 *x, long value) 
{
  if(value != 0) {
    //are we transitioning?
    if(x->chan4Nonzero == 0) {
	  //default the frequency channel to avoid clicks.
	  Update_pokey_sound(x, AUDF4_C, x->chan4lastFreq);
	  x->chan4Nonzero = 1;
	}
  } else {
    x->chan4Nonzero = 0;
  }
  Update_pokey_sound(x, AUDC4_C, filterPokeyValue(value));
}

void atari5200_in4(t_atari5200 *x, long value) 
{
  if (value < 2) value = 2;
  Update_pokey_sound(x, AUDF3_C, value);
  x->chan3lastFreq = value;
}

void atari5200_in5(t_atari5200 *x, long value) 
{
  if(value != 0) {
    //are we transitioning?
    if(x->chan3Nonzero == 0) {
	  //default the frequency channel to avoid clicks.
	  Update_pokey_sound(x, AUDF3_C, x->chan3lastFreq);
	  x->chan3Nonzero = 1;
	}
  } else {
    x->chan3Nonzero = 0;
  }
  Update_pokey_sound(x, AUDC3_C, filterPokeyValue(value));
}
void atari5200_in6(t_atari5200 *x, long value) 
{
  if (value < 2) value = 2;
  Update_pokey_sound(x, AUDF2_C, value);
  x->chan2lastFreq = value;
}

void atari5200_in7(t_atari5200 *x, long value) 
{
  if(value != 0) {
    //are we transitioning?
    if(x->chan2Nonzero == 0) {
	  //default the frequency channel to avoid clicks.
	  Update_pokey_sound(x, AUDF2_C, x->chan2lastFreq);
	  x->chan2Nonzero = 1;
	}
  } else {
    x->chan2Nonzero = 0;
  }
  Update_pokey_sound(x, AUDC2_C, filterPokeyValue(value));
}

void atari5200_in8(t_atari5200 *x, long value) 
{
  if (value < 2) value = 2;
  Update_pokey_sound(x, AUDF1_C, value);
  x->chan1lastFreq = value;
}

void atari5200_in9(t_atari5200 *x, long value) 
{
  if(value != 0) {
    //are we transitioning?
    if(x->chan1Nonzero == 0) {
	  //default the frequency channel to avoid clicks.
	  Update_pokey_sound(x, AUDF1_C, x->chan1lastFreq);
	  x->chan1Nonzero = 1;
	}
  } else {
    x->chan1Nonzero = 0;
  }
  Update_pokey_sound(x, AUDC1_C, filterPokeyValue(value));
}

*/

int filterPokeyValue(long value) {
  int offset = 0;
  
  if(value < 0) value = 0;
  
  if(value > 15 && value <= 31) offset = 1;
  else if(value > 31 && value <= 47) offset = 2;
  else if(value > 47 && value <= 63) offset = 3;
  else if(value > 63 && value <= 79) offset = 4;
  else if(value > 79 && value <= 95) offset = 5;
  else if(value > 95 && value <= 111) offset = 6;
  else if(value > 111 && value <= 127) offset = 7;
  
  return (value % 16) + (offset * 32);
}

t_int *atari5200_perform(t_int *w) 
{     
  t_float *outL = (t_float *)(w[1]); 
  t_atari5200 *x = (t_atari5200 *)(w[3]); 

  int value;

  //Do all the processing here:

  //Value 1
  if(x->value1 != x->value1_old) {
	value = x->value1;
	Update_pokey_sound(x, AUDCTL_C, value);
	x->value1_old = x->value1;
  }

  //Value 2
  if(x->value2 != x->value2_old) {
	value = x->value2;
	if (value < 2) value = 2;
	Update_pokey_sound(x, AUDF4_C, value);
	x->chan4lastFreq = value;
	x->value2_old = x->value2;
  }

  //Value 3
  if(x->value3 != x->value3_old) {
	value = x->value3;
	if(value != 0) {
		//are we transitioning?
		if(x->chan4Nonzero == 0) {
		//default the frequency channel to avoid clicks.
		Update_pokey_sound(x, AUDF4_C, x->chan4lastFreq);
		x->chan4Nonzero = 1;
		}
	} else {
		x->chan4Nonzero = 0;
	}
	Update_pokey_sound(x, AUDC4_C, filterPokeyValue(value));
	x->value3_old = x->value3;
  }

  //Value 4
  if(x->value4 != x->value4_old) {
	value = x->value4;
	if (value < 2) value = 2;
	Update_pokey_sound(x, AUDF3_C, value);
	x->chan3lastFreq = value;
	x->value4_old = x->value4;
  }

  //Value 5
  if(x->value5 != x->value5_old) {
	value = x->value5;
	if(value != 0) {
		//are we transitioning?
		if(x->chan3Nonzero == 0) {
		//default the frequency channel to avoid clicks.
		Update_pokey_sound(x, AUDF3_C, x->chan3lastFreq);
		x->chan3Nonzero = 1;
		}
	} else {
		x->chan3Nonzero = 0;
	}
	Update_pokey_sound(x, AUDC3_C, filterPokeyValue(value));
	x->value5_old = x->value5;
  }

  //Value 6
  if(x->value6 != x->value6_old) {
	value = x->value6;
	if (value < 2) value = 2;
	Update_pokey_sound(x, AUDF2_C, value);
	x->chan2lastFreq = value;
	x->value6_old = x->value6;
  }

  //Value 7
  if(x->value7 != x->value7_old) {
	value = x->value7;
	if(value != 0) {
		//are we transitioning?
		if(x->chan2Nonzero == 0) {
		//default the frequency channel to avoid clicks.
		Update_pokey_sound(x, AUDF2_C, x->chan2lastFreq);
		x->chan2Nonzero = 1;
		}
	} else {
		x->chan2Nonzero = 0;
	}
	Update_pokey_sound(x, AUDC2_C, filterPokeyValue(value));
	x->value7_old = x->value7;
  }

  //Value 8
  if(x->value8 != x->value8_old) {
	value = x->value8;
	if (value < 2) value = 2;
	Update_pokey_sound(x, AUDF1_C, value);
	x->chan1lastFreq = value;
	x->value8_old = x->value8;
  }

  //Value 9
  if(x->value9 != x->value9_old) {
	value = x->value9;
	if(value != 0) {
		//are we transitioning?
		if(x->chan1Nonzero == 0) {
		//default the frequency channel to avoid clicks.
		Update_pokey_sound(x, AUDF1_C, x->chan1lastFreq);
		x->chan1Nonzero = 1;
		}
	} else {
		x->chan1Nonzero = 0;
	}
	Update_pokey_sound(x, AUDC1_C, filterPokeyValue(value));
	x->value9_old = x->value9;
  }

  Pokey_process(x, outL, (int)(w[2]));
  
  return (w + 4);
}  

void Pokey_sound_init(t_atari5200 *x, uint32 freq17, uint16 playback_freq)
{
   uint8 chan;
   int i;
   srand(0x2E59D10F);
   
   for(i=0; i< POLY17_SIZE; i++) { 
     bit17[i] = rand() & 1;
   }

   /* start all of the polynomial counters at zero */
   x->Poly_adjust = 0;
   x->P4 = 0;
   x->P5 = 0;
   x->P17 = 0;

   /* calculate the sample 'divide by N' value based on the playback freq. */
   x->Samp_n_max = ((uint32)freq17 << 8) / playback_freq;

   x->Samp_n_cnt[0] = 0;  /* initialize all bits of the sample */
   x->Samp_n_cnt[1] = 0;  /* 'divide by N' counter */

   x->Poly17_size = POLY17_SIZE;

   for (chan = CHAN1; chan <= CHAN4; chan++)
   {
      x->Outvol[chan] = 0;
	  x->Div_n_cnt[chan] = 0;
      x->Div_n_max[chan] = 0x7fffffffL;
      x->AUDC[chan] = 0;
      x->AUDF[chan] = 0;
   }

   x->AUDCTL = 0;

   x->Base_mult = DIV_64;
}


/*****************************************************************************/
/* Module:  Update_pokey_sound()                                             */
/* Purpose: To process the latest control values stored in the AUDF, AUDC,   */
/*          and AUDCTL registers.  It pre-calculates as much information as  */
/*          possible for better performance.  This routine has not been      */
/*          optimized.                                                       */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    September 22, 1996                                               */
/*                                                                           */
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

void Update_pokey_sound (t_atari5200 *x, uint16 addr, long val)
{
    uint32 new_val = 0;
    uint8 chan;
	uint8 chan_mask;

    /* determine which address was changed */
    switch (addr)
    {
       case AUDF1_C:
	   
          x->AUDF[CHAN1] = val;
          chan_mask = 1 << CHAN1;

          if (x->AUDCTL & CH1_CH2)    /* if ch 1&2 tied together */
            chan_mask |= 1 << CHAN2;   /* then also change on ch2 */
          break;

       case AUDC1_C:
          x->AUDC[CHAN1] = val;
          chan_mask = 1 << CHAN1;
          break;

       case AUDF2_C:
          x->AUDF[CHAN2] = val;
          chan_mask = 1 << CHAN2;
          break;

       case AUDC2_C:
          x->AUDC[CHAN2] = val;
          chan_mask = 1 << CHAN2;
          break;

       case AUDF3_C:
          x->AUDF[CHAN3] = val;
          chan_mask = 1 << CHAN3;

          if (x->AUDCTL & CH3_CH4)   /* if ch 3&4 tied together */
             chan_mask |= 1 << CHAN4;  /* then also change on ch4 */
          break;

       case AUDC3_C:
          x->AUDC[CHAN3] = val;
          chan_mask = 1 << CHAN3;
          break;

       case AUDF4_C:
          x->AUDF[CHAN4] = val;
          chan_mask = 1 << CHAN4;
          break;

       case AUDC4_C:
          x->AUDC[CHAN4] = val;
          chan_mask = 1 << CHAN4;
          break;

       case AUDCTL_C:
          x->AUDCTL = val;
          chan_mask = 15;		/* all channels */

          /* set poly17 counter to 9- or 17-bit */
          if (x->AUDCTL & POLY9)
             x->Poly17_size = POLY9_SIZE;
          else
             x->Poly17_size = POLY17_SIZE;

          /* determine the base multiplier for the 'div by n' calculations */
          if (x->AUDCTL & CLOCK_15)
             x->Base_mult = DIV_15;
          else
             x->Base_mult = DIV_64;

          break;

       default:
          chan_mask = 0;
          break;
    }

    /************************************************************/
    /* As defined in the manual, the exact Div_n_cnt values are */
    /* different depending on the frequency and resolution:     */
    /*    64 kHz or 15 kHz - AUDF + 1                           */
    /*    1 MHz, 8-bit -     AUDF + 4                           */
    /*    1 MHz, 16-bit -    AUDF[CHAN1]+256*AUDF[CHAN2] + 7    */
    /************************************************************/

    /* only reset the channels that have changed */

    if (chan_mask & (1 << CHAN1))
    {
       /* process channel 1 frequency */
       if (x->AUDCTL & CH1_179)
          new_val = x->AUDF[CHAN1] + 4;
       else
          new_val = (x->AUDF[CHAN1] + 1) * x->Base_mult;

       if (new_val != x->Div_n_max[CHAN1])
       {
          x->Div_n_max[CHAN1] = new_val;
          x->Div_n_cnt[CHAN1] = 0;
       }
    }

    if (chan_mask & (1 << CHAN2))
    {
       /* process channel 2 frequency */
       if (x->AUDCTL & CH1_CH2)
          if (x->AUDCTL & CH1_179)
             new_val = x->AUDF[CHAN2] * 256 + x->AUDF[CHAN1] + 7;
          else
             new_val = (x->AUDF[CHAN2] * 256 + x->AUDF[CHAN1] + 1) * x->Base_mult;
       else
          new_val = (x->AUDF[CHAN2] + 1) * x->Base_mult;

       if (new_val != x->Div_n_max[CHAN2])
       {
          x->Div_n_max[CHAN2] = new_val;
          x->Div_n_cnt[CHAN2] = 0;
       }
    }

    if (chan_mask & (1 << CHAN3))
    {
       /* process channel 3 frequency */
       if (x->AUDCTL & CH3_179)
          new_val = x->AUDF[CHAN3] + 4;
       else
          new_val= (x->AUDF[CHAN3] + 1) * x->Base_mult;

       if (new_val!= x->Div_n_max[CHAN3])
       {
          x->Div_n_max[CHAN3] = new_val;
          x->Div_n_cnt[CHAN3] = 0;
       }
    }

    if (chan_mask & (1 << CHAN4))
    {
       /* process channel 4 frequency */
       if (x->AUDCTL & CH3_CH4)
          if (x->AUDCTL & CH3_179)
             new_val = x->AUDF[CHAN4] * 256 + x->AUDF[CHAN3] + 7;
          else
             new_val = (x->AUDF[CHAN4] * 256 + x->AUDF[CHAN3] + 1) * x->Base_mult;
       else
          new_val = (x->AUDF[CHAN4] + 1) * x->Base_mult;

       if (new_val != x->Div_n_max[CHAN4])
       {
          x->Div_n_max[CHAN4] = new_val;
          x->Div_n_cnt[CHAN4] = 0;
       }
    }

    /* if channel is volume only, set current output */
    for (chan = CHAN1; chan <= CHAN4; chan++)
    {
       if (chan_mask & (1 << chan))
       {
          /* I've disabled any frequencies that exceed the sampling
             frequency.  There isn't much point in processing frequencies
             that the hardware can't reproduce.  I've also disabled 
             processing if the volume is zero. */

          /* if the channel is volume only */
          /* or the channel is off (volume == 0) */
          /* or the channel freq is greater than the playback freq */
          if ((x->AUDC[chan] & VOL_ONLY) ||
             ((x->AUDC[chan] & VOLUME_MASK) == 0) ||
              (x->Div_n_max[chan] < (x->Samp_n_max >> 8)))
          {
             /* then set the channel to the selected volume */
             x->Outvol[chan] = x->AUDC[chan] & VOLUME_MASK;
             /* and set channel freq to max to reduce processing */
             x->Div_n_max[chan] = 0x7fffffffL;
          }
       }
    } 
}
                                   
/*****************************************************************************/
/* Module:  Pokey_process()                                                  */
/* Purpose: To fill the output buffer with the sound output based on the     */
/*          pokey chip parameters.  This routine has not been optimized.     */
/*          Though it is not used by the program, I've left it for reference.*/
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    September 22, 1996                                               */
/*                                                                           */
/* Inputs:  *buffer - pointer to the buffer where the audio output will      */
/*                    be placed                                              */
/*          n - size of the playback buffer                                  */
/*                                                                           */
/* Outputs: the buffer will be filled with n bytes of audio - no return val  */
/*                                                                           */
/*****************************************************************************/

void Pokey_process (t_atari5200 *x, t_float *buffer, register uint16 n)
{
    unsigned long *div_n_ptr;
    uint32 *samp_cnt_w_ptr;
    uint32 event_min;
    uint8 next_event;
    uint8 cur_val;
    long *out_ptr;
    uint8 audc;
    uint8 toggle;

    /* set a pointer to the whole portion of the samp_n_cnt */
    samp_cnt_w_ptr = (uint32 *)((uint8 *)(&(x->Samp_n_cnt[0]))+1);
   
    /* set a pointer for optimization */
    out_ptr = x->Outvol;

    /* The current output is pre-determined and then adjusted based on each */
    /* output change for increased performance (less over-all math). */
    /* add the output values of all 4 channels */
    cur_val  = 2;                 /* start with a small offset */
    cur_val += *out_ptr++;
    cur_val += *out_ptr++;
    cur_val += *out_ptr++;
    cur_val += *out_ptr++;

    /* loop until the buffer is filled */

    while (n)
    {
       /* Normally the routine would simply decrement the 'div by N' */
       /* counters and react when they reach zero.  Since we normally */
       /* won't be processing except once every 80 or so counts, */
       /* I've optimized by finding the smallest count and then */
       /* 'accelerated' time by adjusting all pointers by that amount. */

       /* find next smallest event (either sample or chan 1-4) */
       next_event = SAMPLE;
       event_min = *samp_cnt_w_ptr; 

       /* Though I could have used a loop here, this is faster */
       div_n_ptr = x->Div_n_cnt;
       if (*div_n_ptr <= event_min)
       {
          event_min = *div_n_ptr;
          next_event = CHAN1;
       }
       div_n_ptr++;
       if (*div_n_ptr <= event_min)
       {
          event_min = *div_n_ptr;
          next_event = CHAN2;
       }
       div_n_ptr++;
       if (*div_n_ptr <= event_min)
       {
          event_min = *div_n_ptr;
          next_event = CHAN3;
       }
       div_n_ptr++;
       if (*div_n_ptr <= event_min)
       {
          event_min = *div_n_ptr;
          next_event = CHAN4;
       }

       /* decrement all counters by the smallest count found */
       /* again, no loop for efficiency */
       *div_n_ptr -= event_min;
       div_n_ptr--;
       *div_n_ptr -= event_min;
       div_n_ptr--;
       *div_n_ptr -= event_min;
       div_n_ptr--;
       *div_n_ptr -= event_min;

       *samp_cnt_w_ptr -= event_min;

       /* since the polynomials require a mod (%) function which is 
          division, I don't adjust the polynomials on the SAMPLE events,
          only the CHAN events.  I have to keep track of the change,
          though. */
       x->Poly_adjust += event_min;

       /* if the next event is a channel change */
      if (next_event != SAMPLE)
       {
          /* shift the polynomial counters */
          x->P4  = (x->P4  + x->Poly_adjust) % POLY4_SIZE;
          x->P5  = (x->P5  + x->Poly_adjust) % POLY5_SIZE;
          x->P17 = (x->P17 + x->Poly_adjust) % x->Poly17_size;
         
          /* reset the polynomial adjust counter to zero */
          x->Poly_adjust = 0;

          /* adjust channel counter */
          x->Div_n_cnt[next_event] += x->Div_n_max[next_event];

          /* get the current AUDC into a register (for optimization) */
          audc = x->AUDC[next_event];

          /* set a pointer to the current output (for opt...) */
          out_ptr = &(x->Outvol[next_event]);

          /* assume no changes to the output */
          toggle = FALSE;

          /* From here, a good understanding of the hardware is required */
          /* to understand what is happening.  I won't be able to provide */
          /* much description to explain it here. */

          /* if the output is pure or the output is poly5 and the poly5 bit */
          /* is set */
          if ((audc & NOTPOLY5) || bit5[x->P5])
          {
             /* if the PURE bit is set */
             if (audc & PURE)
             {               
                /* then simply toggle the output */
                toggle = TRUE;
             }
             /* otherwise if POLY4 is selected */
             else if (audc & POLY4)
             {
                /* then compare to the poly4 bit */
                toggle = (bit4[x->P4] == !(*out_ptr));
             }
             else
             {
                /* otherwise compare to the poly17 bit */
                toggle = (bit17[x->P17] == !(*out_ptr));
             }
          }

          /* At this point I haven't emulated the filters.  Though I don't
             expect it to be complicated, I don't believe this feature is
             used much anyway.  I'll work on it later. */
          if ((next_event == CHAN1) || (next_event == CHAN3))
          {
             /* INSERT FILTER HERE */
          }

          /* if the current output bit has changed */
          if (toggle)
          {
             if (*out_ptr)
             {
                /* remove this channel from the signal */
                cur_val -= *out_ptr;
                
                /* and turn the output off */
                *out_ptr = 0;
             }
             else
             {
                /* turn the output on */
                *out_ptr = audc & VOLUME_MASK;

                /* and add it to the output signal */
                cur_val += *out_ptr;
             }
          }
       }
       else /* otherwise we're processing a sample */
       {
          /* adjust the sample counter - note we're using the 24.8 integer
             which includes an 8 bit fraction for accuracy */
          *(x->Samp_n_cnt) += x->Samp_n_max;          
       }
	 
	   *(buffer++) = ((float)((cur_val << 2) + 100) - 100.0)/100.0;
	   n--;
	}
}                                    
       

