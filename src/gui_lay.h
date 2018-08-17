#ifndef _CZ_GUI_LAY_LIB
#define _CZ_GUI_LAY_LIB

#include "gui.h"
#include "dxf_create.h"

int lay_mng (gui_obj *gui);
int cmp_layer_name(const void * a, const void * b);
int cmp_layer_ltype(const void * a, const void * b);
int cmp_layer_color(const void * a, const void * b);
int cmp_layer_lw(const void * a, const void * b);
int cmp_layer_use(const void * a, const void * b);
int cmp_layer_off(const void * a, const void * b);
int cmp_layer_freeze(const void * a, const void * b);
int cmp_layer_lock(const void * a, const void * b);
int layer_rename(dxf_drawing *drawing, int idx, char *name);
int layer_use(dxf_drawing *drawing);
int layer_prop(gui_obj *gui);

#endif