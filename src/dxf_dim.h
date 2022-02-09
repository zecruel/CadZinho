#ifndef _DXF_DIM_LIB
#define _DXF_DIM_LIB

#include "dxf.h"
#include "list.h"
#include "graph.h"
#include "dxf_create.h"
#include "dxf_graph.h"
#include "dxf_edit.h"

/* functions */

list_node * dxf_dim_make(dxf_drawing *drawing, dxf_node * ent);

list_node * dxf_dim_linear_make(dxf_drawing *drawing, dxf_node * ent);

list_node * dxf_dim_angular_make(dxf_drawing *drawing, dxf_node * ent);

list_node * dxf_dim_radial_make(dxf_drawing *drawing, dxf_node * ent);

list_node * dxf_dim_ordinate_make(dxf_drawing *drawing, dxf_node * ent);
	
int dxf_find_last_dim (dxf_drawing *drawing);

int dxf_dim_get_blk (dxf_drawing *drawing, dxf_node * ent, dxf_node **blk, dxf_node **blk_rec);

int dxf_dim_rewrite (dxf_drawing *drawing, dxf_node *ent, dxf_node **blk, dxf_node **blk_rec, dxf_node **blk_old, dxf_node **blk_rec_old);

int dxf_dim_get_sty(dxf_drawing *drawing, dxf_dimsty *dim_sty);

int dxf_dim_update_sty(dxf_drawing *drawing, dxf_dimsty *dim_sty);

int dxf_dimsty_use(dxf_drawing *drawing);

#endif