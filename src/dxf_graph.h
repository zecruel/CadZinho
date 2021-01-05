#ifndef _DXF_PARSE_LIB
#define _DXF_PARSE_LIB

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"
#include "dxf_image.h"

/* for insert objects */
struct ins_save{
	dxf_node * ins_ent, *prev;
	double ofs_x, ofs_y, ofs_z;
	double rot, scale_x, scale_y, scale_z;
	int color, ltype, lw;
	int start_idx, end_idx;
	double normal[3];
	double elev;
};

list_node * dxf_graph_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx);

int dxf_ents_parse(dxf_drawing *drawing);

list_node * dxf_ents_parse2(dxf_drawing *drawing, int p_space, int pool_idx);

//int dxf_ents_draw(dxf_drawing *drawing, bmp_img * img, double ofs_x, double ofs_y, double scale);
int dxf_ents_draw(dxf_drawing *drawing, bmp_img * img, struct draw_param param);

int dxf_ents_ext(dxf_drawing *drawing, double * min_x, double * min_y, double * max_x, double * max_y);

list_node * dxf_list_parse(dxf_drawing *drawing, list_node *list, int p_space, int pool_idx);

dxf_node * dxf_ents_isect(dxf_drawing *drawing, double rect_pt1[2], double rect_pt2[2]);

int dxf_ents_isect2(list_node *list, dxf_drawing *drawing, double rect_pt1[2], double rect_pt2[2]);

int dxf_ents_in_rect(list_node *list, dxf_drawing *drawing, double rect_pt1[2], double rect_pt2[2]);

int dxf_list_draw(list_node *list, bmp_img * img, struct draw_param param);

graph_obj * dxf_lwpline_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx);

graph_obj * dxf_image_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx);

int change_ltype (dxf_drawing *drawing, graph_obj * graph, int ltype_idx, double scale);

int change_ltype2 (dxf_drawing *drawing, graph_obj * graph, dxf_ltype ltype, double scale);

#endif