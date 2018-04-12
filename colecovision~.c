/*
  Original SN76496.c 
  by Nicola Salmoria
  
  Max/MSP integration by Kyle Buza
*/

#ifdef PD
#include "m_pd.h"
/* Max -> Pd porting help */
#define t_pxobject t_object
t_class *colecovision_class;
#else /* Max */
#include "ext.h"
#include "z_dsp.h"
void *colecovision_class;  
#endif

#include <stdlib.h>
#include <string.h>

#define STEP        0x10000
#define NG_PRESET   0x0F35
#define MAX_OUTPUT  0x7FFF
#define FB_WNOISE   0x12000
#define FB_PNOISE   0x08000

typedef signed char Int8;
typedef signed int Int16;

typedef struct _colecovision
{ 
  t_pxobject x_obj;
  long UpdateStep;
  long bufsize;
  long NoiseFB;
  long *psg_buffer[2];
  long Output[4];
  long Volume[4];
  long RNG;
  long Count[4];
  long Period[4];
  long VolTable[16];
  long LastRegister;
  long Register[8];
  long lastLatched;
#ifdef PD
  t_float Ch1Tone;  
  t_float Ch1Volume;  
  t_float Ch2Tone;  
  t_float Ch2Volume;  
  t_float Ch3Tone;  
  t_float Ch3Volume;  
  t_float NoiseChShiftRate;  
  t_float NoiseChVolume;  
  t_float Unused;  
#endif
} t_colecovision;  

t_int *colecovision_perform(t_int *w); 
void colecovision_dsp(t_colecovision *x, t_signal **sp, short *count);  
void colecovision_in1(t_colecovision *x, long value);
void colecovision_in2(t_colecovision *x, long value);
void colecovision_in3(t_colecovision *x, long value);
void colecovision_in4(t_colecovision *x, long value);
void colecovision_in5(t_colecovision *x, long value);
void colecovision_in6(t_colecovision *x, long value);
void colecovision_in7(t_colecovision *x, long value);
void colecovision_in8(t_colecovision *x, long value);
void colecovision_in9(t_colecovision *x, long value);

void audio_init(t_colecovision *x);
void SN76496_init(t_colecovision *x);
void SN76496Write(t_colecovision *x, int data);
void SN76496Update(t_colecovision *x, long *buffer[2],int length, unsigned char mask);

