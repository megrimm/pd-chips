// maxus germanus 2011

#include "m_pd.h"
#include "oneline~.h"
#include <math.h>

static t_class *oneline_class;

typedef struct _oneline {
    t_object x_obj;
    t_float x_f;
} t_oneline;

static void *oneline_new(void) {
    t_oneline *x = (t_oneline *)pd_new(oneline_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *oneline_perform(t_int *w) {
	t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    t_int n = (t_int)(w[4]);
    int t;
    //unsigned char v;
    while (n--) {
    t = *in++;
    //v = v108; //Algorithm from oneline~.h
    //*out++ = (((v&0xff)^0x80)-128)/128; //putchar, conversion 2 unsigned char
    *out++ = v101;
    }
    return (w+5);
}

void oneline_dsp(t_oneline *x, t_signal **sp) {
    dsp_add(oneline_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void oneline_tilde_setup(void) {
    oneline_class = class_new(gensym("oneline~"), (t_newmethod)oneline_new, 0,
    	sizeof(t_oneline), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(oneline_class, t_oneline, x_f);
    class_addmethod(oneline_class, (t_method)oneline_dsp, gensym("dsp"), 0);
    post("oneline~ v0.1 maxus germanus 2011");
}