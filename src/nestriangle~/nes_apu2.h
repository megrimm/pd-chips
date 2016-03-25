/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** Permission granted for distribution under the MAME license by the
** author, Matthew Conte.  If you use these routines in your own
** projects, feel free to choose between the MAME license or the GNU
** LGPL, as outlined below.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes_apu.h
**
** NES APU emulation header file
** $Id: nes_apu2.h,v 1.3 2004/06/13 22:01:27 npwoods Exp $
*/

#include "ext.h"
#include "z_dsp.h"

#ifndef _NES_APU_H_
#define _NES_APU_H_

#define int8 char
#define int16 short
#define int32 int
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define boolean uint8
#ifndef TRUE
#define	TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef INLINE
#ifdef __GNUC__
#define  INLINE      static inline
#elif defined(WIN32)
#define  INLINE      static __inline
#else
#define  INLINE      static
#endif
#endif

/* detect if this is included from MAME/MESS */
#ifndef HOLD_LINE
#define HOLE_LINE 1
#define cpunum_readmem(cpu,address) program_read_byte(address)
#define cpunum_set_input_line(cpu,line,state) n2a03_irq()
#endif

/* define this for realtime generated noise */
#define  REALTIME_NOISE

#define  APU_WRA0       0x4000
#define  APU_WRA1       0x4001
#define  APU_WRA2       0x4002
#define  APU_WRA3       0x4003
#define  APU_WRB0       0x4004
#define  APU_WRB1       0x4005
#define  APU_WRB2       0x4006
#define  APU_WRB3       0x4007
#define  APU_WRC0       0x4008
#define  APU_WRC2       0x400A
#define  APU_WRC3       0x400B
#define  APU_WRD0       0x400C
#define  APU_WRD2       0x400E
#define  APU_WRD3       0x400F
#define  APU_WRE0       0x4010
#define  APU_WRE1       0x4011
#define  APU_WRE2       0x4012
#define  APU_WRE3       0x4013

#define  APU_SMASK      0x4015

/* length of generated noise */
#define  APU_NOISE_32K  0x7FFF
#define  APU_NOISE_93   93

#define  APU_BASEFREQ   1789772.7272727272727272

/* to/from 16.16 fixed point */
#define  APU_FIX        16

/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/
 
typedef struct rectangle_s
{
   uint8 regs[4];

   boolean enabled;
   
   int32 phaseacc;
   int32 freq;
   int32 output_vol;
   boolean fixed_envelope;
   boolean holdnote;
   uint8 volume;

   int32 sweep_phase;
   int32 sweep_delay;
   boolean sweep_on;
   uint8 sweep_shifts;
   uint8 sweep_length;
   boolean sweep_inc;

   /* this may not be necessary in the future */
   int32 freq_limit;

   /* rectangle 0 uses a complement addition for sweep
   ** increases, while rectangle 1 uses subtraction
   */
   boolean sweep_complement;

   int32 env_phase;
   int32 env_delay;
   uint8 env_vol;

   int vbl_length;
   uint8 adder;
   int duty_flip;
} rectangle_t;

typedef struct triangle_s
{
   uint8 regs[3];

   boolean enabled;

   int32 freq;
   int32 phaseacc;
   int32 output_vol;

   uint8 adder;

   boolean holdnote;
   boolean counter_started;
   /* quasi-hack */
   int write_latency;

   int vbl_length;
   int linear_length;
} triangle_t;


typedef struct noise_s
{
   uint8 regs[3];

   boolean enabled;

   int32 freq;
   int32 phaseacc;
   int32 output_vol;

   int32 env_phase;
   int32 env_delay;
   uint8 env_vol;
   boolean fixed_envelope;
   boolean holdnote;

   uint8 volume;

   int vbl_length;

#ifdef REALTIME_NOISE
   uint8 xor_tap;
#else
   boolean short_sample;
   int cur_pos;
#endif /* REALTIME_NOISE */
} noise_t;

typedef struct dmc_s
{
   uint8 regs[4];

   /* bodge for timestamp queue */
   boolean enabled;
   
   int32 freq;
   int32 phaseacc;
   int32 output_vol;

   uint32 address;
   uint32 cached_addr;
   int dma_length;
   int cached_dmalength;
   uint8 cur_byte;

   boolean looping;
   boolean irq_gen;
   boolean irq_occurred;

} dmc_t;

enum
{
   APU_FILTER_NONE,
   APU_FILTER_LOWPASS,
   APU_FILTER_WEIGHTED
};

typedef struct apu_s
{
   rectangle_t rectangle[2];
   triangle_t triangle;
   noise_t noise;
   dmc_t dmc;
   uint8 enable_reg;

   int num_samples;

   boolean mix_enable[6];
   int filter_type;

   int32 cycle_rate;

   int sample_rate;
   int sample_bits;
   int refresh_rate;
} apu_t;

typedef struct _nes_noise  // Data structure for this object 
{     
  t_pxobject x_obj; 
  long lin_cnt_load;
  long cnt_start;
  long msb_wavelength;
  long cnt_load;
  long prev_sample;
  long vbl_lut[32];
  long trilength_lut[128];
  long decay_lut[16];
  double apu_basefreq;
  apu_t apu;
} t_nes_tri;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void apu_create(t_nes_tri *x, int cpunum, double basefreq, int sample_rate, int refresh_rate, int sample_bits);
extern void apu_setparams(t_nes_tri *x, int sample_rate, int refresh_rate, int sample_bits);

extern void apu_process(t_nes_tri *x, t_float *buffer, int num_samples);
extern void apu_reset(t_nes_tri *x);

extern void apu_setfilter(t_nes_tri *x, int filter_type);
extern void apu_setchan(int chan, boolean enabled);

extern uint8 apu_read(t_nes_tri *x, uint32 address);
extern void apu_write(t_nes_tri *x, uint32 address, uint8 value);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NES_APU_H_ */
