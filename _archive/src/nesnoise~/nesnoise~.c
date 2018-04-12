#include <stdlib.h>
#include "m_pd.h"
#include "nesapu2.h"

static t_class *nesnoise_class;
// void *nesnoise_class;  

typedef signed char Int8;

/*

TODO:

move apu and statics to t_nesnoise_noise struct
don't call apu_setchan.  only enable nose channel.

*/

void *nesnoise_new(void);  
t_int *nesnoise_perform(t_int *w); 
void nesnoise_dsp(t_nesnoise *x, t_signal **sp, short *count);  
void nesnoise_bang(t_nesnoise *x);
void nesnoise_int(t_nesnoise *x, long value);
void nesnoise_in1(t_nesnoise *x, long value);
void nesnoise_in2(t_nesnoise *x, long value);
void nesnoise_in3(t_nesnoise *x, long value);
void nesnoise_in4(t_nesnoise *x, long value);
void nesnoise_in5(t_nesnoise *x, long value);
//void nesnoise_assist(t_nesnoise *nesnoise, Object *b, long msg, long arg, char *s);
void nesnoise_assist(t_nesnoise *nesnoise, Object *b, long msg, long arg, char *s) 
{
  if(msg == ASSIST_INLET) {
    switch(arg) {
	  case(0): 
	    sprintf(s, "%s", "Sample type (0-1)");
		break;
	  case(1): 
	    sprintf(s, "%s", "Length count");
		break;
	  case(2):
	    sprintf(s, "%s", "Sample Rate");
		break;
	  case(3):
	    sprintf(s, "%s", "Delay on/off");
		break;
	  case(4): 
	    sprintf(s, "%s", "Loop on/off");
		break;
	  case(5):
	    sprintf(s, "%s", "Volume/Hold");
		break;
	}
  } else if(msg == ASSIST_OUTLET) {
    switch(arg) {
	  case(0):
	    sprintf(s, "%s", "Sound signal");
	    break;
	}
  }
}

void main(void) {    
  setup((t_messlist **)&nesnoise_class, (method)nesnoise_new, (method)dsp_free, (short)sizeof(t_nesnoise), 0L, 0);
  addbang((method)nesnoise_bang);
  addint((method)nesnoise_int);    
  addinx((method)nesnoise_in1, 1);
  addinx((method)nesnoise_in2, 2);
  addinx((method)nesnoise_in3, 3);
  addinx((method)nesnoise_in4, 4);
  addinx((method)nesnoise_in5, 5);
  addmess((method)nesnoise_assist, "assist", A_CANT, 0);
  addmess((method)nesnoise_dsp, "dsp", A_CANT, 0);    
  dsp_initclass();
}  

void *nesnoise_new(void)
{  

  t_nesnoise *x = (t_nesnoise *)pd_new(nesnoise_class);  //PD
  inlet_new(&x->x_obj, &x->x_obj.ob_pd,gensym("signal"), gensym("signal")); //PD
  outlet_new(&x->x_obj, gensym("signal")); //PD
  
  //t_nesnoise *x = (t_nesnoise *)newobject(nesnoise_class);  //MSP
  //dsp_setup((t_pxobject *)x, 1);    // left inlet  MSP
  //outlet_new((t_pxobject *)x, "signal");  // left outlet MSP
  
  intin(x, 1);
  intin(x, 2);
  intin(x, 3);
  intin(x, 4);
  intin(x, 5);
  //intin(x, 6);
  //intin(x, 7);
  x->samprate = 0;
  x->decayEnab = 0;
  x->loop = 0;
  x->decayEnab = 1;
  x->volume = 0;
  x->prev_sample = 0;
  apu_create(x, 1, APU_BASEFREQ, 44100, 60, 16);
  apu_write(x, APU_WRD2, (x->samprate | x->samptype));
  apu_write(x, APU_WRD0, (x->loop | x->decayEnab | x->volume));
  apu_write(x, APU_SMASK, (0x1 << 3));
  return (x); 
}   

