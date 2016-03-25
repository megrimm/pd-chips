/*
  Sound emulation from Gnuboy.
*/

#include <stdlib.h>
#include "m_pd.h"
#include "sound.h"

static t_class *gb_class;
// void *gb_class;  

typedef signed char Int8;
typedef signed int Int16;

#ifdef _WIN32
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

void *gb_new(void);  
t_int *gb_perform(t_int *w); 
void gb_dsp(t_gb *x, t_signal **sp, short *count);

void gb_bang(t_gb *x, long value); 
void gb_in1(t_gb *x, long value);
void gb_in2(t_gb *x, long value);
void gb_in3(t_gb *x, long value);
void gb_in4(t_gb *x, long value);
void gb_in5(t_gb *x, long value);
void gb_in6(t_gb *x, long value);
//void gb_in7(t_gb *x, long value);
void gb_assist(t_gb *gb, Object *b, long msg, long arg, char *s);

void gb_assist(t_gb *gb, Object *b, long msg, long arg, char *s) 
{
  if(msg == ASSIST_INLET) {
    switch(arg) {
	  case(0): 
	    sprintf(s, "%s", "Trigger the channel");
		break;
	  case(1):
	    sprintf(s, "%s", "Length count load register");
		break;
	  case(2):
	    sprintf(s, "%s", "Divide ratio frequency");
		break;
	  case(3): 
	    sprintf(s, "%s", "Shift register width");
		break;
	  case(4):
	    sprintf(s, "%s", "Shift clock frequency");
		break;
	  case(5):
	    sprintf(s, "%s", "Volume sweep length; direction (0: down; 1: up)");
		break;
	  case(6):
	    sprintf(s, "%s", "Initial volume for volume sweep");
		break;
	}
  } else if(msg == ASSIST_OUTLET) {
    switch(arg) {
	  case(0):
	    sprintf(s, "%s", "Left Channel");
	    break;
	  case(1):
	    sprintf(s, "%s", "Right Channel");
	    break;
	}
  }
}

void main(void) {    
  setup((t_messlist **)&gb_class, (method)gb_new, (method)dsp_free, (short)sizeof(t_gb), 0L, 0);
  addbang((method)gb_bang); 
  addinx((method)gb_in1, 1);
  addinx((method)gb_in2, 2);
  addinx((method)gb_in3, 3);
  addinx((method)gb_in4, 4);
  addinx((method)gb_in5, 5);
  addinx((method)gb_in6, 6);
  //addinx((method)gb_in7, 7);
  addmess((method)gb_dsp, "dsp", A_CANT, 0);   
  finder_addclass("All Objects", "gb~");
  addmess((method)gb_assist, "assist", A_CANT, 0); 
  dsp_initclass();
}  

void *gb_new(void)
{  
  t_gb *x = (t_gb *)pd_new(gb_class);  
  dsp_setup((t_object *)x, 0);          // no inlet 
  outlet_new((t_object *)x, "signal");  // outlet
  outlet_new((t_object *)x, "signal");  // outlet
  intin(x, 1);
  intin(x, 2);
  intin(x, 3);
  intin(x, 4);
  intin(x, 5);
  intin(x, 6);
  //intin(x, 7);
  sound_reset(x);
  sound_write(x, RI_NR44, 128);
  return (x); 
}   

void gb_dsp(t_gb *x, t_signal **sp, short *count) 
{   
  dsp_add(gb_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x); 
}  

/*
#define RI_NR10 0x10
#define RI_NR11 0x11
#define RI_NR12 0x12
#define RI_NR13 0x13
#define RI_NR14 0x14
#define RI_NR21 0x16
#define RI_NR22 0x17
#define RI_NR23 0x18
#define RI_NR24 0x19
#define RI_NR30 0x1A
#define RI_NR31 0x1B
#define RI_NR32 0x1C
#define RI_NR33 0x1D
#define RI_NR34 0x1E
#define RI_NR41 0x20
#define RI_NR42 0x21
#define RI_NR43 0x22
#define RI_NR44 0x23
#define RI_NR50 0x24
#define RI_NR51 0x25
#define RI_NR52 0x26

*/

/*
NR41
5-0	Length counter load register

NR42
7-4	(Initial) Channel Volume
3	Volume sweep direction (0: down; 1: up)
2-0	Length of each step in sweep (if 0, sweeping is off)

NR43
7-4	Shift clock frequency (s)
3	Shift Register width (0: 15 bits; 1: 7 bits)
2-0	Dividing Ratio of frequency (r)
*/

void gb_bang(t_gb *x, long value) 
{
   int8 v = value & 0x00;
   x->cnt_enab = v;
   sound_write(x, RI_NR42, (x->vol << 4) | (x->vol_sweep_dir << 3) | (x->sweep_len));
}

void gb_in1(t_gb *x, long value) 
{
  int8 v = value & 0x0F;
  x->vol = v;
}

void gb_in2(t_gb *x, long value) 
{
   int8 v = value & 0x07;
   if(value < 0) {
     x->vol_sweep_dir = 0;
   } else {
     x->vol_sweep_dir = 1;
   }
   x->sweep_len = v;
}

void gb_in3(t_gb *x, long value) 
{
   int8 v = value & 0x0F;
   x->shift_clk = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gb_in4(t_gb *x, long value) 
{
   int8 v = value & 0x01;
   x->shift_reg_width = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gb_in5(t_gb *x, long value) 
{
   int8 v = value & 0x07;
   x->div_freq = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gb_in6(t_gb *x, long value) 
{
  int8 v = value & 0x1F;
  sound_write(x, RI_NR41, v);
}  

/*
void gb_in7(t_gb *x, long value) 
{
    int8 v = value & 0x01;
	x->main_on = 1;
	x->cnt_enab = v;
    sound_write(x, RI_NR44, (x->main_on << 7) | (x->cnt_enab << 6));
} 
*/ 

t_int *gb_perform(t_int *w) 
{     
  t_float *outL = (t_float *)(w[1]);
  t_float *outR = (t_float *)(w[2]); 
  t_gb *x = (t_gb *)(w[4]);  

  sound_mix(x, outL, outR, (int)(w[3]));

  return (w + 5);
}  

