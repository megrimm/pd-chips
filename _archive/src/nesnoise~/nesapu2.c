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
** nes_apu.c
**
** NES APU emulation
** $Id: nes_apu2.c,v 1.2 2004/06/13 22:01:26 npwoods Exp $
*/

#include <string.h>
#include <assert.h>
#include "nesapu2.h"
#include <stdlib.h>
#include "m_pd.h"

#define  log_printf logerror
#define  ASSERT assert

#define  APU_OVERSAMPLE
#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))

/* noise lookups for both modes */
#ifndef REALTIME_NOISE
static int8 noise_long_lut[APU_NOISE_32K];
static int8 noise_short_lut[APU_NOISE_93];
#endif /* !REALTIME_NOISE */


/* vblank length table used for rectangles, triangle, noise */
static const uint8 vbl_length[32] =
{
    5, 127,
   10,   1,
   19,   2,
   40,   3,
   80,   4,
   30,   5,
    7,   6,
   13,   7,
    6,   8,
   12,   9,
   24,  10,
   48,  11,
   96,  12,
   36,  13,
    8,  14,
   16,  15
};

/* frequency limit of rectangle channels */
static const int freq_limit[8] =
{
   0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */
static const int noise_freq[16] =
{
     4,    8,   16,   32,   64,   96,  128,  160,
   202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */
const int dmc_clocks[16] =
{
   428, 380, 340, 320, 286, 254, 226, 214,
   190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */
static const int duty_lut[4] = { 2, 4, 8, 12 };

/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
#ifdef REALTIME_NOISE
INLINE int8 shift_register15(uint8 xor_tap)
{
   static int sreg = 0x4000;
   int bit0, tap, bit14;

   bit0 = sreg & 1;
   tap = (sreg & xor_tap) ? 1 : 0;
   bit14 = (bit0 ^ tap);
   sreg >>= 1;
   sreg |= (bit14 << 14);
   return (bit0 ^ 1);
}
#else /* !REALTIME_NOISE */
static void shift_register15(int8 *buf, int count)
{
   static int sreg = 0x4000;
   int bit0, bit1, bit6, bit14;

   if (count == APU_NOISE_93)
   {
      while (count--)
      {
         bit0 = sreg & 1;
         bit6 = (sreg & 0x40) >> 6;
         bit14 = (bit0 ^ bit6);
         sreg >>= 1;
         sreg |= (bit14 << 14);
         *buf++ = bit0 ^ 1;
      }
   }
   else /* 32K noise */
   {
      while (count--)
      {
         bit0 = sreg & 1;
         bit1 = (sreg & 2) >> 1;
         bit14 = (bit0 ^ bit1);
         sreg >>= 1;
         sreg |= (bit14 << 14);
         *buf++ = bit0 ^ 1;
      }
   }
}
#endif /* !REALTIME_NOISE */

/* RECTANGLE WAVE
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_RECTANGLE_OUTPUT chan->output_vol
static int32 apu_rectangle(t_nesnoise *x, rectangle_t *chan)
{
   int32 output;

#ifdef APU_OVERSAMPLE
   int num_times;
   int32 total;
#endif /* APU_OVERSAMPLE */

   APU_VOLUME_DECAY(chan->output_vol);

   if (FALSE == chan->enabled || 0 == chan->vbl_length)
      return APU_RECTANGLE_OUTPUT;

   /* vbl length counter */
   if (FALSE == chan->holdnote)
      chan->vbl_length--;

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= 4; /* 240/60 */
   while (chan->env_phase < 0)
   {
      chan->env_phase += chan->env_delay;

      if (chan->holdnote)
         chan->env_vol = (chan->env_vol + 1) & 0x0F;
      else if (chan->env_vol < 0x0F)
         chan->env_vol++;
   }

   /* TODO: using a table of max frequencies is not technically
   ** clean, but it is fast and (or should be) accurate
   */
   if (chan->freq < 8 || (FALSE == chan->sweep_inc && chan->freq > chan->freq_limit))
      return APU_RECTANGLE_OUTPUT;

   /* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
   if (chan->sweep_on && chan->sweep_shifts)
   {
      chan->sweep_phase -= 2; /* 120/60 */
      while (chan->sweep_phase < 0)
      {
         chan->sweep_phase += chan->sweep_delay;

         if (chan->sweep_inc) /* ramp up */
         {
            if (TRUE == chan->sweep_complement)
               chan->freq += ~(chan->freq >> chan->sweep_shifts);
            else
               chan->freq -= (chan->freq >> chan->sweep_shifts);
         }
         else /* ramp down */
         {
            chan->freq += (chan->freq >> chan->sweep_shifts);
         }
      }
   }

   chan->phaseacc -= (x->apu).cycle_rate; /* # of cycles per sample */
   if (chan->phaseacc >= 0)
      return APU_RECTANGLE_OUTPUT;

#ifdef APU_OVERSAMPLE
   num_times = total = 0;

   if (chan->fixed_envelope)
      output = chan->volume << 8; /* fixed volume */
   else
      output = (chan->env_vol ^ 0x0F) << 8;
#endif /* APU_OVERSAMPLE */

   while (chan->phaseacc < 0)
   {
      chan->phaseacc += (chan->freq + 1) << APU_FIX;
      chan->adder = (chan->adder + 1) & 0x0F;

#ifdef APU_OVERSAMPLE
      if (chan->adder < chan->duty_flip)
         total += output;
      else
         total -= output;

      num_times++;
#endif /* APU_OVERSAMPLE */
   }

#ifdef APU_OVERSAMPLE
   chan->output_vol = total / num_times;
#else /* !APU_OVERSAMPLE */
   if (chan->fixed_envelope)
      output = chan->volume << 8; /* fixed volume */
   else
      output = (chan->env_vol ^ 0x0F) << 8;

   if (0 == chan->adder)
      chan->output_vol = output;
   else if (chan->adder == chan->duty_flip)
      chan->output_vol = -output;
#endif /* !APU_OVERSAMPLE */

   return APU_RECTANGLE_OUTPUT;
}

/* TRIANGLE WAVE
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
#define  APU_TRIANGLE_OUTPUT  (chan->output_vol + (chan->output_vol >> 2))
static int32 apu_triangle(t_nesnoise *x, triangle_t *chan)
{
   APU_VOLUME_DECAY(chan->output_vol);

   if (FALSE == chan->enabled || 0 == chan->vbl_length) {
      return APU_TRIANGLE_OUTPUT;	  
   }

   if (chan->counter_started)
   {
      if (chan->linear_length > 0)
         chan->linear_length--;
      if (chan->vbl_length && FALSE == chan->holdnote)
         chan->vbl_length--;
   }
   else if (FALSE == chan->holdnote && chan->write_latency)
   {
      if (--chan->write_latency == 0)
         chan->counter_started = TRUE;
   }

   if (0 == chan->linear_length || chan->freq < (4 << APU_FIX)) /* inaudible */
      return APU_TRIANGLE_OUTPUT;

   chan->phaseacc -= (x->apu).cycle_rate; /* # of cycles per sample */
   while (chan->phaseacc < 0)
   {
   //post("chan phaseacc");
      chan->phaseacc += chan->freq;
      chan->adder = (chan->adder + 1) & 0x1F;

      if (chan->adder & 0x10)
         chan->output_vol -= (2 << 8);
      else
         chan->output_vol += (2 << 8);
   }

   return APU_TRIANGLE_OUTPUT;
}


