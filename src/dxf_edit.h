#ifndef _DXF_EDIT_LIB
#define _DXF_EDIT_LIB

#include "dxf.h"
#include "list.h"

int dxf_edit_move2 (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_move (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z);

#endif