#include <stdlib.h>
#include "m_pd.h"

// #include "ext.h"
// #include "z_dsp.h"
#include "nes_apu2.h"

void *nes_tri_class;  

typedef signed char Int8;

void *nes_tri_new(void);  
t_int *nes_tri_perform(t_int *w); 
void nes_tri_dsp(t_nes_tri *x, t_signal **sp, short *count);  
void nes_tri_bang(t_nes_tri *x);
void nes_tri_int(t_nes_tri *x, long value);
void nes_tri_in1(t_nes_tri *x, long value);
void nes_tri_in2(t_nes_tri *x, long value);
void nes_tri_in3(t_nes_tri *x, long value);
void nes_tri_in4(t_nes_tri *x, long value);

void main(void) {    

  setup((t_messlist **)&nes_tri_class, (method)nes_tri_new, (method)dsp_free, (short)sizeof(t_nes_tri), 0L, 0);
  addbang((method)nes_tri_bang);
  addint((method)nes_tri_int);    
  addinx((method)nes_tri_in1, 1);
  addinx((method)nes_tri_in2, 2);
  addinx((method)nes_tri_in3, 3);
  addinx((method)nes_tri_in4, 4);
  addmess((method)nes_tri_dsp, "dsp", A_CANT, 0);    
  dsp_initclass(); 
}  

void *nes_tri_new(void)
{  
  t_nes_tri *x = (t_nes_tri *)newobject(nes_tri_class);  
  dsp_setup((t_pxobject *)x, 1);          // left inlet  
  outlet_new((t_pxobject *)x, "signal");  // left outlet 
  intin(x, 1);
  intin(x, 2);
  intin(x, 3);
  intin(x, 4);
  x->msb_wavelength = 0;
  x->cnt_load = 0;
  x->lin_cnt_load = 0;
  x->cnt_start = 0;
  apu_create(x, 1, APU_BASEFREQ, 44100, 60, 16);
  apu_write(x, APU_SMASK, (0x1 << 2));  //enable triangle channel
  return (x); 
}   

void nes_tri_dsp(t_nes_tri *x, t_signal **sp, short *count) 
{   
  dsp_add(nes_tri_perform, 3, sp[1]->s_vec, sp[0]->s_n, x);  
}  

void nes_tri_int(t_nes_tri *x, long value) 
{
  //msb wavelength
  int8 v = (int8)value;
  v &= 0x07;
  x->msb_wavelength = v;
  apu_write(x, APU_WRC3, x->msb_wavelength | (x->cnt_load << 3));

}

void nes_tri_in1(t_nes_tri *x, long value) 
{
  int8 v = (int8)value;
  v &= 0x3F;
  x->cnt_load = v;
  apu_write(x, APU_WRC3, x->msb_wavelength | (x->cnt_load << 3));
}

void nes_tri_in2(t_nes_tri *x, long value) 
{
  int8 v = (int8)value;
  v &= 0x7F;
  x->lin_cnt_load = v;
  apu_write(x, APU_WRC0, x->lin_cnt_load | (x->cnt_start << 7));
}

void nes_tri_in3(t_nes_tri *x, long value) 
{
  int8 v = (int8)value;
  v &= 0x01;
  x->cnt_start = v;
  apu_write(x, APU_WRC0, x->lin_cnt_load | (x->cnt_start << 7));
}

void nes_tri_in4(t_nes_tri *x, long value) 
{
  apu_write(x, APU_WRC2, value);
}

void nes_tri_bang(t_nes_tri *x) 
{   
}  

t_int *nes_tri_perform(t_int *w) 
{      
  t_float *outL = (t_float *)(w[1]);
  t_nes_tri *x = (t_nes_tri *)(w[3]);
   
  apu_process(x, outL,  (int)(w[2]));  
  
  return (w + 4);
}  