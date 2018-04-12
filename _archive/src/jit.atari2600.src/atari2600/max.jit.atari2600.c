// jit.atari2600.c
// Atari 2600 emulator. 
// author: kyle buza 2006
//
// Modified from max.jit.gl.gridshape example
// author: jkc
// © 2002 cycling '74

#ifdef __cplusplus

extern "C" {
#endif

#include "jit.common.h"
#include "jit.gl.h"
#include "z_dsp.h"
#include "JitterStruct.h"

typedef struct _max_jit_atari2600 
{
	t_pxobject      ob;
	void			*obex;
} t_max_jit_atari2600;

t_jit_err jit_atari2600_init(void); 
void *max_jit_atari2600_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_atari2600_free(t_max_jit_atari2600 *x);
void max_jit_atari2600_dsp(t_max_jit_atari2600 *x, t_signal **sp, short *count);
t_int *atari2600_perform(t_int *w); 
void max_atari2600_list(t_max_jit_atari2600 *x, Symbol *s, short ac, Atom *av);
t_class *max_jit_atari2600_class;

//Below are the Stella (C++) methods we need to access from max.jit.atari2600.c
void sendDirection(struct Console*, char dir, char state);

void sendDir(struct Console* c, char dir, char state) 
{
	sendDirection(c, dir, state);
}

void fillAudioBuffer(struct Console* c, float* buf, unsigned int size);

void processAudio(struct Console* c, float* buf, unsigned int size) 
{
	fillAudioBuffer(c, buf, size);
}

void tiamanip(struct Console* c, int v1, int v2, int v3, int v4);

void manipulateTia(struct Console* c, int v1, int v2, int v3, int v4) 
{
	tiamanip(c, v1, v2, v3, v4);
}		 	

void main(void)
{	
	void *classex, *jitclass;
	
	jit_atari2600_init();	
	setup((t_messlist **)&max_jit_atari2600_class, (method)max_jit_atari2600_new, (method)max_jit_atari2600_free, (short)sizeof(t_max_jit_atari2600), 
		0L, A_GIMME, 0);

	addmess((method)max_jit_atari2600_dsp, "dsp", A_CANT, 0);
	dsp_initclass();

	classex = max_jit_classex_setup(calcoffset(t_max_jit_atari2600, obex));
	jitclass = jit_class_findbyname(gensym("jit_atari2600"));	
    max_jit_classex_standard_wrap(classex, jitclass, 0);   //getattributes/dumpout/maxjitclassaddmethods/etc    
    addmess((method)max_jit_ob3d_assist, "assist", A_CANT,0);  

	addmess((method)max_atari2600_list, "list", A_GIMME, 0);

	// add methods for 3d drawing
    max_ob3d_setup();
}

void max_atari2600_list(t_max_jit_atari2600 *x, Symbol *s, short ac, Atom *av) 
{
	long dir;
	long status;
	long result = 0;

	if(ac != 2) return;
	dir = av->a_w.w_long;
	av++;
	status = av->a_w.w_long;
	result = ((dir & 0x00FF) << 8) | (status & 0x00FF);
	void *o = max_jit_obex_jitob_get(x);
	t_jit_atari2600* y = (t_jit_atari2600*)max_jit_obex_jitob_get(x);
	if(!y->theConsole) return;
	sendDir(y->theConsole, (dir & 0x00FF),  status & 0x00FF);
}

void max_jit_atari2600_dsp(t_max_jit_atari2600 *x, t_signal **sp, short *count) {
	
	dsp_add(atari2600_perform, 3, sp[0]->s_vec, sp[0]->s_n, x);
}

t_int *atari2600_perform(t_int *w) 
{
  t_float *outL = (t_float *)(w[1]);

  //Get the jit_obex
  t_jit_atari2600* y = (t_jit_atari2600*)max_jit_obex_jitob_get((t_max_jit_atari2600 *)(w[3]));
  if(!y->theConsole) return (w + 4);
  processAudio(y->theConsole, outL, (int)(w[2]));
  
  return (w + 4);
}


void max_jit_atari2600_free(t_max_jit_atari2600 *x)
{
	dsp_free((t_pxobject *)x);
	max_jit_ob3d_detach(x);
	jit_object_free(max_jit_obex_jitob_get(x));
	max_jit_obex_free(x);
}

void *max_jit_atari2600_new(t_symbol *s, long argc, t_atom *argv)
{
	t_max_jit_atari2600 *x;
	void *jit_ob;
	long attrstart;
	t_symbol *dest_name_sym = _jit_sym_nothing;

	if (x = (t_max_jit_atari2600 *)max_jit_obex_new(max_jit_atari2600_class, gensym("jit_atari2600"))) 
	{
		//get normal args
		attrstart = max_jit_attr_args_offset(argc,argv);
		if (attrstart&&argv) 
		{
			jit_atom_arg_getsym(&dest_name_sym, 0, attrstart, argv);
		}

		if (jit_ob = jit_object_new(gensym("jit_atari2600"), dest_name_sym)) 
		{
			max_jit_obex_jitob_set(x, jit_ob);
			max_jit_obex_dumpout_set(x, outlet_new(x,NULL));
			max_jit_attr_args(x, argc, argv);	

			// attach the jit object's ob3d to a new outlet for sending drawing messages.	
			max_jit_ob3d_attach(x, (t_jit_object*)jit_ob, outlet_new(x, "jit_matrix"));	

			dsp_setup((t_pxobject *)x,0);
			outlet_new((t_pxobject *)x, "signal");
		} 
		else 
		{
			error("jit.atari2600: could not allocate object");
			freeobject((t_object *)x);
			x = NULL;
		}
	}
	return (x);
}

#ifdef __cplusplus
}
#endif


