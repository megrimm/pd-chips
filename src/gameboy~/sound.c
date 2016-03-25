
//pcm.pos needs to be reset to zero after each copy to speaker 
// 1<<21 -> 2097152
// @ 44100 hz:  snd.rate = 47.5544
//cpu.snd = 32832 (average?) -> l/r channel buffer sizes of 349...
//mix before reg write
// can ignore audio_done.
//call sound_reset() at init to initialize snd.wave

#include "sound.h"
#include "noise.h"

#include <stdlib.h>
#include "m_pd.h"

#define S1 (((gb->ch)[0]))
#define S2 (((gb->ch)[1]))
#define S3 (((gb->ch)[2]))
#define S4 (((gb->ch)[3]))

static int sound = 1;
static int samplerate = 44100;
static int stereo = 1;
//static volatile int audio_done;

void pcm_init(t_gb* gb)
{
	//int i;
	//SDL_AudioSpec as;

	if (!sound) return;
	
	//SDL_InitSubSystem(SDL_INIT_AUDIO);
	//as.freq = samplerate;
	//as.format = AUDIO_U8;
	//as.channels = 1 + stereo;
	//as.samples = samplerate / 60;
	//for (i = 1; i < as.samples; i<<=1);
	//as.samples = i;
	//as.callback = audio_callback;
	//as.userdata = 0;
	//if (SDL_OpenAudio(&as, 0) == -1)
	//	return;
	
	gb->hz = samplerate;
	//post("pcm.hz %d", gb->hz);
	gb->stereo = (1+stereo) - 1;
	//pcm.len = as.size;  /* KBFIX guessing here... */
	gb->len = 256;  /* KBFIX 128 bytes per channel l & r */
	//pcm.buf = malloc(pcm.len);
	//gb->pos = 0;
	//memset(pcm.buf, 0, pcm.len);
	
	//SDL_PauseAudio(0);
}

/*

struct pcm pcm;


static int sound = 1;
static int samplerate = 44100;
static int stereo = 1;
static volatile int audio_done;

rcvar_t pcm_exports[] =
{
	RCV_BOOL("sound", &sound),
	RCV_INT("stereo", &stereo),
	RCV_INT("samplerate", &samplerate),
	RCV_END
};


static void audio_callback(void *blah, byte *stream, int len)
{
	memcpy(stream, pcm.buf, len);
	audio_done = 1;
}


void pcm_init()
{
	int i;
	SDL_AudioSpec as;

	if (!sound) return;
	
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	as.freq = samplerate;
	as.format = AUDIO_U8;
	as.channels = 1 + stereo;
	as.samples = samplerate / 60;
	for (i = 1; i < as.samples; i<<=1);
	as.samples = i;
	as.callback = audio_callback;
	as.userdata = 0;
	if (SDL_OpenAudio(&as, 0) == -1)
		return;
	
	pcm.hz = as.freq;
	pcm.stereo = as.channels - 1;
	pcm.len = as.size;
	pcm.buf = malloc(pcm.len);
	pcm.pos = 0;
	memset(pcm.buf, 0, pcm.len);
	
	SDL_PauseAudio(0);
}

int pcm_submit()
{
	if (!pcm.buf) return 0;
	if (pcm.pos < pcm.len) return 1;
	while (!audio_done)
		SDL_Delay(4);
	audio_done = 0;
	pcm.pos = 0;
	return 1;
}

void pcm_close()
{
	if (sound) SDL_CloseAudio();
}

*/
/*

const static byte dmgwave[16] =
{
	0xac, 0xdd, 0xda, 0x48,
	0x36, 0x02, 0xcf, 0x16,
	0x2c, 0x04, 0xe5, 0x2c,
	0xac, 0xdd, 0xda, 0x48
};

const static byte cgbwave[16] =
{
	0x00, 0xff, 0x00, 0xff,
	0x00, 0xff, 0x00, 0xff,
	0x00, 0xff, 0x00, 0xff,
	0x00, 0xff, 0x00, 0xff,
};

const static byte sqwave[4][8] =
{
	{  0, 0,-1, 0, 0, 0, 0, 0 },
	{  0,-1,-1, 0, 0, 0, 0, 0 },
	{ -1,-1,-1,-1, 0, 0, 0, 0 },
	{ -1, 0, 0,-1,-1,-1,-1,-1 }
};
*/

const static int freqtab[8] =
{
	(1<<14)*2,
	(1<<14),
	(1<<14)/2,
	(1<<14)/3,
	(1<<14)/4,
	(1<<14)/5,
	(1<<14)/6,
	(1<<14)/7
};