/* WHITE NOISE CHANNEL
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
#define  APU_NOISE_OUTPUT  ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)

static int32 apu_noise(t_nesnoise *x, noise_t *chan)
{
   
   int32 outvol;

#if defined(APU_OVERSAMPLE) && defined(REALTIME_NOISE)
#else /* !(APU_OVERSAMPLE && REALTIME_NOISE) */
   int32 noise_bit;
#endif /* !(APU_OVERSAMPLE && REALTIME_NOISE) */
#ifdef APU_OVERSAMPLE
   int num_times;
   int32 total;
#endif /* APU_OVERSAMPLE */

   APU_VOLUME_DECAY(chan->output_vol);

   if (FALSE == chan->enabled || 0 == chan->vbl_length) return APU_NOISE_OUTPUT;

   /* vbl length counter */
   if (FALSE == chan->holdnote)
      chan->vbl_length--;

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   chan->env_phase -= 4; /* 240/60 */
   while (chan->env_phase < 0)
   {
      chan->env_phase += chan->env_delay;

      if (chan->holdnote)
         chan->env_vol = (chan->env_vol + 1) & 0x0F;
      else if (chan->env_vol < 0x0F)
         chan->env_vol++;
   }

   chan->phaseacc -= (x->apu).cycle_rate; /* # of cycles per sample */
   if (chan->phaseacc >= 0)
      return APU_NOISE_OUTPUT;

