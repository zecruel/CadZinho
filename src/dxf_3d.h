#ifndef _CZ_DXF_3D_LIB
#define _CZ_DXF_3D_LIB

#include "script.h"
#include "gui_script.h"

int dxf_3d_init ();
dxf_node * dxf_new_mesh  (char *chunk, int color, char *layer, int pool);

#endif