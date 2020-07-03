#ifndef _DXF_ENT_LIB
#define _DXF_ENT_LIB

#include "dxf.h"
#include "dxf_math.h"
#include <math.h>

int dxf_lwpline_get_pt(dxf_node * obj, dxf_node ** next,
double *pt1_x, double *pt1_y, double *pt1_z, double *bulge);

int dxf_pline_get_pt(dxf_node * obj, dxf_node ** next,
double *pt1_x, double *pt1_y, double *pt1_z, double *bulge);

int dxf_layer_get(dxf_drawing *drawing, dxf_node * ent);

int dxf_ltype_get(dxf_drawing *drawing, dxf_node * ent);



#endif