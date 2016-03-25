
#ifndef __SOUND_H__
#define __SOUND_H__

#include <stdlib.h>
#include "m_pd.h"

#ifdef IS_LITTLE_ENDIAN
#define LO 0
#define HI 1
#else
#define LO 1
#define HI 0
#endif

#define RCV_END { 0, rcv_end, 0, 0 }
#define RCV_INT(n,v) { (n), rcv_int, 1, (v) }
#define RCV_BOOL(n,v) { (n), rcv_bool, 1, (v) }

#define RI_P1    0x00
#define RI_SB    0x01
#define RI_SC    0x02
#define RI_DIV   0x04
#define RI_TIMA  0x05
#define RI_TMA   0x06
#define RI_TAC   0x07

#define RI_KEY1  0x4D

#define RI_RP    0x56

#define RI_SVBK  0x70

/* Interrupts flags */

#define RI_IF    0x0F
#define RI_IE    0xFF

/* LCDC */

#define RI_LCDC  0x40
#define RI_STAT  0x41
#define RI_SCY   0x42
#define RI_SCX   0x43
#define RI_LY    0x44
#define RI_LYC   0x45
#define RI_DMA   0x46
#define RI_BGP   0x47
#define RI_OBP0  0x48
#define RI_OBP1  0x49
#define RI_WY    0x4A
#define RI_WX    0x4B

#define RI_VBK   0x4F

#define RI_HDMA1 0x51
#define RI_HDMA2 0x52
#define RI_HDMA3 0x53
#define RI_HDMA4 0x54
#define RI_HDMA5 0x55

#define RI_BCPS  0x68
#define RI_BCPD  0x69
#define RI_OCPS  0x6A
#define RI_OCPD  0x6B



/* Sound */

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

//#define REG(n) ram.hi[(n)]
#define REG(n) gb->hi[(n)]

/* Interrupts flags */

#define R_IF    REG(RI_IF)
#define R_IE    REG(RI_IE)

/* Sound */

#define R_NR10 REG(RI_NR10)
#define R_NR11 REG(RI_NR11)
#define R_NR12 REG(RI_NR12)
#define R_NR13 REG(RI_NR13)
#define R_NR14 REG(RI_NR14)
#define R_NR21 REG(RI_NR21)
#define R_NR22 REG(RI_NR22)
#define R_NR23 REG(RI_NR23)
#define R_NR24 REG(RI_NR24)
#define R_NR30 REG(RI_NR30)
#define R_NR31 REG(RI_NR31)
#define R_NR32 REG(RI_NR32)
#define R_NR33 REG(RI_NR33)
#define R_NR34 REG(RI_NR34)
#define R_NR41 REG(RI_NR41)
#define R_NR42 REG(RI_NR42)
#define R_NR43 REG(RI_NR43)
#define R_NR44 REG(RI_NR44)
#define R_NR50 REG(RI_NR50)
#define R_NR51 REG(RI_NR51)
#define R_NR52 REG(RI_NR52)

typedef unsigned char byte;

typedef unsigned char un8;
typedef unsigned short un16;
typedef unsigned int un32;

typedef signed char n8;
typedef signed short n16;
typedef signed int n32;

typedef un16 word;
typedef word addr;

typedef enum rctype
{
	rcv_end,
	rcv_int,
	rcv_string,
	rcv_vector,
	rcv_bool
} rcvtype_t;

typedef struct rcvar_s
{
	char *name;
	int type;
	int len;
	void *mem;
} rcvar_t;

union reg
{
	byte b[2][2];
	word w[2];
	un32 d; /* padding for alignment, carry */
};

struct cpu
{
	union reg pc, sp, bc, de, hl, af;
	int ime, ima;
	int speed;
	int halt;
	int div, tim;
	int lcdc;
	int snd;
};

struct hw
{
	byte ilines;
	byte pad;
	int cgb, gba;
	int hdma;
};

struct sndchan
{
	int on;
	unsigned pos;
	int cnt, encnt, swcnt;
	int len, enlen, swlen;
	int swfreq;
	int freq;
	int envol, endir;
};

typedef struct _gb
{ 
  t_object x_obj;
  int rate;
  long vol;
  long vol_sweep_dir;
  long sweep_len;
  long shift_clk;
  long shift_reg_width;
  long main_on;
  long cnt_enab;
  long div_freq;
  struct sndchan ch[4];
  int stereo;
  int hz;
  int len;
  byte hi[256];

/* #ifdef PD
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
*/
} t_gb; 

void sound_write(t_gb* gb, byte r, byte b);
void sound_reset(t_gb* gb);
void sound_mix(t_gb* gb, t_float* bufL, t_float* bufR, int size);

#endif