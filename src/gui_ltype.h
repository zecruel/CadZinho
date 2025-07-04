#ifndef _CZ_GUI_LTYP_LIB
#define _CZ_GUI_LTYP_LIB

#include "gui.h"
#include "dxf_create.h"

int ltyp_mng (gui_obj *gui);

int ltype_rename(dxf_drawing *drawing, int idx, char *name);
int ltype_use(dxf_drawing *drawing);
int ltype_prop(gui_obj *gui);

#endif