#ifdef APU_OVERSAMPLE
   num_times = total = 0;

   if (chan->fixed_envelope)
      outvol = chan->volume << 8; /* fixed volume */
   else
      outvol = (chan->env_vol ^ 0x0F) << 8;
#endif /* APU_OVERSAMPLE */

   while (chan->phaseacc < 0)
   {
      chan->phaseacc += chan->freq;

#ifdef REALTIME_NOISE

#ifdef APU_OVERSAMPLE
      if (shift_register15(chan->xor_tap))
         total += outvol;
      else
         total -= outvol;

      num_times++;
#else /* !APU_OVERSAMPLE */
      noise_bit = shift_register15(chan->xor_tap);
#endif /* !APU_OVERSAMPLE */

#else /* !REALTIME_NOISE */
      chan->cur_pos++;

      if (chan->short_sample)
      {
         if (APU_NOISE_93 == chan->cur_pos)
            chan->cur_pos = 0;
      }
      else
      {
         if (APU_NOISE_32K == chan->cur_pos)
            chan->cur_pos = 0;
      }

#ifdef APU_OVERSAMPLE
      if (chan->short_sample)
         noise_bit = noise_short_lut[chan->cur_pos];
      else
         noise_bit = noise_long_lut[chan->cur_pos];

      if (noise_bit)
         total += outvol;
      else
         total -= outvol;

      num_times++;
#endif /* APU_OVERSAMPLE */
#endif /* !REALTIME_NOISE */
   }

#ifdef APU_OVERSAMPLE
   chan->output_vol = total / num_times;
#else /* !APU_OVERSAMPLE */
   if (chan->fixed_envelope)
      outvol = chan->volume << 8; /* fixed volume */
   else
      outvol = (chan->env_vol ^ 0x0F) << 8;

#ifndef REALTIME_NOISE
   if (chan->short_sample)
      noise_bit = noise_short_lut[chan->cur_pos];
   else
      noise_bit = noise_long_lut[chan->cur_pos];
#endif /* !REALTIME_NOISE */

   if (noise_bit)
      chan->output_vol = outvol;
   else
      chan->output_vol = -outvol;
#endif /* !APU_OVERSAMPLE */

   return APU_NOISE_OUTPUT;
}