void nesnoise_dsp(t_nesnoise *x, t_signal **sp, short *count) 
{   
  dsp_add(nesnoise_perform, 3, sp[1]->s_vec, sp[0]->s_n, x); 
} 

/*  WHITE NOISE CHANNEL
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/ 

/*
    +-----------------------+
	|	bit3=0  		|
	+-------+---------------+
	|	|frames  		|
	|bits	+-------+-------+
	|4-6	|bit7=0	|bit7=1	|
	+-------+-------+-------+
	|0  	|05  	|06	|
	|1  	|0A 	|0C	|
	|2  	|14 	|18	|
	|3  	|28 	|30	|
	|4  	|50 	|60	|
	|5  	|1E 	|24	|
	|6  	|07 	|08	|
	|7  	|0E 	|10	|
	+-------+-------+-------+

	+---------------+
	|	bit3=1   	|
	+-------+-------+
	|bits	|	    |
	|4-7	|frames	|
	+-------+-------+
	|0	    |7F	|
	|1	    |01	|
	|2	    |02	|
	|3	    |03	|
	|4	    |04	|
	|5	    |05	|
	|6	    |06	|
	|7	    |07	|
	|8	    |08	|
	|9	    |09	|
	|A	    |0A	|
	|B   	|0B	|
	|C  	|0C	|
	|D  	|0D	|
	|E  	|0E	|
	|F  	|0F	|
	+-------+-------+
*/

void nesnoise_int(t_nesnoise *x, long value) 
{
   int8 v = (int8)value;
   v = ((v & 0x1) << 7);
   x->samptype = v;
   //post("samptype %d", v);
   apu_write(x, APU_WRD2, (x->samprate | x->samptype));
}

void nesnoise_in1(t_nesnoise *x, long value) 
{
   int8 v = (int8)value;
   v = (v & 0x0F);
   x->volume = v;
   //post("samptype %d", v);
   apu_write(x, APU_WRD0, (x->loop | x->decayEnab | x->volume));
   //post("volume %d", value);
}

void nesnoise_in2(t_nesnoise *x, long value) 
{
   int8 v = (int8)value;
   v = ((v & 0x1) << 5);
   x->loop = v;
   //post("samptype %d", v);
   apu_write(x, APU_WRD0, (x->loop | x->decayEnab | x->volume));
   //post("looping %d", value & 0x1);
}

void nesnoise_in3(t_nesnoise *x, long value) 
{
   int8 v = (int8)value;
   v ^= 0x1;
   v = ((v & 0x1) << 4);
   x->decayEnab = v;
   //post("samptype %d", v);
   apu_write(x, APU_WRD0, (x->loop | x->decayEnab | x->volume));
   //post("enabling decay %d", value & 0x1);
}

void nesnoise_in4(t_nesnoise *x, long value) 
{ 
   int8 v = (int8)value;
   v = (v & 0x0F);
   x->samprate = v;
   //post("samprate %d", v);
   apu_write(x, APU_WRD2, (x->samprate | x->samptype));
}

void nesnoise_in5(t_nesnoise *x, long value) 
{ 
  int8 v = (int8) value;
  v = (v << 3);
  x->lencnt = v;
  //post("len cnt %d", value);
  apu_write(x, APU_WRD3, x->lencnt);
}

void nesnoise_in6(t_nesnoise *x, long value) 
{

}

void nesnoise_in7(t_nesnoise *x, long value) 
{

}

void nesnoise_bang(t_nesnoise *x) 
{   
}  

t_int *nesnoise_perform(t_int *w) 
{      
  t_float *outL = (t_float *)(w[1]);
  t_nesnoise *x = (t_nesnoise *)(w[3]);
   
  apu_process(x, outL,  (int)(w[2]));  
  
  return (w + 4);
}  