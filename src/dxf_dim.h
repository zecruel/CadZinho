#ifndef _DXF_DIM_LIB
#define _DXF_DIM_LIB

#include "dxf.h"
#include "list.h"
#include "graph.h"
#include "dxf_create.h"
#include "dxf_graph.h"
#include "dxf_edit.h"

/* functions */
list_node * dxf_dim_linear_make(dxf_drawing *drawing, dxf_node * ent, double scale, 
	double an_scale, int an_format, 
	int term_type,
	int tol_type, double tol_up, double tol_low);
	
int dxf_find_last_dim (dxf_drawing *drawing);

#endif