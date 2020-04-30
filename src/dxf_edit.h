#ifndef _DXF_EDIT_LIB
#define _DXF_EDIT_LIB

#include "dxf.h"
#include "list.h"
#include "graph.h"

enum Explode_mode{
	EXPL_DEFAULT = 0,
	EXPL_VALUE = 1,
	EXPL_TAG = 2,
};

int dxf_edit_move2 (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_move (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z);

int dxf_edit_rot (dxf_node * obj, double ang);

int dxf_edit_mirror (dxf_node * obj, double x0, double y0, double x1, double y1);

int mtext_change_text (dxf_node *obj, char *text, int len, int pool);

list_node * dxf_edit_expl_ins(dxf_drawing *drawing, dxf_node * ins_ent, int mode);

#endif