INLINE void apu_dmcreload(dmc_t *chan)
{
   chan->address = chan->cached_addr;
   chan->dma_length = chan->cached_dmalength;
   chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
#define  APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)
static int32 apu_dmc(t_nesnoise *x, dmc_t *chan)
{
   int delta_bit;

   APU_VOLUME_DECAY(chan->output_vol);

   /* only process when channel is alive */
   if (chan->dma_length)
   {
      chan->phaseacc -= (x->apu).cycle_rate; /* # of cycles per sample */

      while (chan->phaseacc < 0)
      {
         chan->phaseacc += chan->freq;

         delta_bit = (chan->dma_length & 7) ^ 7;

         if (7 == delta_bit)
         {
            /* FIX!!! chan->cur_byte = cpunum_readmem(apu->cpunum, chan->address); */

            /* steal a cycle from CPU */
            //nes6502_burn(1);

            if (0xFFFF == chan->address)
               chan->address = 0x8000;
            else
               chan->address++;
         }

         if (--chan->dma_length == 0)
         {
            /* if loop bit set, we're cool to retrigger sample */
            if (chan->looping)
               apu_dmcreload(chan);
            else
            {
               /* check to see if we should generate an irq */
               if (chan->irq_gen)
               {
                  chan->irq_occurred = TRUE;
                 // cpunum_set_input_line(apu->cpunum, 0, HOLD_LINE);
               }

               /* bodge for timestamp queue */
               chan->enabled = FALSE;
               break;
            }
         }

         /* positive delta */
         if (chan->cur_byte & (1 << delta_bit))
         {
            if (chan->regs[1] < 0x7D)
            {
               chan->regs[1] += 2;
               chan->output_vol += (2 << 8);
            }
         }
         /* negative delta */
         else
         {
            if (chan->regs[1] > 1)
            {
               chan->regs[1] -= 2;
               chan->output_vol -= (2 << 8);
            }
         }
      }
   }

   return APU_DMC_OUTPUT;
}


void apu_write(t_nesnoise *x, uint32 address, uint8 value)
{
   int chan;
   apu_t *apu = &(x->apu);

   //ASSERT(apu);
   switch (address)
   {
   /* rectangles */
   case APU_WRA0:
   case APU_WRB0:
   //post("writing apu_wrab0 %d", value);
      chan = (address & 4) ? 1 : 0;
      apu->rectangle[chan].regs[0] = value;

      //apu->rectangle[chan].volume = value & 0x0F;
	  apu->rectangle[chan].volume = 0x0F;
      apu->rectangle[chan].env_delay = x->decay_lut[value & 0x0F];
      apu->rectangle[chan].holdnote = (value & 0x20) ? TRUE : FALSE;
      apu->rectangle[chan].fixed_envelope = (value & 0x10) ? TRUE : FALSE;
      apu->rectangle[chan].duty_flip = duty_lut[value >> 6];
      break;

   case APU_WRA1:
   case APU_WRB1:
      chan = (address & 4) ? 1 : 0;
      apu->rectangle[chan].regs[1] = value;
      apu->rectangle[chan].sweep_on = (value & 0x80) ? TRUE : FALSE;
      apu->rectangle[chan].sweep_shifts = value & 7;
      apu->rectangle[chan].sweep_delay = x->decay_lut[(value >> 4) & 7];

      apu->rectangle[chan].sweep_inc = (value & 0x08) ? TRUE : FALSE;
      apu->rectangle[chan].freq_limit = freq_limit[value & 7];
      break;

   case APU_WRA2:
   case APU_WRB2:
      chan = (address & 4) ? 1 : 0;
      apu->rectangle[chan].regs[2] = value;
//      if (apu->rectangle[chan].enabled)
         apu->rectangle[chan].freq = (apu->rectangle[chan].freq & ~0xFF) | value;
      break;

   case APU_WRA3:
   case APU_WRB3:
      chan = (address & 4) ? 1 : 0;
      apu->rectangle[chan].regs[3] = value;

      apu->rectangle[chan].vbl_length = x->vbl_lut[value >> 3];
      apu->rectangle[chan].env_vol = 0;
      apu->rectangle[chan].freq = ((value & 7) << 8) | (apu->rectangle[chan].freq & 0xFF);
      apu->rectangle[chan].adder = 0;
      break;

   /* triangle */
   case APU_WRC0:
      apu->triangle.regs[0] = value;
      apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;

      if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)
         apu->triangle.linear_length = x->trilength_lut[value & 0x7F];

      break;

   case APU_WRC2:

      apu->triangle.regs[1] = value;
      apu->triangle.freq = ((((apu->triangle.regs[2] & 7) << 8) + value) + 1) << APU_FIX;
      break;

   case APU_WRC3:

      apu->triangle.regs[2] = value;

      /* this is somewhat of a hack.  there appears to be some latency on
      ** the Real Thing between when trireg0 is written to and when the
      ** linear length counter actually begins its countdown.  we want to
      ** prevent the case where the program writes to the freq regs first,
      ** then to reg 0, and the counter accidentally starts running because
      ** of the sound queue's timestamp processing.
      **
      ** set latency to a couple hundred cycles -- should be plenty of time
      ** for the 6502 code to do a couple of table dereferences and load up
      ** the other triregs
      */

      /* 06/13/00 MPC -- seems to work OK */
      apu->triangle.write_latency = (int) (228 / (apu->cycle_rate >> APU_FIX));

      apu->triangle.freq = ((((value & 7) << 8) + apu->triangle.regs[1]) + 1) << APU_FIX;
      apu->triangle.vbl_length = x->vbl_lut[value >> 3];
      apu->triangle.counter_started = FALSE;
      apu->triangle.linear_length = x->trilength_lut[apu->triangle.regs[0] & 0x7F];

      break;

   /* noise */
   case APU_WRD0:
      apu->noise.regs[0] = value;
      apu->noise.env_delay = x->decay_lut[value & 0x0F];
      apu->noise.holdnote = (value & 0x20) ? TRUE : FALSE;
      apu->noise.fixed_envelope = (value & 0x10) ? TRUE : FALSE;
      apu->noise.volume = value & 0x0F;
      break;

   case APU_WRD2:
      apu->noise.regs[1] = value;
      apu->noise.freq = noise_freq[value & 0x0F] << APU_FIX;

#ifdef REALTIME_NOISE
      apu->noise.xor_tap = (value & 0x80) ? 0x40: 0x02;
#else /* !REALTIME_NOISE */
      /* detect transition from long->short sample */
      if ((value & 0x80) && FALSE == apu->noise.short_sample)
      {
         /* recalculate short noise buffer */
         shift_register15(noise_short_lut, APU_NOISE_93);
         apu->noise.cur_pos = 0;
      }
      apu->noise.short_sample = (value & 0x80) ? TRUE : FALSE;
#endif /* !REALTIME_NOISE */
      break;

   case APU_WRD3:
      apu->noise.regs[2] = value;

      apu->noise.vbl_length = x->vbl_lut[value >> 3];
      apu->noise.env_vol = 0; /* reset envelope */
      break;

   /* DMC */
   case APU_WRE0:
      apu->dmc.regs[0] = value;

      apu->dmc.freq = dmc_clocks[value & 0x0F] << APU_FIX;
      apu->dmc.looping = (value & 0x40) ? TRUE : FALSE;

      if (value & 0x80)
         apu->dmc.irq_gen = TRUE;
      else
      {
         apu->dmc.irq_gen = FALSE;
         apu->dmc.irq_occurred = FALSE;
      }
      break;

   case APU_WRE1: /* 7-bit DAC */
      /* add the _delta_ between written value and
      ** current output level of the volume reg
      */
      value &= 0x7F; /* bit 7 ignored */
      apu->dmc.output_vol += ((value - apu->dmc.regs[1]) << 8);
      apu->dmc.regs[1] = value;
      break;

   case APU_WRE2:
      apu->dmc.regs[2] = value;
      apu->dmc.cached_addr = 0xC000 + (uint16) (value << 6);
      break;

   case APU_WRE3:
      apu->dmc.regs[3] = value;
      apu->dmc.cached_dmalength = ((value << 4) + 1) << 3;
      break;

   case APU_SMASK:
      /* bodge for timestamp queue */
      apu->dmc.enabled = (value & 0x10) ? TRUE : FALSE;

      apu->enable_reg = value;

      for (chan = 0; chan < 2; chan++)
      {
         if (value & (1 << chan))
            apu->rectangle[chan].enabled = TRUE;
         else
         {
            apu->rectangle[chan].enabled = FALSE;
            apu->rectangle[chan].vbl_length = 0;
         }
      }

      if (value & 0x04)
         apu->triangle.enabled = TRUE;
      else
      {
         apu->triangle.enabled = FALSE;
         apu->triangle.vbl_length = 0;
         apu->triangle.linear_length = 0;
         apu->triangle.counter_started = FALSE;
         apu->triangle.write_latency = 0;
      }

      if (value & 0x08)
         apu->noise.enabled = TRUE;
      else
      {
         apu->noise.enabled = FALSE;
         apu->noise.vbl_length = 0;
      }

      if (value & 0x10)
      {
         if (0 == apu->dmc.dma_length)
            apu_dmcreload(&apu->dmc);
      }
      else
         apu->dmc.dma_length = 0;

      apu->dmc.irq_occurred = FALSE;
      break;

      /* unused, but they get hit in some mem-clear loops */
   case 0x4009:
   case 0x400D:
      break;

   default:
      break;
   }
}