rcvar_t sound_exports[] =
{
	RCV_END
};


static void s1_freq_d(t_gb* gb, int d)
{
	if (gb->rate > (d<<4)) S1.freq = 0;
	else S1.freq = (gb->rate << 17)/d;
}

static void s1_freq(t_gb* gb)
{
	s1_freq_d(gb, 2048 - (((R_NR14&7)<<8) + R_NR13));
}

static void s2_freq(t_gb* gb)
{
	int d = 2048 - (((R_NR24&7)<<8) + R_NR23);
	if (gb->rate > (d<<4)) S2.freq = 0;
	else S2.freq = (gb->rate << 17)/d;
}

static void s3_freq(t_gb* gb)
{
	int d = 2048 - (((R_NR34&7)<<8) + R_NR33);
	if (gb->rate > (d<<3)) S3.freq = 0;
	else S3.freq = (gb->rate << 21)/d;
}

static void s4_freq(t_gb* gb)
{
	S4.freq = (freqtab[R_NR43&7] >> (R_NR43 >> 4)) * gb->rate;
	if (S4.freq >> 18) S4.freq = 1<<18;
}

/*
void sound_dirty()
{
	S1.swlen = ((R_NR10>>4) & 7) << 14;
	S1.len = (64-(R_NR11&63)) << 13;
	S1.envol = R_NR12 >> 4;
	S1.endir = (R_NR12>>3) & 1;
	S1.endir |= S1.endir - 1;
	S1.enlen = (R_NR12 & 7) << 15;
	s1_freq();
	S2.len = (64-(R_NR21&63)) << 13;
	S2.envol = R_NR22 >> 4;
	S2.endir = (R_NR22>>3) & 1;
	S2.endir |= S2.endir - 1;
	S2.enlen = (R_NR22 & 7) << 15;
	s2_freq();
	S3.len = (256-R_NR31) << 20;
	s3_freq();
	S4.len = (64-(R_NR41&63)) << 13;
	S4.envol = R_NR42 >> 4;
	S4.endir = (R_NR42>>3) & 1;
	S4.endir |= S4.endir - 1;
	S4.enlen = (R_NR42 & 7) << 15;
	s4_freq();
}

void sound_off()
{
	memset(&S1, 0, sizeof S1);
	memset(&S2, 0, sizeof S2);
	memset(&S3, 0, sizeof S3);
	memset(&S4, 0, sizeof S4);
	R_NR10 = 0x80;
	R_NR11 = 0xBF;
	R_NR12 = 0xF3;
	R_NR14 = 0xBF;
	R_NR21 = 0x3F;
	R_NR22 = 0x00;
	R_NR24 = 0xBF;
	R_NR30 = 0x7F;
	R_NR31 = 0xFF;
	R_NR32 = 0x9F;
	R_NR33 = 0xBF;
	R_NR41 = 0xFF;
	R_NR42 = 0x00;
	R_NR43 = 0x00;
	R_NR44 = 0xBF;
	R_NR50 = 0x77;
	R_NR51 = 0xF3;
	R_NR52 = 0xF1;
	sound_dirty();
}

*/


void sound_reset(t_gb* gb)
{
    pcm_init(gb);
	//memset(&snd, 0, sizeof snd);
	if (gb->hz) gb->rate = (1<<21) / gb->hz;
	else gb->rate = 0;
	//post("sound_reset %d", snd.rate);
	//KBFIX play with this...
	//memcpy(WAVE, hw.cgb ? cgbwave : dmgwave, 16);
	//memcpy(gb->wave, cgbwave, 16);
	//memcpy(WAVE, dmgwave, 16);
	//memcpy((gb->hi)+0x30, gb->wave, 16);
	//sound_off();
	sound_write(gb, RI_NR50, 0x00FF);
    sound_write(gb, RI_NR51, 0x00FF);
}



