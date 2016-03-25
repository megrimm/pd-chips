/*
  Sound emulation from Gnuboy.
*/

//#ifdef PD
#include "m_pd.h"
//#else /* Max */
//#include "ext.h"
//#include "z_dsp.h"
//#endif

#include "sound.h"

void *gameboy_class;  

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

void *gameboy_new(void);  
t_int *gameboy_perform(t_int *w); 
void gameboy_dsp(t_gameboy *x, t_signal **sp, short *count);

void gameboy_bang(t_gameboy *x, long value); 
void gameboy_in1(t_gameboy *x, long value);
void gameboy_in2(t_gameboy *x, long value);
void gameboy_in3(t_gameboy *x, long value);
void gameboy_in4(t_gameboy *x, long value);
void gameboy_in5(t_gameboy *x, long value);
void gameboy_in6(t_gameboy *x, long value);
//void gameboy_in7(t_gameboy *x, long value);
void gameboy_assist(t_gameboy *gameboy, Object *b, long msg, long arg, char *s);

/*
#if MSP
void gameboy_assist(t_gameboy *gameboy, Object *b, long msg, long arg, char *s) 
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
  } 
#endif

#if PD
void gameboy_assist (t_pvtuner *x, void *b, long msg, long arg, char *dst)
{
	post("INLETS: input pitch_factor synthesis_threshold");
	post("ARGUMENTS: lo_freq hi_freq");
}
#endif



  else if(msg == ASSIST_OUTLET) {
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
*/

/*
void main(void) {    
  setup((t_messlist **)&gameboy_class, (method)gameboy_new, (method)dsp_free, (short)sizeof(t_gameboy), 0L, 0);
  addbang((method)gameboy_bang); 
  addinx((method)gameboy_in1, 1);
  addinx((method)gameboy_in2, 2);
  addinx((method)gameboy_in3, 3);
  addinx((method)gameboy_in4, 4);
  addinx((method)gameboy_in5, 5);
  addinx((method)gameboy_in6, 6);
  //addinx((method)gameboy_in7, 7);
  addmess((method)gameboy_dsp, "dsp", A_CANT, 0);   
  finder_addclass("All Objects", "gameboy~");
  addmess((method)gameboy_assist, "assist", A_CANT, 0); 
  dsp_initclass();
}  
*/

void gameboy_tilde_setup(void)
{
	gameboy_class = class_new(gensym("gameboy~"), (t_newmethod)gameboy_new, 
							  (t_method)gameboy_free ,sizeof(t_gameboy), 0L, 0);
	CLASS_MAINSIGNALIN(pvtuner_class, t_pvtuner, x_f );
	addbang((method)gameboy_bang); 
  	addinx((method)gameboy_in1, 1);
  	addinx((method)gameboy_in2, 2);
  	addinx((method)gameboy_in3, 3);
  	addinx((method)gameboy_in4, 4);
  	addinx((method)gameboy_in5, 5);
  	addinx((method)gameboy_in6, 6);
  	//addinx((method)gameboy_in7, 7);
  	class_addmethod(gameboy_class, (t_method)gameboy_dsp, gensym("dsp"), 0);
	class_addmethod(gameboy_class, (t_method)gameboy_assist, gensym("assist"), 0);
	post(version);
}

void *gameboy_new(void)
{  
  t_gameboy *x = (t_gameboy *)pd_new(gameboy_class);  
  //dsp_setup((t_object *)x, 0);          // no inlet 
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

void gameboy_dsp(t_gameboy *x, t_signal **sp, short *count) 
{   
  dsp_add(gameboy_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x); 
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

void gameboy_bang(t_gameboy *x, long value) 
{
   int8 v = value & 0x00;
   x->cnt_enab = v;
   sound_write(x, RI_NR42, (x->vol << 4) | (x->vol_sweep_dir << 3) | (x->sweep_len));
}

void gameboy_in1(t_gameboy *x, long value) 
{
  int8 v = value & 0x0F;
  x->vol = v;
}

void gameboy_in2(t_gameboy *x, long value) 
{
   int8 v = value & 0x07;
   if(value < 0) {
     x->vol_sweep_dir = 0;
   } else {
     x->vol_sweep_dir = 1;
   }
   x->sweep_len = v;
}

void gameboy_in3(t_gameboy *x, long value) 
{
   int8 v = value & 0x0F;
   x->shift_clk = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gameboy_in4(t_gameboy *x, long value) 
{
   int8 v = value & 0x01;
   x->shift_reg_width = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gameboy_in5(t_gameboy *x, long value) 
{
   int8 v = value & 0x07;
   x->div_freq = v;
   sound_write(x, RI_NR43, (x->shift_clk << 4) | (x->shift_reg_width << 3) | (x->div_freq));
}

void gameboy_in6(t_gameboy *x, long value) 
{
  int8 v = value & 0x1F;
  sound_write(x, RI_NR41, v);
}  

/*
void gameboy_in7(t_gameboy *x, long value) 
{
    int8 v = value & 0x01;
	x->main_on = 1;
	x->cnt_enab = v;
    sound_write(x, RI_NR44, (x->main_on << 7) | (x->cnt_enab << 6));
} 
*/ 

t_int *gameboy_perform(t_int *w) 
{     
  t_float *outL = (t_float *)(w[1]);
  t_float *outR = (t_float *)(w[2]); 
  t_gameboy *x = (t_gameboy *)(w[4]);  

  sound_mix(x, outL, outR, (int)(w[3]));

  return (w + 5);
}  