/* Read from $4000-$4017 */
uint8 apu_read(t_nesnoise *x, uint32 address)
{
   uint8 value;
   
   apu_t *apu = &(x->apu);

   //ASSERT(apu);

   switch (address)
   {
   case APU_SMASK:
      value = 0;
      /* Return 1 in 0-5 bit pos if a channel is playing */
      if (apu->rectangle[0].enabled && apu->rectangle[0].vbl_length)
         value |= 0x01;
      if (apu->rectangle[1].enabled && apu->rectangle[1].vbl_length)
         value |= 0x02;
      if (apu->triangle.enabled && apu->triangle.vbl_length)
         value |= 0x04;
      if (apu->noise.enabled && apu->noise.vbl_length)
         value |= 0x08;

      /* bodge for timestamp queue */
      if (apu->dmc.enabled)
         value |= 0x10;

      if (apu->dmc.irq_occurred)
         value |= 0x80;

      break;

   default:
      value = (address >> 8); /* heavy capacitance on data bus */
      break;
   }

   return value;
}


void apu_getpcmdata(t_nesnoise *x, int *num_samples, int *sample_bits)
{
   //ASSERT(apu);
   //*data = apu->buffer;
   //FIX FIX FIX FIX FIX FIX!!
   /*
   *num_samples = apu->num_samples;
   *sample_bits = apu->sample_bits;
   */
}


