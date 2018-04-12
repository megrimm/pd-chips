
#define MAX_CARTNAME_LEN 128

//Stella is written in C++.  This is how we interface C++ structs from C.

struct Console;

typedef struct _jit_atari2600 
{
	t_object			ob;
	void				*ob3d;
	t_jit_glchunk		*chunk;
	long				recalc;
	t_symbol			*shape;
	long				dim[2];
	long				rom_vis_dim[2];
	long				game_dim[2];
	float				rad_minor;
	char				gridmode;
	char				displaylist;
	long				visualizer;
	char                rom[MAX_CARTNAME_LEN];
	long                clock;
	long                palette;
	long                visC;
	long                m2;
	long                m3;
	long                m4;
	long                m5;
	long                m6;
	long                m7;
	long                m8;
	long                m9;
	long                m10;
	long                m11;
	long                m12;
	long                m13;
	Console				*theConsole;
	GLuint				dlref; 
} t_jit_atari2600;