void sound_mix(t_gb* gb, t_float* bufL, t_float* bufR, int size)
{
	int s, l, r, f, n;
	int i;
	
	unsigned short val;
	//float va;
/*
	if (!gb->rate || cpu.snd < gb->rate) return;
	for (; cpu.snd >= gb->rate; cpu.snd -= gb->rate)
*/

	for (i=0; i <= size; i++)
	{
		l = r = 0;

		if (S1.on)
		{
		    
			//s = sqwave[R_NR11>>6][(S1.pos>>18)&7] & S1.envol;
			S1.pos += S1.freq;
			if ((R_NR14 & 64) && ((S1.cnt += gb->rate) >= S1.len))
				S1.on = 0;
			if (S1.enlen && (S1.encnt += gb->rate) >= S1.enlen)
			{
				S1.encnt -= S1.enlen;
				S1.envol += S1.endir;
				if (S1.envol < 0) S1.envol = 0;
				if (S1.envol > 15) S1.envol = 15;
			}
			if (S1.swlen && (S1.swcnt += gb->rate) >= S1.swlen)
			{
				S1.swcnt -= S1.swlen;
				f = S1.swfreq;
				n = (R_NR10 & 7);
				if (R_NR10 & 8) f -= (f >> n);
				else f += (f >> n);
				if (f > 2047)
					S1.on = 0;
				else
				{
					S1.swfreq = f;
					R_NR13 = f;
					R_NR14 = (R_NR14 & 0xF8) | (f>>8);
					s1_freq_d(gb, 2048 - f);
				}
			}
			s <<= 2;
			if (R_NR51 & 1) {
			  r += s;
			}
			if (R_NR51 & 16) {
			  l += s;
			}
		}
		
		if (S2.on)
		{
			//s = sqwave[R_NR21>>6][(S2.pos>>18)&7] & S2.envol;
			S2.pos += S2.freq;
			if ((R_NR24 & 64) && ((S2.cnt += gb->rate) >= S2.len))
				S2.on = 0;
			if (S2.enlen && (S2.encnt += gb->rate) >= S2.enlen)
			{
				S2.encnt -= S2.enlen;
				S2.envol += S2.endir;
				if (S2.envol < 0) S2.envol = 0;
				if (S2.envol > 15) S2.envol = 15;
			}
			s <<= 2;
			if (R_NR51 & 2) r += s;
			if (R_NR51 & 32) l += s;
		}
		
		if (S3.on)
		{
			//s = gb->wave[(S3.pos>>22) & 15];
			if (S3.pos & (1<<21)) s &= 15;
			else s >>= 4;
			s -= 8;
			S3.pos += S3.freq;
			if ((R_NR34 & 64) && ((S3.cnt += gb->rate) >= S3.len))
				S3.on = 0;
			if (R_NR32 & 96) s <<= (3 - ((R_NR32>>5)&3));
			else s = 0;
			if (R_NR51 & 4) r += s;
			if (R_NR51 & 64) l += s;
		}

		if (S4.on)
		{
			if (R_NR43 & 8) s = 1 & (noise7[
				(S4.pos>>20)&15] >> (7-((S4.pos>>17)&7)));
			else s = 1 & (noise15[
				(S4.pos>>20)&4095] >> (7-((S4.pos>>17)&7)));
			s = (-s) & S4.envol;
			S4.pos += S4.freq;
			if ((R_NR44 & 64) && ((S4.cnt += gb->rate) >= S4.len))
				S4.on = 0;
			if (S4.enlen && (S4.encnt += gb->rate) >= S4.enlen)
			{
				S4.encnt -= S4.enlen;
				S4.envol += S4.endir;
				if (S4.envol < 0) S4.envol = 0;
				if (S4.envol > 15) S4.envol = 15;
			}
			s += s << 1;
			if (R_NR51 & 8) r += s;
			if (R_NR51 & 128) l += s;
		}
		l *= (R_NR50 & 0x07);
		r *= ((R_NR50 & 0x70)>>4);
		l >>= 4;
		r >>= 4;
		
		if (l > 127) l = 127;
		else if (l < -128) l = -128;
		if (r > 127) r = 127;
		else if (r < -128) r = -128;

		val =  r + 128;
		*(bufR++) = (((float)val) - 128.0)/128.0;
			
		val =  l + 128;
		*(bufL++) = (((float)val) - 128.0)/128.0;
	}
	R_NR52 = (R_NR52&0xf0) | S1.on | (S2.on<<1) | (S3.on<<2) | (S4.on<<3);
}

/*

byte sound_read(byte r)
{
	sound_mix();
	return REG(r);
}
*/

void s1_init(t_gb* gb)
{
	S1.swcnt = 0;
	S1.swfreq = ((R_NR14&7)<<8) + R_NR13;
	S1.envol = R_NR12 >> 4;
	S1.endir = (R_NR12>>3) & 1;
	S1.endir |= S1.endir - 1;
	S1.enlen = (R_NR12 & 7) << 15;
	if (!S1.on) S1.pos = 0;
	S1.on = 1;
	S1.cnt = 0;
	S1.encnt = 0;
}