void apu_process(t_nesnoise *x, t_float *buffer, int num_samples)
{
   //FIX FIX FIX FIX!!
   //static int32 prev_sample = 0;
   
   
   int32 next_sample, accum;
   uint8 val = 0;
   float va = 0;
   apu_t *apu = &(x->apu);
   //int16 *buf16;
   //uint8 *buf8;

   //ASSERT(apu);

   if (NULL != buffer)
   {
      while (num_samples--)
      {
         accum = 0;
		 
         //if (apu->mix_enable[0]) accum += apu_rectangle(&apu->rectangle[0]);
         //if (apu->mix_enable[1]) accum += apu_rectangle(&apu->rectangle[1]);
         //if (apu->mix_enable[2]) accum += apu_triangle(&apu->triangle);
         if (apu->mix_enable[3]) accum += apu_noise(x, &apu->noise);
         //if (apu->mix_enable[4]) accum += apu_dmc(&apu->dmc);

         /* do any filtering */
         if (APU_FILTER_NONE != apu->filter_type)
         {
            next_sample = accum;

            if (APU_FILTER_LOWPASS == apu->filter_type)
            {
               accum += x->prev_sample;
               accum >>= 1;
            }
            else
               accum = (accum + accum + accum + x->prev_sample) >> 2;

            x->prev_sample = next_sample;
         }

         /* little extra kick for the kids */
         accum <<= 1;

         /* prevent clipping */
         if (accum > 0x7FFF) accum = 0x7FFF;
         else if (accum < -0x8000) accum = -0x8000;

         /* signed 16-bit output, unsigned 8-bit */
          val = (accum >> 8) ^ 0x80;
		  va = (((float)val))/2000.0;
		  *(buffer++) = va;
	  }
   }
}

