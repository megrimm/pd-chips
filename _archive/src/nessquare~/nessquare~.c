#ifdef PD
#include "m_pd.h"
/* Max -> Pd porting help */
#define t_pxobject t_object
t_class *nes_square_class;
#else /* Max */
#include "ext.h"
#include "z_dsp.h"
void *nes_square_class; 
#endif

#include "nes_apu2.h"

 

typedef signed char Int8;

void *nes_square_new(void);  
t_int *nes_square_perform(t_int *w); 
void nes_square_dsp(t_nes_square *x, t_signal **sp, short *count);  
void nes_square_int(t_nes_square *x, long value);
void nes_square_in1(t_nes_square *x, long value);
void nes_square_in2(t_nes_square *x, long value);
void nes_square_in3(t_nes_square *x, long value);
void nes_square_in4(t_nes_square *x, long value);
void nes_square_in5(t_nes_square *x, long value);
void nes_square_in6(t_nes_square *x, long value);
void nes_square_in7(t_nes_square *x, long value);

void nes_square_assist(t_nes_square *nes_square, Object *b, long msg, long arg, char *s);

void nes_square_assist(t_nes_square *nes_square, Object *b, long msg, long arg, char *s) 
{
  if(msg == ASSIST_INLET) {
    switch(arg) {
	  case(0): 
	    sprintf(s, "%s", "Volume/Envelope decay rate. (loop enable if < 0) ");
		break;
	  case(1): 
	    sprintf(s, "%s", "Length counter clock disable / Envelope decay looping enable");
		break;
	  case(2):
	    sprintf(s, "%s", "Duty cycle type");
		break;
	  case(3):
	    sprintf(s, "%s", "Right shift amount");
		break;
	  case(4): 
	    sprintf(s, "%s", "Sweep update rate. (Freq shift decrease/increase if < 0 / > 0)");
		break;
	  case(5):
	    sprintf(s, "%s", "LSB frequency");
		break;
	  case(6): 
	    sprintf(s, "%s", "Length count load register value");
		break;
	  case(7):
	    sprintf(s, "%s", "MSB frequency");
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

/*

0-3	volume / envelope decay rate
4	envelope decay disable

5	length counter clock disable / envelope decay looping enable

6-7	duty cycle type (unused on noise channel)

-------

0-2	right shift amount

3	decrease / increase (1/0) wavelength
4-6	sweep update rate
7	sweep enable

0-7	8 LSB of wavelength

0-2	3 MS bits of wavelength 

3-7	length counter load register

*/


void main(void) {    

  setup((t_messlist **)&nes_square_class, (method)nes_square_new, (method)dsp_free, (short)sizeof(t_nes_square), 0L, 0);
  addint((method)nes_square_int);    
  addinx((method)nes_square_in1, 1);
  addinx((method)nes_square_in2, 2);
  addinx((method)nes_square_in3, 3);
  addinx((method)nes_square_in4, 4);
  addinx((method)nes_square_in5, 5);
  addinx((method)nes_square_in6, 6);
  addinx((method)nes_square_in7, 7);
  addmess((method)nes_square_dsp, "dsp", A_CANT, 0);
  addmess((method)nes_square_assist, "assist", A_CANT, 0);    
  dsp_initclass();
}  

void *nes_square_new(void)
{  
  t_nes_square *x = (t_nes_square *)newobject(nes_square_class);  
  dsp_setup((t_pxobject *)x, 1);    // left inlet  
  outlet_new((t_pxobject *)x, "signal");  // left outlet 
  intin(x, 1);
  intin(x, 2);
  intin(x, 3);
  intin(x, 4);
  intin(x, 5);
  intin(x, 6);
  intin(x, 7);
  apu_create(x, 1, APU_BASEFREQ, 44100, 60, 16);
  apu_write(x, APU_SMASK, (0x2));
  return (x); 
}   

void nes_square_dsp(t_nes_square *x, t_signal **sp, short *count) 
{   
  dsp_add(nes_square_perform, 3, sp[1]->s_vec, sp[0]->s_n, x);  
}  

void nes_square_int(t_nes_square *x, long value) 
{
  int8 v = value & 0x0F;
 
  if(value < 0) {
    x->env_decay_disab = 0;
  } else {
    x->env_decay_disab = 1;
  }
  x->vol = v;
   //post("B2 :: vol %d decay_enab %d", v, x->env_decay_disab);
  apu_write(x, APU_WRB0, x->vol | (x->env_decay_disab << 4) | (x->len_cnt << 5) | (x->dut_cycle_type << 6));
}

void nes_square_in1(t_nes_square *x, long value) 
{
  int8 v = value & 0x01;
  //post("range : 0-1");
  //post("B0 :: lsb cnt %d", v);
  x->len_cnt = v;
  apu_write(x, APU_WRB0, x->vol | (x->env_decay_disab << 4) | (x->len_cnt << 5) | (x->dut_cycle_type << 6));
}

void nes_square_in2(t_nes_square *x, long value) 
{
  int8 v = value & 0x03;
  x->dut_cycle_type = v;
  //post("B0 :: dut cycle type %d", v);
  apu_write(x, APU_WRB0, x->vol | (x->env_decay_disab << 4) | (x->len_cnt << 5) | (x->dut_cycle_type << 6));
}

void nes_square_in3(t_nes_square *x, long value) 
{
  int8 v = value & 0x07;
  //post("B1 :: rshift amt %d", v);
  x->rshift_amt = v;
  apu_write(x, APU_WRB1, x->rshift_amt | (x->wavelen_incr << 3) | (x->sweep_rate << 4) | (x->sweep_enab << 7));
}

void nes_square_in4(t_nes_square *x, long value) 
{
  int8 v = value & 0x07;
  x->sweep_rate = v;
  if(value < 0) {
    //decrease sweep rate -->bit 3 set to 0
	x->sweep_enab = 1;
	x->wavelen_incr = 0;
  } else if(value > 0) {
    x->sweep_enab = 1;
    x->wavelen_incr = 1;
  } else {
    x->sweep_enab = 0;
  }
  //post("B1 :: sweep rate %d sweep enab %d wavelen incr %d", x->sweep_rate, x->sweep_enab, x->wavelen_incr);
  apu_write(x, APU_WRB1, x->rshift_amt | (x->wavelen_incr << 3) | (x->sweep_rate << 4) | (x->sweep_enab << 7)); 
}

void nes_square_in5(t_nes_square *x, long value) 
{ 
 // post("B2 :: lsb wavelen %d", value);
  apu_write(x, APU_WRB2, value);  //lsb wavelength
}

void nes_square_in6(t_nes_square *x, long value) 
{
  int8 v = value & 0x1F;
  //post("range : 0-31");
  //post("B3 :: len_cnt_load %d", v);
  x->len_cnt_ld = v;
  apu_write(x, APU_WRB3, x->msb_wavelen | (x->len_cnt_ld << 3));
}

void nes_square_in7(t_nes_square *x, long value) 
{
  int8 v = value & 0x07;
  //post("B3 :: msb wavelen %d", v);
  x->msb_wavelen = v;
  apu_write(x, APU_WRB3, x->msb_wavelen | (x->len_cnt_ld << 3));
}

t_int *nes_square_perform(t_int *w) 
{      
  t_float *outL = (t_float *)(w[1]);
  t_nes_square *x = (t_nes_square *)(w[3]);
  apu_process(x, outL,  (int)(w[2]));  
  return (w + 4);
}  