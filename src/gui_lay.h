#ifndef _CZ_GUI_LAY_LIB
#define _CZ_GUI_LAY_LIB

#include "gui.h"
#include "dxf_create.h"

int lay_mng (gui_obj *gui);

int layer_rename(dxf_drawing *drawing, int idx, char *name);
int layer_use(dxf_drawing *drawing);
int layer_prop(gui_obj *gui);

#endif