/* set the filter type */
void apu_setfilter(t_nesnoise *x, int filter_type)
{
   (x->apu).filter_type = filter_type;
}

void apu_reset(t_nesnoise *x)
{
   uint32 address;

   /* use to avoid bugs =) */
   for (address = 0x4000; address <= 0x4013; address++)
      apu_write(x, address, 0);

#ifdef NSF_PLAYER
   apu_write(x, 0x400C, 0x10); /* silence noise channel on NSF start */
   apu_write(x, 0x4015, 0x0F);
#else /* !NSF_PLAYER */
   apu_write(x, 0x4015, 0x00);
#endif /* !NSF_PLAYER */

}

void apu_build_luts(t_nesnoise *x, int num_samples)
{
   int i;

   /* lut used for enveloping and frequency sweeps */
   for (i = 0; i < 16; i++)
      x->decay_lut[i] = num_samples * (i + 1);

   /* used for note length, based on vblanks and size of audio buffer */
   for (i = 0; i < 32; i++) {
      x->vbl_lut[i] = vbl_length[i] * num_samples;
   }

   /* triangle wave channel's linear length table */
   for (i = 0; i < 128; i++)
      x->trilength_lut[i] = (int) (0.25 * (i * num_samples));

#ifndef REALTIME_NOISE
   /* generate noise samples */
   shift_register15(noise_long_lut, APU_NOISE_32K);
   shift_register15(noise_short_lut, APU_NOISE_93);
#endif /* !REALTIME_NOISE */
}

void apu_setparams(t_nesnoise *x, int sample_rate, int refresh_rate, int sample_bits)
{
   apu_t *apu = &(x->apu);
   apu->sample_rate = sample_rate;
   apu->refresh_rate = refresh_rate;
   apu->sample_bits = sample_bits;

   apu->num_samples = sample_rate / refresh_rate;

   /* turn into fixed point! */
   apu->cycle_rate = (int32) (x->apu_basefreq * 65536.0 / (float) sample_rate);

   /* build various lookup tables for apu */
   apu_build_luts(x, apu->num_samples);

   apu_reset(x);
}

/* Initializes emulated sound hardware, creates waveforms/voices */
/* (1, 60, 44100, 60, 16)*/
void apu_create(t_nesnoise *x, int cpunum, double basefreq, int sample_rate, int refresh_rate, int sample_bits)
{
   int channel;
   /* set non default base frequency? */
   x->apu_basefreq = (basefreq) ? basefreq : APU_BASEFREQ;

   /* set the stupid flag to tell difference between two rectangles */
   (x->apu).rectangle[0].sweep_complement = TRUE;
   (x->apu).rectangle[1].sweep_complement = FALSE;
   

   apu_setparams(x, sample_rate, refresh_rate, sample_bits);

   for (channel = 0; channel < 6; channel++) {
	  (x->apu).mix_enable[channel] = TRUE;
   }

   apu_setfilter(x, APU_FILTER_LOWPASS);
   
	(x->apu).rectangle[0].enabled = TRUE;
	(x->apu).rectangle[1].enabled = TRUE;
	(x->apu).triangle.enabled = TRUE;
	(x->apu).noise.enabled = TRUE;
}