#ifdef PD
void *colecovision_new(void)
{  
  t_colecovision *x = (t_colecovision *)pd_new(colecovision_class);  
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  floatinlet_new(&x->x_obj, &x->Ch1Tone);
  floatinlet_new(&x->x_obj, &x->Ch1Volume);
  floatinlet_new(&x->x_obj, &x->Ch2Tone);
  floatinlet_new(&x->x_obj, &x->Ch2Volume);
  floatinlet_new(&x->x_obj, &x->Ch3Tone);
  floatinlet_new(&x->x_obj, &x->Ch3Volume);
  floatinlet_new(&x->x_obj, &x->NoiseChShiftRate);
  floatinlet_new(&x->x_obj, &x->NoiseChVolume);
  floatinlet_new(&x->x_obj, &x->Unused);
  x->lastLatched = 0;
  audio_init(x);
  return (x); 
}   
void colecovision_tilde_setup(void) {    
	colecovision_class = class_new(gensym("colecovision~"),
			(t_newmethod)colecovision_new, 0, sizeof(t_colecovision), CLASS_DEFAULT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(colecovision_class, t_colecovision, Ch1Tone);
	class_addmethod(colecovision_class, (t_method)colecovision_dsp, gensym("dsp"), 0);
}  
#else /* Max */
void colecovision_assist(t_colecovision *colecovision, Object *b, long msg, long arg, char *s) 
{
  if(msg == ASSIST_INLET) {
    switch(arg) {
	  case(1): 
	    sprintf(s, "%s", "Channel 1 Tone");
		break;
	  case(2):
	    sprintf(s, "%s", "Channel 1 Volume");
		break;
	  case(3):
	    sprintf(s, "%s", "Channel 2 Tone");
		break;
	  case(4): 
	    sprintf(s, "%s", "Channel 2 Volume");
		break;
	  case(5):
	    sprintf(s, "%s", "Channel 3 Tone");
		break;
	  case(6):
	    sprintf(s, "%s", "Channel 3 Volume");
		break;
	  case(7):
	    sprintf(s, "%s", "Nosie Channel Shift Rate");
		break;
	  case(8):
	    sprintf(s, "%s", "Noise Channel Volume");
		break;
	  case(9):
	    sprintf(s, "%s", "Not currently used.");
		break;
	}
  } else if(msg == ASSIST_OUTLET) {
    switch(arg) {
	  case(0):
	    sprintf(s, "%s", "L audio signal");
	    break;
	  case(1):
	    sprintf(s, "%s", "R audio signal'");
	    break;
	}
  }
}

void *colecovision_new(void)
{  
  t_colecovision *x = (t_colecovision *)newobject(colecovision_class);  
  dsp_setup((t_pxobject *)x, 0);          // no inlet 
  outlet_new((t_pxobject *)x, "signal");  // outlet
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
  x->lastLatched = 0;
  audio_init(x);
  return (x); 
}   

void main(void) {    
  setup((t_messlist **)&colecovision_class, (method)colecovision_new, (method)dsp_free, (short)sizeof(t_colecovision), 0L, 0);
  addinx((method)colecovision_in1, 1);
  addinx((method)colecovision_in2, 2);
  addinx((method)colecovision_in3, 3);
  addinx((method)colecovision_in4, 4);
  addinx((method)colecovision_in5, 5);
  addinx((method)colecovision_in6, 6);
  addinx((method)colecovision_in7, 7);
  addinx((method)colecovision_in8, 8);
  addinx((method)colecovision_in9, 9);
  addmess((method)colecovision_dsp, "dsp", A_CANT, 0);   
  finder_addclass("All Objects", "colecovision~");
  addmess((method)colecovision_assist, "assist", A_CANT, 0); 
  dsp_initclass();
}  
#endif /* PD */

void colecovision_dsp(t_colecovision *x, t_signal **sp, short *count) 
{   
  dsp_add(colecovision_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x); 
}  

void colecovision_in1(t_colecovision *x, long value) 
{
}

void colecovision_in2(t_colecovision *x, long value) 
{
  //Noise Channel volume
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x000F) value = 0x000F;
  
  value ^= 0x000F;

  if(x->lastLatched != 0x0080) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0080;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (3 << 5) | (1 << 4) | value;
  } else {
	value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);

}

void colecovision_in3(t_colecovision *x, long value) 
{
  //Noise Channel shift rate
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x00FF) value = 0x00FF;
  
  if(x->lastLatched != 0x0040) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0040;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (3 << 5) | (0 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
}

void colecovision_in4(t_colecovision *x, long value) 
{
  //Volume Channel 3
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x000F) value = 0x000F;
  
  value ^= 0x000F;

  if(x->lastLatched != 0x0020) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0020;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (2 << 5) | (1 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);

}

void colecovision_in5(t_colecovision *x, long value) 
{
  //Tone Channel 3
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x00FF) value = 0x00FF;
  
  if(x->lastLatched != 0x0010) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0010;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (2 << 5) | (0 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
}

void colecovision_in6(t_colecovision *x, long value) 
{
  //Volume Channel 2
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x000F) value = 0x000F;
  
  value ^= 0x000F;
  
  if(x->lastLatched != 0x0008) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0008;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (1 << 5) | (1 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
}  
void colecovision_in7(t_colecovision *x, long value) 
{
  //Tone Channel 2
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x00FF) value = 0x00FF;
  
  if(x->lastLatched != 0x0004) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0004;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (1 << 5) | (0 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
}

void colecovision_in8(t_colecovision *x, long value) 
{
  //Volume Channel 1
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x000F) value = 0x000F;
  
  value ^= 0x000F;
  
  if(x->lastLatched != 0x0002) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0002;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (0 << 5) | (1 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
  
}

void colecovision_in9(t_colecovision *x, long value) 
{
  //Tone Channel 1
  int thisData = 0;
  
  if(value < 0) value = 0;
  else if (value > 0x00FF) value = 0x00FF;
  
  if(x->lastLatched != 0x0001) {
    //e.g. NOT me...
	//latch this channel, and set the bit
	x->lastLatched = 0x0001;
	value &= 0x000F;
	//latch -- channel -- type -- data
	thisData = (1 << 7) | (0 << 5) | (0 << 4) | value;
  } else {
    value &= 0x003F;
	thisData = (0 << 7) | value;
  }
  
  SN76496Write(x, thisData);
}

