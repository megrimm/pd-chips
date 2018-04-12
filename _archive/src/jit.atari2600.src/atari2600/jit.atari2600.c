// jit.atari2600.c
// Atari 2600 emulator. 
// author: kyle buza 2006
//
// Modified from jit.gl.gridshape example
// author: jkc
// © 2002 cycling '74

#ifdef __cplusplus
#include "JitterEntry.hxx"
extern "C" {
#endif

#include <math.h>

#include "jit.common.h"
#include "jit.gl.h"
#include "ext_strings.h"
#include "JitterStruct.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

void *_jit_atari2600_class;

t_jit_err jit_atari2600_init(void) ;
t_jit_atari2600 *jit_atari2600_new(t_symbol * dest_name);
void jit_atari2600_free(t_jit_atari2600 *x);

t_jit_err jit_atari2600_shape(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_dim(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_rad_minor(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_rom(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_visualizer(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_clock(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m2(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m3(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m4(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m5(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m6(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m7(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m8(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m9(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m10(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m11(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m12(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_m13(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_palette(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_visC(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv);
t_jit_err jit_atari2600_draw(t_jit_atari2600 *x);
t_jit_err jit_atari2600_dest_changed(t_jit_atari2600 *x);
void jit_atari2600_get_color(t_jit_atari2600 *x, float *red, float *green, float *blue, float *alpha);

t_jit_err jit_atari2600_recalc(t_jit_atari2600 *x);
t_jit_err jit_atari2600_dest_closing(t_jit_atari2600 *x);

void calc_sphere(t_jit_atari2600 *x);
void calc_cylinder(t_jit_atari2600 *x);
void calc_torus(t_jit_atari2600 *x);
void calc_opencylinder(t_jit_atari2600 *x);
void calc_circle(t_jit_atari2600 *x);
void calc_cube(t_jit_atari2600 *x);
void calc_opencube(t_jit_atari2600 *x);
void calc_plane(t_jit_atari2600 *x);

void color_surface(t_jit_atari2600 *x);

void draw_grid(t_jit_object *matrix, GLenum mode);

t_symbol *ps_circle,*ps_sphere,*ps_torus,*ps_cylinder,*ps_opencylinder,*ps_cube,*ps_opencube,*ps_plane;


//Below are the Stella (C++) methods we need to access from jit.atari2600.c
unsigned short* get_jit_pix(struct Console*);
void display_reads(Console* c, long val);
void set_visualizer_const(Console* c, long val);
void mod_palette(Console* c, long index, long val);
void call_manipulator(struct Console*, long val);
void call_manipulator2(struct Console*, long val);
void call_manipulator3(struct Console*, long val);
void call_manipulator4(struct Console*, long val);
void call_manipulator5(struct Console*, long val);
void call_manipulator6(struct Console*, long val);
void call_manipulator7(struct Console*, long val);
void call_manipulator8(struct Console*, long val);
void call_manipulator9(struct Console*, long val);
void call_manipulator10(struct Console*, long val);
void call_manipulator11(struct Console*, long val);
void call_manipulator12(struct Console*, long val);
void call_manipulator13(struct Console*, long val);

int call_updateConsole(Console* c, int rowStride, int dim0, int dim1, char* bp);

int updateConsole(struct Console* p, int rowStride, int dim0, int dim1, char* bp) 
{
	return call_updateConsole(p, rowStride, dim0, dim1, bp);
}

unsigned short* getTiaPixels(struct Console* c) 
{
	return get_jit_pix(c);
}

void manipulate(struct Console* c, long val) 
{
	call_manipulator(c, val);
}

void manipulate2(struct Console* c, long val) {
	call_manipulator2(c, val);
}

void manipulate3(struct Console* c, long val) 
{
	call_manipulator3(c, val);
}

void manipulate4(struct Console* c, long val) 
{
	call_manipulator4(c, val);
}

void manipulate5(struct Console* c, long val) 
{
	call_manipulator5(c, val);
}

void manipulate6(struct Console* c, long val) 
{
	call_manipulator6(c, val);
}

void manipulate7(struct Console* c, long val) 
{
	call_manipulator7(c, val);
}

void manipulate8(struct Console* c, long val) 
{
	call_manipulator8(c, val);
}

void manipulate9(struct Console* c, long val) 
{
	call_manipulator9(c, val);
}

void manipulate10(struct Console* c, long val) 
{
	call_manipulator10(c, val);
}

void manipulate11(struct Console* c, long val) 
{
	call_manipulator11(c, val);
}

void manipulate12(struct Console* c, long val) 
{
	call_manipulator12(c, val);
}
void manipulate13(struct Console* c, long val) 
{
	call_manipulator13(c, val);
}

void set_vis_const(Console* c, long val) 
{
	set_visualizer_const(c, val);
}

void displayReads(struct Console* c, long val) 
{
	display_reads(c, val);
}

void modify_palette(struct Console* c, long index, long val) 
{
	mod_palette(c, index, val);
}

// --------------------------------------------------------------------------------
//

t_jit_err jit_atari2600_init(void) 
{
	long attrflags=0;
	long ob3d_flags=0;
	t_jit_object *attr;
	void * ob3d;
	
	_jit_atari2600_class = jit_class_new("jit_atari2600", (method)jit_atari2600_new, (method)jit_atari2600_free,
		sizeof(t_jit_atari2600),A_CANT,0L); //A_CANT = untyped
	
	// set up object extension for 3d object, customized with flags
	ob3d = jit_ob3d_setup(_jit_atari2600_class, calcoffset(t_jit_atari2600, ob3d), ob3d_flags);

	// add attributes
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;
	// shape
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"shape",_jit_sym_symbol,attrflags,
		(method)0L,(method)jit_atari2600_shape,calcoffset(t_jit_atari2600, shape));	
	jit_class_addattr(_jit_atari2600_class,attr);	
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset_array,"dim",_jit_sym_long,2,attrflags,
		(method)0L,(method)jit_atari2600_dim,0/*fix*/,calcoffset(t_jit_atari2600,dim));
	jit_attr_addfilterset_clip(attr,2,0,TRUE,FALSE);
	jit_class_addattr(_jit_atari2600_class,attr);	
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"rad_minor",_jit_sym_float32,attrflags,
		(method)0L,(method)jit_atari2600_rad_minor,calcoffset(t_jit_atari2600, rad_minor));	
	jit_class_addattr(_jit_atari2600_class,attr);		
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"gridmode",_jit_sym_char,attrflags,
		(method)0L,(method)0L,calcoffset(t_jit_atari2600, gridmode));	
	jit_class_addattr(_jit_atari2600_class,attr);	
	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"rom",_jit_sym_symbol,attrflags,
		(method)0L,(method)jit_atari2600_rom,calcoffset(t_jit_atari2600, rom));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"visualizer",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_visualizer,calcoffset(t_jit_atari2600, visualizer));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"clock",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_clock,calcoffset(t_jit_atari2600, clock));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m2",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m2,calcoffset(t_jit_atari2600, m2));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m3",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m3,calcoffset(t_jit_atari2600, m3));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m4",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m4,calcoffset(t_jit_atari2600, m4));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m5",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m5,calcoffset(t_jit_atari2600, m5));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m6",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m6,calcoffset(t_jit_atari2600, m6));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m7",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m7,calcoffset(t_jit_atari2600, m7));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m8",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m8,calcoffset(t_jit_atari2600, m8));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m9",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m9,calcoffset(t_jit_atari2600, m9));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m10",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m10,calcoffset(t_jit_atari2600, m10));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m11",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m11,calcoffset(t_jit_atari2600, m11));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m12",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m12,calcoffset(t_jit_atari2600, m12));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"m13",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_m13,calcoffset(t_jit_atari2600, m13));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"visC",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_visC,calcoffset(t_jit_atari2600, visC));	
	jit_class_addattr(_jit_atari2600_class,attr);

	attr = (t_jit_object*)jit_object_new(_jit_sym_jit_attr_offset,"palette",_jit_sym_long,attrflags,
		(method)0L,(method)jit_atari2600_palette,calcoffset(t_jit_atari2600, palette));	
	jit_class_addattr(_jit_atari2600_class,attr);

	
	// handle draw method.  called in automatic mode by jit.gl.render, or otherwise through ob3d when banged.
	// this is A_CANT because draw setup needs to happen in the ob3d beforehand. 
	jit_class_addmethod(_jit_atari2600_class, (method)jit_atari2600_draw, "ob3d_draw", A_CANT, 0L);
	
	// handle dest_changed method.
	// this method is called by jit.render when the destination context changes: for example,
	// when the user moves the window from one monitor to another.  Anything your object keeps
	// in the OpenGL machine -- textures, display lists, vertex shaders, etc. -- will need to
	// be rebuilt here. 
	jit_class_addmethod(_jit_atari2600_class, (method)jit_atari2600_dest_changed, "dest_changed", A_CANT, 0L);
	
	// must register for ob3d	
	jit_class_addmethod(_jit_atari2600_class, (method)jit_object_register, 			"register",		A_CANT, 0L);
	jit_class_addmethod(_jit_atari2600_class, (method)jit_atari2600_dest_closing, 	"dest_closing",	A_CANT, 0L);

	jit_class_register(_jit_atari2600_class);

	ps_circle 			= gensym("circle");
	ps_sphere 			= gensym("sphere");
	ps_torus 			= gensym("torus");
	ps_cylinder 		= gensym("cylinder");
	ps_opencylinder 	= gensym("opencylinder");
	ps_cube 			= gensym("cube");
	ps_opencube 		= gensym("opencube");
	ps_plane 			= gensym("plane");

	return JIT_ERR_NONE;
}


t_jit_atari2600 *jit_atari2600_new(t_symbol * dest_name)
{
	t_jit_atari2600 *x;
	
	post("\njit.atari2600 :: (version 0.7) by Kyle Buza.  Based on the Stella Atari 2600 emulator version 1.4.1.  This software has NO WARRANTY\n");

	// make jit object
	if (x = (t_jit_atari2600 *)jit_object_alloc(_jit_atari2600_class)) 
	{
		// create and attach ob3d
		jit_ob3d_new(x, dest_name);
		
		// set instance variable defaults
		x->shape = ps_plane;
		x->dim[0] = 160;
		x->dim[1] = 210;	
		x->chunk = jit_glchunk_grid_new(_jit_sym_gl_quad_grid, 12, x->dim[0], x->dim[1]);
		x->rad_minor = 0.5;
		x->recalc = 1;
		x->dlref = 0;
		x->gridmode = 0;	
		x->visualizer = 0;
		x->palette = 0;
		strcpy(x->rom, "");
	} 
	else 
	{
		x = NULL;
	}	
	return x;
}

t_jit_err jit_atari2600_shape(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv)
{
	if (argc&&argv) {
		x->shape = jit_atom_getsym(argv);
		x->recalc = 1;		
	}
	
	return JIT_ERR_NONE;
}


t_jit_err jit_atari2600_dim(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv)
{
	if (argc&&argv&&x->chunk&&x->chunk->m_vertex) {
		t_jit_matrix_info info;
		jit_object_method(x->chunk->m_vertex,_jit_sym_dim, argc, argv);
		jit_object_method(x->chunk->m_vertex,_jit_sym_getinfo,&info);
		x->dim[0] = info.dim[0];
		x->dim[1] = info.dim[1];
		x->recalc = 1;		
	}
	
	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_rad_minor(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv)
{
	if (argc&&argv) {
		x->rad_minor = jit_atom_getfloat(argv);
		if (x->shape==ps_torus)
			x->recalc = 1;		
	}
	
	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_visualizer(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) 
{
	t_jit_matrix_info info;
	unsigned short* pixels;
	int i,j;
	void* m;

	if (argc&&argv) {
		x->visualizer = jit_atom_getlong(argv) > 0 ? 1 : 0;
		if (!(m=x->chunk->m_vertex)) {
			return JIT_ERR_NONE;
		}
		jit_object_method(m,_jit_sym_getinfo,&info);
		if(!x->theConsole) return JIT_ERR_NONE;
		pixels = getTiaPixels(x->theConsole);
		displayReads(x->theConsole, x->visualizer);
		//Now clear the buffer
		for (i=0;i<info.dim[1];i++) {
			for (j=0;j<info.dim[0];j++) {
				pixels[(i*info.dim[0] + j) % (160 * 210)] = 0;
			}
		}
		x->recalc = 1;
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_clock(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->clock = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate(x->theConsole, x->clock);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m2(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m2 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate2(x->theConsole, x->m2);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m3(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m3 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate3(x->theConsole, x->m3);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m4(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m4 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate4(x->theConsole, x->m4);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m5(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m5 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate5(x->theConsole, x->m5);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m6(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m6 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate6(x->theConsole, x->m6);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m7(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m7 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate7(x->theConsole, x->m7);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m8(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m8 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate8(x->theConsole, x->m8);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m9(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m9 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate9(x->theConsole, x->m9);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m10(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m10 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate10(x->theConsole, x->m10);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m11(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m11 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate11(x->theConsole, x->m11);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m12(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m12 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate12(x->theConsole, x->m12);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_m13(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->m13 = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		manipulate13(x->theConsole, x->m13);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_visC(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv) {

	if (argc&&argv) {
		x->visC = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		set_vis_const(x->theConsole, x->visC);
	}

	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_rom(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv)
{
	if (argc&&argv) {
		if(strlen(argv[0].a_w.w_sym->s_name) > MAX_CARTNAME_LEN-1) {
			post("ERROR.  ROM name attribute too long.  Please shorten the cart name.");
			return JIT_ERR_GENERIC; 
		}
		strcpy(x->rom, argv[0].a_w.w_sym->s_name);
		char* romArray[] = {x->rom};

		//This creates the console.
		x->theConsole = (Console*)jitterEntry(romArray[0]);
		if(!x->theConsole) {
			post("Could not initialize the Atari 2600. :: ERROR 40");
			return JIT_ERR_NONE;
		}
		x->recalc = 1;		
	}
	return JIT_ERR_NONE;
}


t_jit_err jit_atari2600_palette(t_jit_atari2600 *x, void *attr, long argc, t_atom *argv)
{
	if (argc&&argv) {
		x->palette = jit_atom_getlong(argv);
		if(!x->theConsole) return JIT_ERR_NONE;
		modify_palette(x->theConsole, (x->palette & 0x00FF),(x->palette >> 8 ));
		x->recalc = 1;		
	}
	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_recalc(t_jit_atari2600 *x)
{
	if (x->chunk&&x->chunk->m_vertex) {
		if (x->shape==ps_plane) {
			calc_plane(x);
		} else if (x->shape==ps_cube) {
			calc_cube(x);
		} else if (x->shape==ps_opencube) {
			calc_opencube(x);
		} else if (x->shape==ps_circle) {
			calc_circle(x);
		} else if (x->shape==ps_opencylinder) {
			calc_opencylinder(x);
		} else if (x->shape==ps_torus) {
			calc_torus(x);
		} else if (x->shape==ps_cylinder) {
			calc_cylinder(x);
		} else {
			calc_sphere(x);			
		}
	}
	
	return JIT_ERR_NONE;
}

void jit_atari2600_free(t_jit_atari2600 *x)
{
	if(x->theConsole) Cleanup();
	x->theConsole = 0;

	if (x->chunk)
		jit_glchunk_delete(x->chunk);

	if (jit_ob3d_set_context(x)==JIT_ERR_NONE) {
		if (x->dlref)
			glDeleteLists(x->dlref,1);
	}
	jit_ob3d_free(x);
}

t_jit_err jit_atari2600_dest_closing(t_jit_atari2600 *x)
{
	if (x->dlref) {
		glDeleteLists(x->dlref,1);
		x->dlref=0;
		x->recalc=1;
	}
	return JIT_ERR_NONE;
}

t_jit_err jit_atari2600_draw(t_jit_atari2600 *x)
{
	t_jit_err result = JIT_ERR_NONE;
	GLenum prim;
	
	prim = (x->gridmode) ? GL_TRIANGLE_STRIP : GL_QUAD_STRIP;
	
	if (x->recalc) {
		jit_atari2600_recalc(x);		
		x->recalc = 0;
	}
	
	// draw our chunk of OpenGL geometry. 
	if (x->chunk&&x->chunk->m_vertex) {
		color_surface(x);
		result = jit_ob3d_draw_chunk(x->ob3d, x->chunk); //output matrix
	}	
	return result;
}

t_jit_err jit_atari2600_dest_changed(t_jit_atari2600 *x)
{
	if (x->dlref) x->recalc=1;
	return JIT_ERR_NONE;
}

void jit_atari2600_get_color(t_jit_atari2600 *x, float *red, float *green, float *blue, float *alpha)
{
	long ac=0;
	t_atom *av=NULL;
	
	jit_object_method(x,gensym("getcolor"),&ac,&av);
	if ((ac==4)&&av) {
		*red 	= jit_atom_getfloat(av);
		*green 	= jit_atom_getfloat(av+1);
		*blue 	= jit_atom_getfloat(av+2);
		*alpha 	= jit_atom_getfloat(av+3);
		jit_freebytes(av,ac*sizeof(t_atom));
	}
}


void calc_sphere(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;
	int numminor = x->dim[0]-1;
	double majorstep = (M_PI/(double)nummajor);
	double minorstep = (2.0*M_PI/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);
		
	for (i=0;i<=nummajor;i++) {
		double a = i*majorstep;
		double r = jit_math_sin(a);
		double z = jit_math_cos(a);
		double c,x,y;
	
		p = (float *)(bp + i*rowstride);

		for (j=0;j<=numminor;j++) {
			c = j*minorstep;
			x = r*jit_math_cos(c);
			y = r*jit_math_sin(c);

			//vertex
			*p++ = x;
			*p++ = y; 
			*p++ = z; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = x; 
			*p++ = y; 
			*p++ = z; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}

void calc_opencylinder(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;
	int numminor = x->dim[0]-1;
	double majorstep = (2./(double)nummajor);
	double minorstep = (2.0*M_PI/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);
		
	for (i=0;i<=nummajor;i++) {
		double z = 1. - i*majorstep;
		double c,x,y;
	
		p = (float *)(bp + i*rowstride);

		for (j=0;j<=numminor;j++) {
			c = j*minorstep;
			x = jit_math_cos(c);
			y = jit_math_sin(c);

			//vertex
			*p++ = x; 
			*p++ = y; 
			*p++ = z; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = x; 
			*p++ = y; 
			*p++ = 0.; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}

void calc_cylinder(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-3;
	int numminor = x->dim[0]-1;
	double majorstep = (2./(double)nummajor);
	double minorstep = (2.0*M_PI/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	if (x->dim[1]<3)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);

	p = (float *)bp;
	//top
	for (j=0;j<=numminor;j++) {
		//vertex
		*p++ = 0; 
		*p++ = 0; 
		*p++ = 1; 
		//texture
		*p++ = j*numminor_inv; 
		*p++ = 0;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = 1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
	//body	
	for (i=0;i<=nummajor;i++) {
		double z = 1. - i*majorstep;
		double c,x,y;
	
		p = (float *)(bp + (i+1)*rowstride);

		for (j=0;j<=numminor;j++) {
			c = j*minorstep;
			x = jit_math_cos(c);
			y = jit_math_sin(c);

			//vertex
			*p++ = x; 
			*p++ = y; 
			*p++ = z; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = x; 
			*p++ = y; 
			*p++ = 0.; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
	//bottom
	p = (float *)(bp + (nummajor+2)*rowstride);
	for (j=0;j<=numminor;j++) {
		//vertex
		*p++ = 0; 
		*p++ = 0; 
		*p++ = -1; 
		//texture
		*p++ = j*numminor_inv; 
		*p++ = 1;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = -1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
}

void calc_cube(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-5;
	int numminor = x->dim[0];
	double majorstep = (2./(double)nummajor);
	double minorstep;
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)(numminor-1);
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
	double tx,ty,vx,vy,nx,ny;
	long numface0,numface1,numface2,numface3;
	long tface0,tface1,tface2;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	if (x->dim[1]<3)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);

	numface0 = numminor/4;
	tface0 = numface0;

	numface1 = numminor/2 - tface0;
	tface1 = tface0+numface1;

	numface2 = (3*numminor)/4 - tface1;
	tface2 = tface1+numface2;

	numface3 = numminor - tface2;


	p = (float *)bp;
	//top
	for (j=0;j<numminor;j++) {
		//vertex
		*p++ = 0; 
		*p++ = 0; 
		*p++ = 1; 
		//texture
		*p++ = 0.5; 
		*p++ = 0.5;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = 1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
	p = (float *)(bp + rowstride);
	for (j=0;j<numminor;j++) {
		if (j<tface0) {
			tx=(float)j/(float)(numface0-1);	
			vx=1;
			vy=(2.*tx)-1.;	
			ty=1-tx; //swap
			tx=1;
		} else if (j<tface1) {
			tx=(float)(j-tface0)/(float)(numface1-1);
			vx=1.-(2.*tx);	
			vy=1;
			tx=1-tx;
			ty=0;
		} else if (j<tface2) {
			tx=(float)(j-tface1)/(float)(numface2-1);
			vx=-1;	
			vy=1.-(2.*tx);	
			ty=tx; //swap
			tx=0;
		} else {	
			tx=(float)(j-tface2)/(float)(numface3-1);
			vx=(2.*tx)-1.;	
			vy=-1;
			ty=1;
		}
		//vertex
		*p++ = vx;
		*p++ = vy; 
		*p++ = 1; 
		//texture
		*p++ = tx; 
		*p++ = ty;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = 1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
	
	//body	
	for (i=0;i<=nummajor;i++) {
		double vz = 1. - i*majorstep;
	
		p = (float *)(bp + (i+2)*rowstride);
		
		for (j=0;j<numminor;j++) {
			if (j<tface0) {
				tx=(float)j/(float)(numface0-1);	
				vx=1;
				vy=(2.*tx)-1.;	
				nx=1;
				ny=0;
			} else if (j<tface1) {
				tx=(float)(j-tface0)/(float)(numface1-1);
				vx=1.-(2.*tx);	
				vy=1;
				nx=0;
				ny=1;
			} else if (j<tface2) {
				tx=(float)(j-tface1)/(float)(numface2-1);
				vx=-1;	
				vy=1.-(2.*tx);	
				nx=-1;
				ny=0;
			} else {	
				tx=(float)(j-tface2)/(float)(numface3-1);
				vx=(2.*tx)-1.;	
				vy=-1;
				nx=0;
				ny=-1;
			}
			//vertex
			*p++ = vx; 
			*p++ = vy; 
			*p++ = vz; 
			//texture
			//*p++ = j*numminor_inv; 
			*p++ = tx; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = nx; 
			*p++ = ny; 
			*p++ = 0; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
	//bottom
	p = (float *)(bp + (nummajor+3)*rowstride);
	for (j=0;j<numminor;j++) {
		if (j<tface0) {
			tx=(float)j/(float)(numface0-1);	
			vx=1;
			vy=(2.*tx)-1.;	
			ty=1-tx; //swap
			tx=1;
		} else if (j<tface1) {
			tx=(float)(j-tface0)/(float)(numface1-1);
			vx=1.-(2.*tx);	
			vy=1;
			tx=1-tx;
			ty=0;
		} else if (j<tface2) {
			tx=(float)(j-tface1)/(float)(numface2-1);
			vx=-1;	
			vy=1.-(2.*tx);	
			ty=tx; //swap
			tx=0;
		} else {	
			tx=(float)(j-tface2)/(float)(numface3-1);
			vx=(2.*tx)-1.;	
			vy=-1;
			ty=1;
		}
		//vertex
		*p++ = vx; 
		*p++ = vy; 
		*p++ = -1; 
		//texture
		*p++ = tx; 
		*p++ = ty;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = -1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
	p = (float *)(bp + (nummajor+4)*rowstride);
	for (j=0;j<numminor;j++) {
		//vertex
		*p++ = 0; 
		*p++ = 0; 
		*p++ = -1; 
		//texture
		*p++ = 0.5; 
		*p++ = 0.5;
		//normals
		*p++ = 0; 
		*p++ = 0; 
		*p++ = -1; 
		//color rgba
		*p++ = red;
		*p++ = green;
		*p++ = blue;
		*p++ = alpha;
	}
}

void calc_opencube(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;
	int numminor = x->dim[0];
	double majorstep = (2./(double)nummajor);
	double minorstep;
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)(numminor-1);
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
	double tx,ty,vx,vy,nx,ny;
	long numface0,numface1,numface2,numface3;
	long tface0,tface1,tface2;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	if (x->dim[1]<3)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);

	numface0 = numminor/4;
	tface0 = numface0;

	numface1 = numminor/2 - tface0;
	tface1 = tface0+numface1;

	numface2 = (3*numminor)/4 - tface1;
	tface2 = tface1+numface2;

	numface3 = numminor - tface2;


	p = (float *)bp;
	
	//body	
	for (i=0;i<=nummajor;i++) {
		double vz = 1. - i*majorstep;
	
		p = (float *)(bp + i*rowstride);
		
		for (j=0;j<numminor;j++) {
			if (j<tface0) {
				tx=(float)j/(float)(numface0-1);	
				vx=1;
				vy=(2.*tx)-1.;	
				nx=1;
				ny=0;
			} else if (j<tface1) {
				tx=(float)(j-tface0)/(float)(numface1-1);
				vx=1.-(2.*tx);	
				vy=1;
				nx=0;
				ny=1;
			} else if (j<tface2) {
				tx=(float)(j-tface1)/(float)(numface2-1);
				vx=-1;	
				vy=1.-(2.*tx);	
				nx=-1;
				ny=0;
			} else {	
				tx=(float)(j-tface2)/(float)(numface3-1);
				vx=(2.*tx)-1.;	
				vy=-1;
				nx=0;
				ny=-1;
			}
			//vertex
			*p++ = vx; 
			*p++ = vy; 
			*p++ = vz; 
			//texture
			*p++ = tx; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = nx; 
			*p++ = ny; 
			*p++ = 0; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}


void calc_circle(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;
	int numminor = x->dim[0]-1;
	double majorstep = (1./(double)nummajor);
	double minorstep = (2.0*M_PI/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);
		
	for (i=0;i<=nummajor;i++) {
		double r = 1. - i*majorstep;
		double c,x,y;
	
		p = (float *)(bp + i*rowstride);

		for (j=0;j<=numminor;j++) {
			c = 2.0*M_PI - j*minorstep;
			x = r*jit_math_cos(c);
			y = r*jit_math_sin(c);

			//vertex
			*p++ = x; 
			*p++ = y; 
			*p++ = 0; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = 0; 
			*p++ = 0; 
			*p++ = 1; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}

void calc_plane(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;  //min 2
	int numminor = x->dim[0]-1;  //min 2 
	double majorstep = (2.0/(double)nummajor);
	double minorstep = (2.0/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
	
	if (!x->chunk) return;

	if (!(m=x->chunk->m_vertex)) return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp) return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);
		
	for (i=0;i<=nummajor;i++) {
		double x,y;
		y = i*majorstep-1.;

		p = (float *)(bp + i*rowstride);
		
		for (j=0;j<=numminor;j++) {
			x = 1.-j*minorstep;

			//vertex
			*p++ = x; 
			*p++ = y; 
			*p++ = 0; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = 0; 
			*p++ = 0; 
			*p++ = 1; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}


void calc_torus(t_jit_atari2600 *x)
{
	int nummajor = x->dim[1]-1;
	int numminor = x->dim[0]-1;
	double majorstep = (2.0*M_PI/(double)nummajor);
	double minorstep = (2.0*M_PI/(double)numminor);
	double nummajor_inv = 1./(double)nummajor;
	double numminor_inv = 1./(double)numminor;
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;
	double radminor=x->rad_minor;
		
	if (!x->chunk)
		return;

	if (!(m=x->chunk->m_vertex))
		return;
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp)
		return;
	
	rowstride = info.dimstride[1];
	
	jit_atari2600_get_color(x,&red,&green,&blue,&alpha);
		
	for (i=0;i<=nummajor;i++) {
		double a = i*majorstep;
		double x = jit_math_cos(a);
		double y = jit_math_sin(a);
		double c,b,r,z;
	
		p = (float *)(bp + i*rowstride);

		for (j=0;j<=numminor;j++) {
 		    b = j*minorstep;
 			c = jit_math_cos(b);
      		r = radminor*c + 1.;
      		z = jit_math_sin(b);

			//vertex
			*p++ = x*r; 
			*p++ = y*r; 
			*p++ = z*radminor; 
			//texture
			*p++ = j*numminor_inv; 
			*p++ = i*nummajor_inv;
			//normals
			*p++ = x*c; 
			*p++ = y*c; 
			*p++ = z; 
			//color rgba
			*p++ = red;
			*p++ = green;
			*p++ = blue;
			*p++ = alpha;
		}
	}
}

void color_surface(t_jit_atari2600 *x)
{
	float *p;
	char *bp=NULL;
	void *m;
	int i,j,rowstride;
	t_jit_matrix_info info;
	float red,green,blue,alpha;

	if (!x->chunk) {
		return;
	}

	if (!(m=x->chunk->m_vertex)) {
		return;
	}
	
	jit_object_method(m,_jit_sym_getinfo,&info);
	jit_object_method(m,_jit_sym_getdata,&bp);
	
	if (!bp) {
		return;
	}
	
	rowstride = info.dimstride[1];
	
	p = (float *)bp;

	if(!x->theConsole) return;

	//Update bp inside the emulator.
	updateConsole(x->theConsole, rowstride, info.dim[0], info.dim[1], bp);
}


//_____________________________________________________
// util

#define _I_X	0
#define _I_Y	1
#define _I_Z	2
#define _I_S	3
#define _I_T	4
#define _I_NX	5
#define _I_NY	6
#define _I_NZ	7
#define _I_R	8
#define _I_G	9
#define _I_B	10
#define _I_A	11

//not drawing per vertex color in this object

void draw_grid(t_jit_object *matrix, GLenum mode)
{
	float *p,*p2;
	char *bp=NULL;
	int i,j,rowstride,width,height,planecount,up=0;
	t_jit_matrix_info info;

	if (!matrix) return;

	jit_object_method(matrix,_jit_sym_getinfo,&info);
	jit_object_method(matrix,_jit_sym_getdata,&bp);

	if (!bp) return;

	planecount	= info.planecount;
	rowstride 	= info.dimstride[1];
	height 		= info.dim[1]-1;
	width 		= info.dim[0];
		
	for (i=0;i<height;i++) {
		p = (float *)(bp + i*rowstride);
		p2 = (float *)(bp + (i+1)*rowstride);
		
		glBegin(mode);
		switch (planecount) {		
		case 16:
		case 15:
		case 14:
		case 13:
		case 12:
			for (j=0;j<width;j++) {
				
				glNormal3f(p[_I_NX],p[_I_NY],p[_I_NZ]);
//				glColor4f(p[_I_R],p[_I_G],p[_I_B],p[_I_A]);
				glTexCoord2f(p[_I_S],p[_I_T]);
				glVertex3f(p[_I_X],p[_I_Y],p[_I_Z]);

				glNormal3f(p2[_I_NX],p2[_I_NY],p2[_I_NZ]);
//				glColor4f(p2[_I_R],p2[_I_G],p2[_I_B],p2[_I_A]);
				glTexCoord2f(p2[_I_S],p2[_I_T]);
				glVertex3f(p2[_I_X],p2[_I_Y],p2[_I_Z]);
				
				p+=planecount;
				p2+=planecount;	    	
			}
			break;
		case 11:
		case 10:
		case 9:
		case 8:
			for (j=0;j<width;j++) {
			
				glNormal3f(p[_I_NX],p[_I_NY],p[_I_NZ]);
				glTexCoord2f(p[_I_S],p[_I_T]);
				glVertex3f(p[_I_X],p[_I_Y],p[_I_Z]);

				glNormal3f(p2[_I_NX],p2[_I_NY],p2[_I_NZ]);
				glTexCoord2f(p2[_I_S],p2[_I_T]);
				glVertex3f(p2[_I_X],p2[_I_Y],p2[_I_Z]);

				p+=planecount;
				p2+=planecount;	    	
			}
			break;
		case 7:
		case 6:
		case 5:
			for (j=0;j<width;j++) {
			
				glTexCoord2f(p[_I_S],p[_I_T]);
				glVertex3f(p[_I_X],p[_I_Y],p[_I_Z]);

				glTexCoord2f(p2[_I_S],p2[_I_T]);
				glVertex3f(p2[_I_X],p2[_I_Y],p2[_I_Z]);

				p+=planecount;
				p2+=planecount;	    	
			}
			break;
		case 4:
		case 3:
			for (j=0;j<width;j++) {
			
				glVertex3f(p[_I_X],p[_I_Y],p[_I_Z]);

				glVertex3f(p2[_I_X],p2[_I_Y],p2[_I_Z]);

				p+=planecount;
				p2+=planecount;	    	
			}
			break;
		}
		glEnd();
	}
}


#ifdef __cplusplus
}
#endif
