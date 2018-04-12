// maxus germanus 2011

#include "m_pd.h"
#include "0~.h"

static t_class *0_class;

typedef struct _0 {
    t_object x_obj;
    t_float x_f;
} t_0;

static void *0_new(void) {
    t_0 *x = (t_0 *)pd_new(0_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *0_perform(t_int *w) {
	t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    int t;
    while (n--) {
    t = *in++;
    *out++ = v00;
    }
    return (w+5);
}

void 0_dsp(t_0 *x, t_signal **sp) {
    dsp_add(0_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void 0_tilde_setup(void) {
    0_class = class_new(gensym("0~"), (t_newmethod)0_new, 0,
    	sizeof(t_0), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(0_class, t_0, x_f);
    class_addmethod(0_class, (t_method)0_dsp, gensym("dsp"), 0);
    post("0~ v0.1 maxus germanus 2011");
}