/*
  x->lastLatched has the following structure:
  
      0      0      0      0      0      0      0      0
      |      |      |      |      |      |      |      |
      V      V      V      V      V      V      V      V
     VC3    NC3    VC2    TC2    VC1    TC1    VC0    TC0
*/

t_int *colecovision_perform(t_int *w) 
{
  int count;    
  t_float *outL = (t_float *)(w[1]);
  t_float *outR = (t_float *)(w[2]);
  int n = (int)(w[3]);
  t_colecovision *x = (t_colecovision *)(w[4]);

#ifdef PD
  // get the data from the inlets
  colecovision_in1(x, (long)x->Ch1Tone);
  colecovision_in2(x, (long)x->Ch1Volume);
  colecovision_in3(x, (long)x->Ch2Tone);
  colecovision_in4(x, (long)x->Ch2Volume);
  colecovision_in5(x, (long)x->Ch3Tone);
  colecovision_in6(x, (long)x->Ch3Volume);
  colecovision_in7(x, (long)x->NoiseChShiftRate);
  colecovision_in8(x, (long)x->NoiseChVolume);
  colecovision_in9(x, (long)x->Unused);
#endif

  SN76496Update(x, x->psg_buffer, n, 0xFF /* MODIFY??? */);
  
  //for(count = 0; count < snd.bufsize; count += 1)
  for(count = 0; count < n; count += 1)
  {
	signed short left   = 0;
	signed short right  = 0;
	left  += x->psg_buffer[0][count];
	right += x->psg_buffer[1][count];
	*outR++ = ((float)(right/16384.0)) * 1.06;
	*outL++ = ((float)(left/16384.0)) * 1.06;
  }
  
  return (w + 5);
}  

void audio_init(t_colecovision *x)
{
    int rate = 22050;

    /* Calculate buffer size in samples */
    x->bufsize = (rate / 60);

    /* SN76489 sound stream */
    x->psg_buffer[0] = (long *)malloc(x->bufsize * 2);
    x->psg_buffer[1] = (long *)malloc(x->bufsize * 2);
    if(!(x->psg_buffer[0]) || !(x->psg_buffer[1])) {
	  post("SN76489: Error allocating audio buffers.");
	  return;
	}
    memset(x->psg_buffer[0], 0, x->bufsize * 2);
    memset(x->psg_buffer[1], 0, x->bufsize * 2);

    /* Set up SN76489 emulation */
    SN76496_init(x);
}

void SN76496_init(t_colecovision *x)
{
    int i;
	double out;  
	int sample_rate = 22050;
	int gain = (255 >> 8) & 0xFF;

	//R->SampleRate = sample_rate;
	//R->UpdateStep = ((double)STEP * R->SampleRate * 16) / 3579545;
	x->UpdateStep = ((double)STEP * sample_rate * 16) / 3579545;

	for (i = 0;i < 4;i++) x->Volume[i] = 0;
	x->LastRegister = 0;
	for (i = 0;i < 8;i+=2)
	{       
		x->Register[i] = 0;
		x->Register[i + 1] = 0x0f;      /* volume = 0 */
	}

	for (i = 0;i < 4;i++)
	{
		x->Output[i] = 0;
		x->Period[i] = x->Count[i] = x->UpdateStep;
	}
	x->RNG = NG_PRESET;
	x->Output[3] = x->RNG & 1;
	                           
	gain &= 0xff;           
	out = MAX_OUTPUT / 3;   
	while (gain-- > 0) out *= 1.023292992;     
                                        
	for (i = 0;i < 15;i++)          
	{                                       
		if (out > MAX_OUTPUT / 3) x->VolTable[i] = MAX_OUTPUT / 3;
		else x->VolTable[i] = out;
		out /= 1.258925412;
	}               
                        
	x->VolTable[15] = 0; 
}