void s2_init(t_gb* gb)
{
	S2.envol = R_NR22 >> 4;
	S2.endir = (R_NR22>>3) & 1;
	S2.endir |= S2.endir - 1;
	S2.enlen = (R_NR22 & 7) << 15;
	if (!S2.on) S2.pos = 0;
	S2.on = 1;
	S2.cnt = 0;
	S2.encnt = 0;
}

void s3_init(t_gb* gb)
{
	int i;
	if (!S3.on) S3.pos = 0;
	S3.cnt = 0;
	S3.on = R_NR30 >> 7;
	
	if (S3.on) for (i = 0; i < 16; i++)
	    (gb->hi)[i+0x30] = 0x13 ^ (gb->hi)[i+0x31];
		//ram.hi[i+0x30] = 0x13 ^ ram.hi[i+0x31];
}

void s4_init(t_gb* gb)
{
	S4.envol = R_NR42 >> 4;
	S4.endir = (R_NR42>>3) & 1;
	S4.endir |= S4.endir - 1;
	S4.enlen = (R_NR42 & 7) << 15;
	S4.on = 1;
	S4.pos = 0;
	S4.cnt = 0;
	S4.encnt = 0;
}


void sound_write(t_gb* gb, byte r, byte b)
{
	
	//if (!(R_NR52 & 128) && r != RI_NR52) return;
	if ((r & 0xF0) == 0x30)
	{
		//KBFIX if (S3.on) sound_mix(gb);
		//if (!S3.on) gb->wave[r-0x30] = (gb->hi)[r] = b;
			//gb->wave[r-0x30] = ram.hi[r] = b;
		//return;
	}
	// KBFIX sound_mix();
	switch (r)
	{
	case RI_NR10:
		R_NR10 = b;
		S1.swlen = ((R_NR10>>4) & 7) << 14;
		S1.swfreq = ((R_NR14&7)<<8) + R_NR13;
		break;
	case RI_NR11:
		R_NR11 = b;
		S1.len = (64-(R_NR11&63)) << 13;
		break;
	case RI_NR12:
		R_NR12 = b;
		S1.envol = R_NR12 >> 4;
		S1.endir = (R_NR12>>3) & 1;
		S1.endir |= S1.endir - 1;
		S1.enlen = (R_NR12 & 7) << 15;
		break;
	case RI_NR13:
		R_NR13 = b;
		s1_freq(gb);
		break;
	case RI_NR14:
		R_NR14 = b;
		s1_freq(gb);
		if (b & 128) s1_init(gb);
		break;
	case RI_NR21:
		R_NR21 = b;
		S2.len = (64-(R_NR21&63)) << 13;
		break;
	case RI_NR22:
		R_NR22 = b;
		S2.envol = R_NR22 >> 4;
		S2.endir = (R_NR22>>3) & 1;
		S2.endir |= S2.endir - 1;
		S2.enlen = (R_NR22 & 7) << 15;
		break;
	case RI_NR23:
		R_NR23 = b;
		s2_freq(gb);
		break;
	case RI_NR24:
		R_NR24 = b;
		s2_freq(gb);
		if (b & 128) s2_init(gb);
		break;
	case RI_NR30:
		R_NR30 = b;
		if (!(b & 128)) S3.on = 0;
		break;
	case RI_NR31:
		R_NR31 = b;
		S3.len = (256-R_NR31) << 13;
		break;
	case RI_NR32:
		R_NR32 = b;
		break;
	case RI_NR33:
		R_NR33 = b;
		s3_freq(gb);
		break;
	case RI_NR34:
		R_NR34 = b;
		s3_freq(gb);
		if (b & 128) s3_init(gb);
		break;
	case RI_NR41:
		R_NR41 = b;
		S4.len = (64-(R_NR41&63)) << 13;
		break;
	case RI_NR42:
		R_NR42 = b;
		S4.envol = R_NR42 >> 4;
		S4.endir = (R_NR42>>3) & 1;
		S4.endir |= S4.endir - 1;
		S4.enlen = (R_NR42 & 7) << 15;
		break;
	case RI_NR43:
		R_NR43 = b;
		s4_freq(gb);
		break;
	case RI_NR44:
		R_NR44 = b;
		if (b & 128) s4_init(gb);
		break;
	case RI_NR50:
		R_NR50 = b;
		break;
	case RI_NR51:
		R_NR51 = b;
		break;
	case RI_NR52:
		R_NR52 = b;
		//if (!(R_NR52 & 128))
			//sound_off();
		break;
	default:
		return;
	}
}