void SN76496Write(t_colecovision *x, int data)
{
	if (data & 0x80)
	{
		int r = (data & 0x70) >> 4;
		int c = r/2;

		x->LastRegister = r;
		x->Register[r] = (x->Register[r] & 0x3f0) | (data & 0x0f);
		switch (r)
		{
			case 0: /* tone 0 : frequency */
			case 2: /* tone 1 : frequency */
			case 4: /* tone 2 : frequency */
				x->Period[c] = x->UpdateStep * x->Register[r];
				if (x->Period[c] == 0) x->Period[c] = x->UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((x->Register[6] & 0x03) == 0x03) x->Period[3] = 2 * x->Period[2];
				}
				break;
			case 1: /* tone 0 : volume */
			case 3: /* tone 1 : volume */
			case 5: /* tone 2 : volume */
			case 7: /* noise  : volume */
				x->Volume[c] = x->VolTable[data & 0x0f];
				break;
			case 6: /* noise  : frequency, mode */
			{
				int n = x->Register[6];
				x->NoiseFB = (n & 4) ? FB_WNOISE : FB_PNOISE;
				n &= 3;
				/* N/512,N/1024,N/2048,Tone #3 output */
				x->Period[3] = (n == 3) ? 2 * x->Period[2] : (x->UpdateStep << (5+n));

				/* reset noise shifter */
				x->RNG = NG_PRESET;
				x->Output[3] = x->RNG & 1;
			}
			break;
		}
	} else
	{
		int r = x->LastRegister;
		int c = r/2;

		switch (r)
		{
			case 0: /* tone 0 : frequency */
			case 2: /* tone 1 : frequency */
			case 4: /* tone 2 : frequency */
				x->Register[r] = (x->Register[r] & 0x0f) | ((data & 0x3f) << 4);
				x->Period[c] = x->UpdateStep * x->Register[r];
				if (x->Period[c] == 0) x->Period[c] = x->UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((x->Register[6] & 0x03) == 0x03) x->Period[3] = 2 * x->Period[2];
				}
				break;
		}
	}
}

void SN76496Update(t_colecovision *x, long *buffer[2],int length, unsigned char mask)
{
    int i, j;
    int buffer_index = 0;
	
	/* If the volume is 0, increase the counter */
	for (i = 0;i < 4;i++)
	{
		if (x->Volume[i] == 0)
		{
			/* note that I do count += length, NOT count = length + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (x->Count[i] <= length*STEP) x->Count[i] += length*STEP;
		}
	}

	while (length > 0)
	{
		int vol[4];
        unsigned int out[2];
		int left;

		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0;

		for (i = 0;i < 3;i++)
		{
			if (x->Output[i]) vol[i] += x->Count[i]; 
			
			x->Count[i] -= STEP;
			/* Period[i] is the half period of the square wave. Here, in each */
			/* loop I add Period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by Period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, Output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (x->Count[i] <= 0)
			{
				x->Count[i] += x->Period[i];
				if (x->Count[i] > 0)
				{
					x->Output[i] ^= 1;
					if (x->Output[i]) vol[i] += x->Period[i];
					break;
				}
				x->Count[i] += x->Period[i];
				vol[i] += x->Period[i];
			}
			
			if (x->Output[i]) vol[i] -= x->Count[i];
		}

		left = STEP;
		do
		{
			int nextevent;

			if (x->Count[3] < left) nextevent = x->Count[3];
			else nextevent = left;

			if (x->Output[3]) vol[3] += x->Count[3];
			x->Count[3] -= nextevent;
			if (x->Count[3] <= 0)
			{
				if (x->RNG & 1) x->RNG ^= x->NoiseFB;
				x->RNG >>= 1;
				x->Output[3] = x->RNG & 1;
				x->Count[3] += x->Period[3];
				if (x->Output[3]) vol[3] += x->Period[3];
			}
			if (x->Output[3]) vol[3] -= x->Count[3];
				left -= nextevent;
		} while (left > 0);

		out[0] = out[1] = 0;
		for(j = 0; j < 4; j += 1)
		{
			int k = vol[j] * x->Volume[j];
			if(mask & (1 << (4+j))) out[0] += k;
			if(mask & (1 << (0+j))) out[1] += k;
		}

		if(out[0] > MAX_OUTPUT * STEP) out[0] = MAX_OUTPUT * STEP;
		if(out[1] > MAX_OUTPUT * STEP) out[1] = MAX_OUTPUT * STEP;
		buffer[0][buffer_index] = out[0] / STEP;
		buffer[1][buffer_index] = out[1] / STEP;

		/* Next sample set */
		buffer_index += 1;
		length--;
	}
}
