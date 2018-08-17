#include "dxf_graph.h"
#include "list.h"
#include "dxf_attract.h"

//#include "dxf_colors.h"
extern bmp_color dxf_colors[];
#include <string.h>

int dxf_hatch_parse(list_node *list_ret, dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx);

int dxf_ent_get_color(dxf_drawing *drawing, dxf_node * ent, int ins_color){
	int color = 7;
	if((ent) && (drawing)){
		color = 256; /*default color entity color */
		dxf_node *color_obj, *layer_obj;
		
		if (color_obj = dxf_find_attr2(ent, 62)){
			color = color_obj->value.i_data;
		}
		
		if (abs(color) >= 256){ /* color is by layer */
			if (layer_obj = dxf_find_attr2(ent, 8)){
				char layer[DXF_MAX_CHARS];
				strncpy(layer, layer_obj->value.s_data, DXF_MAX_CHARS);
				str_upp(layer);
				
				/* find the layer index */
				int lay_idx = dxf_lay_idx(drawing, layer);
				color = drawing->layers[lay_idx].color;
			}
			else return 7; /* fail to find layer color*/
		}
		else if (color == 0){/* color is by block */
			color = ins_color;
		}
		if ((color == 0) || (abs(color) >= 256)) return 7; /* invalid  color*/
	}
	return color;
}

int dxf_ent_get_ltype(dxf_drawing *drawing, dxf_node * ent, int ins_ltype){
	int ltype_default = dxf_ltype_idx(drawing, "Continuous");
	int ltype = ltype_default, by_layer = 0;
	if((ent) && (drawing)){
		dxf_node *ltype_obj, *layer_obj;
		char ltype_name[DXF_MAX_CHARS], *new_name;
		
		if (ltype_obj = dxf_find_attr2(ent, 6)){
			strncpy(ltype_name, ltype_obj->value.s_data, DXF_MAX_CHARS);
			
			/* remove trailing spaces */
			new_name = trimwhitespace(ltype_name);
			/* change to upper case */
			str_upp(new_name);
			
			if (strcmp(new_name, "BYBLOCK") == 0){
				ltype = ins_ltype;
			}
			else if (strcmp(new_name, "BYLAYER") == 0){
				by_layer = 1;
			}
			else{
				ltype = dxf_ltype_idx(drawing, new_name);
			}
			
		} else by_layer = 1;
		
		if (by_layer){ /* ltype is by layer */
			if (layer_obj = dxf_find_attr2(ent, 8)){
				char layer[DXF_MAX_CHARS];
				strncpy(layer, layer_obj->value.s_data, DXF_MAX_CHARS);
				str_upp(layer);
				
				/* find the layer index */
				int lay_idx = dxf_lay_idx(drawing, layer);
				ltype = dxf_ltype_idx(drawing, drawing->layers[lay_idx].ltype);
			}
			else ltype = ltype_default; /* fail to find layer ltype*/
		}
		if ((ltype == 0) || (ltype >= drawing->num_ltypes)) 
			ltype = ltype_default;; /* invalid  ltype*/
	}
	return ltype;
}

int change_ltype (dxf_drawing *drawing, graph_obj * graph, int ltype_idx){
	/* change the graph line pattern */
	if((graph) && (drawing)){
		int i;
		graph->patt_size = drawing->ltypes[ltype_idx].size;
		for (i = 0; i < drawing->ltypes[ltype_idx].size; i++){
			graph->pattern[i] = drawing->ltypes[ltype_idx].pat[i];
		}
		return 1;
	}
	return 0;
}

int dxf_ent_get_lw(dxf_drawing *drawing, dxf_node * ent, int ins_lw){
	int lw = 0, by_layer = 0;
	if((ent) && (drawing)){
		dxf_node *lw_obj, *layer_obj;
		
		if (lw_obj = dxf_find_attr2(ent, 370)){
			lw = lw_obj->value.i_data;
		} else by_layer = 1;
		
		if ((lw == -1) || (by_layer == 1)){ /* line weight is by layer */
			if (layer_obj = dxf_find_attr2(ent, 8)){
				char layer[DXF_MAX_CHARS];
				strncpy(layer, layer_obj->value.s_data, DXF_MAX_CHARS);
				str_upp(layer);
				
				/* find the layer index */
				int lay_idx = dxf_lay_idx(drawing, layer);
				lw = drawing->layers[lay_idx].line_w;
			}
			else return 0; /* fail to find layer line weigth*/
		}
		else if (lw == -2){/* line weight is by block */
			lw = ins_lw;
		}
		else if (lw == -3){/* line weight is default */
			lw = 0; /*TODO*/
		}
		if ((lw < 0) || (lw > 211)){/* invalid line weight */
			lw = 0;
		}
	}
	return lw;
}

graph_obj * dxf_line_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		
		/*flags*/
		int pt1 = 0, pt2 = 0, paper = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 21:
						pt2_y = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 31:
						pt2_z = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 67:
						paper = current->value.i_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			graph_obj *curr_graph = graph_new(pool_idx);
			
			if (curr_graph){
				line_add(curr_graph, pt1_x, pt1_y, pt1_z, pt2_x, pt2_y, pt2_z);
			}
			
			return curr_graph;
		}
	}
	return NULL;
}

graph_obj * dxf_circle_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double radius;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		/*flags*/
		int pt1 = 0, paper = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 40:
						radius = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			graph_obj *curr_graph = graph_new(pool_idx);
			if (curr_graph){
				
				/* add the graph */
				graph_arc(curr_graph, pt1_x, pt1_y, pt1_z, radius, 0.0, 0.0, 1);
				
				/* convert OCS to WCS */
				normal[0] = extru_x;
				normal[1] = extru_y;
				normal[2] = extru_z;
				graph_mod_axis(curr_graph, normal, elev);
			}
			return curr_graph;
		}
	}
	return NULL;
}

graph_obj * dxf_arc_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double radius;
		double start_ang = 0.0, end_ang = 0.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		/*flags*/
		int pt1 = 0, paper = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 40:
						radius = current->value.d_data;
						break;
					case 50:
						start_ang = current->value.d_data;
						break;
					case 51:
						end_ang = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			graph_obj *curr_graph = graph_new(pool_idx);
			if (curr_graph){
				
				/* add the graph */
				graph_arc(curr_graph, pt1_x, pt1_y, pt1_z, radius, start_ang, end_ang, 1);
				
				/* convert OCS to WCS */
				normal[0] = extru_x;
				normal[1] = extru_y;
				normal[2] = extru_z;
				graph_mod_axis(curr_graph, normal, elev);
			}
			return curr_graph;
		}
	}
	return NULL;
}

void dxf_ellipse_mod_axis(graph_obj * master, double x_axis[3], double z_axis[3]){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double y_axis[3], point[3], x_col[3], y_col[3];
			
			
			double x0, y0, x1, y1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			
			master->ext_ini = 0;
			
			unit_vector(z_axis);
			
			cross_product(z_axis, x_axis, y_axis);
			unit_vector(y_axis);
			
			unit_vector(x_axis);
			
			x_col[0] = x_axis[0];
			x_col[1] = y_axis[0];
			x_col[2] = z_axis[0];
			
			y_col[0] = x_axis[1];
			y_col[1] = y_axis[1];
			y_col[2] = z_axis[1];
			
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				point[0] = current->x0;
				point[1] = current->y0;
				point[2] = current->z0;
				x0 = dot_product(point, x_col);
				y0 = dot_product(point, y_col);
				
				point[0] = current->x1;
				point[1] = current->y1;
				point[2] = current->z1;
				x1 = dot_product(point, x_col);
				y1 = dot_product(point, y_col);
				
				
				/* update the graph */
				current->x0 = x0;
				current->y0 = y0;
				current->x1 = x1;
				current->y1 = y1;
				
				/*update the extent of graph */
				/* sort the coordinates of entire line*/
				min_x = (x0 < x1) ? x0 : x1;
				min_y = (y0 < y1) ? y0 : y1;
				max_x = (x0 > x1) ? x0 : x1;
				max_y = (y0 > y1) ? y0 : y1;
				if (master->ext_ini == 0){
					master->ext_ini = 1;
					master->ext_min_x = min_x;
					master->ext_min_y = min_y;
					master->ext_max_x = max_x;
					master->ext_max_y = max_y;
				}
				else{
					master->ext_min_x = (master->ext_min_x < min_x) ? master->ext_min_x : min_x;
					master->ext_min_y = (master->ext_min_y < min_y) ? master->ext_min_y : min_y;
					master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
					master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
	
}

graph_obj * dxf_ellipse_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double minor_ax;
		double start_ang = 0.0, end_ang = 0.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		/*flags*/
		int pt1 = 0, pt2 = 0, paper = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 21:
						pt2_y = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 31:
						pt2_z = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 40:
						minor_ax = current->value.d_data;
						break;
					case 41:
						start_ang = current->value.d_data;
						break;
					case 42:
						end_ang = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			graph_obj *curr_graph = graph_new(pool_idx);
			if (curr_graph){
				
				/* add the graph */
				//graph_ellipse(curr_graph, pt1_x, pt1_y, pt1_z, pt2_x, pt2_y, pt2_z, minor_ax, start_ang, end_ang);
				
				/* convert OCS to WCS */
				//normal[0] = extru_x;
				//normal[1] = extru_y;
				//normal[2] = extru_z;
				//graph_mod_axis(curr_graph, normal);
				double major_ax = sqrt(pow(pt2_x, 2) + pow(pt2_y, 2)) ;
				
				graph_ellipse2(curr_graph, major_ax, minor_ax, start_ang, end_ang);
				double x_axis[3] = {pt2_x, pt2_y, pt2_z};
				double z_axis[3] = {extru_x, extru_y, extru_z};
				dxf_ellipse_mod_axis(curr_graph, x_axis, z_axis);
				
				graph_modify(curr_graph, pt1_x, pt1_y, 1.0, 1.0, 0.0);
			}
			return curr_graph;
		}
	}
	return NULL;
}

graph_obj * dxf_pline_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL, *prev;
		graph_obj *curr_graph = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double start_w = 0, end_w = 0;
		double bulge = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		int pline_flag = 0;
		int first = 0, closed =0;
		double prev_x, prev_y, prev_z, last_x, last_y, last_z;
		double prev_bulge = 0;
		
		/*flags*/
		int pt1 = 0, init = 0, paper = 0, vert = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			prev = current;
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 40:
						start_w = current->value.d_data;
						break;
					case 41:
						end_w = current->value.d_data;
						break;
					case 42:
						bulge = current->value.d_data;
						break;
					case 70:
						pline_flag = current->value.i_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			else if (current->type == DXF_ENT){
				if((init == 0) &&
				(((p_space == 0) && (paper == 0)) || 
				((p_space != 0) && (paper != 0)))){
					init = 1;
					curr_graph = graph_new(pool_idx);
					if (curr_graph){
						
						/*change tickness */
						curr_graph->tick = start_w;
						
						pt1_x = 0; pt1_y = 0; pt1_z = 0;
						bulge =0;
						
						if (pline_flag & 1){
							closed = 1;
						}
						else {
							closed = 0;
						}
						
						/* for convertion OCS to WCS */
						normal[0] = extru_x;
						normal[1] = extru_y;
						normal[2] = extru_z;
					}
				}
				
				if ((strcmp(current->obj.name, "VERTEX") == 0) && (current->obj.content)){
					current = current->obj.content->next;
					vert = 1;
					//printf("vertice\n");
					continue;
				}
			}
			current = current->next; /* go to the next in the list */
		
		
			if ((current == NULL) && (vert)){
				vert = 0;
				if((first != 0) && (curr_graph != NULL)){
					//printf("(%0.2f, %0.2f)-(%0.2f, %0.2f)\n", prev_x, prev_y, pt1_x, pt1_y);
					if (prev_bulge == 0){
						line_add(curr_graph, prev_x, prev_y, prev_z, pt1_x, pt1_y, pt1_z);
					}
					else{
						graph_arc_bulge(curr_graph, prev_x, prev_y, prev_z, pt1_x, pt1_y, pt1_z, prev_bulge);
					}
				}
				else if(first == 0){
					first = 1;
					
					//printf("primeiro vertice\n");
					last_x = pt1_x;
					last_y = pt1_y;
					last_z = pt1_z;
				}
				prev_x = pt1_x;
				prev_y = pt1_y;
				prev_z = pt1_z;
				prev_bulge = bulge;
				
				pt1_x = 0; pt1_y = 0; pt1_z = 0;
				bulge =0;
			}
		
			while (current == NULL){
				
				prev = prev->master;
				if (prev){ /* up in structure */
					
					/* ====== close complex entities ============== */
					if (prev == ent){ /* back on polyline ent */
						if((closed != 0) && (curr_graph != NULL)){
							if (prev_bulge == 0){
								line_add(curr_graph, prev_x, prev_y, prev_z, last_x, last_y, last_z);
							}
							else{
								graph_arc_bulge(curr_graph, prev_x, prev_y, prev_z, last_x, last_y, last_z, prev_bulge);
							}
						}
						//printf("fim\n");
						
						/* convert OCS to WCS */
						graph_mod_axis(curr_graph, normal, elev);
						
						break;
					}
					/* try to continue on previous point in structure */
					current = prev->next;
				}
			}
		}
		
		return curr_graph;
	}
	return NULL;
}

graph_obj * dxf_lwpline_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL, *prev;
		graph_obj *curr_graph = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double start_w = 0, end_w = 0, fix_w = 0;
		double bulge = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		int pline_flag = 0;
		int first = 0, closed =0;
		double prev_x, prev_y, prev_z, last_x, last_y, last_z, curr_x;
		double prev_bulge = 0;
		double elev = 0.0;
		
		/*flags*/
		int pt1 = 0, init = 0, paper = 0;
		
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			prev = current;
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 40:
						start_w = current->value.d_data;
						break;
					case 41:
						end_w = current->value.d_data;
						break;
					case 42:
						bulge = current->value.d_data;
						break;
					case 43:
						fix_w = current->value.d_data;
						break;
					case 70:
						pline_flag = current->value.i_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			if (pt1){
				pt1 = 0;
				
				if((init != 0) &&(first == 0)){
					first = 1;
					
					//printf("primeiro vertice\n");
					last_x = curr_x;
					last_y = pt1_y;
					last_z = pt1_z;
					prev_x = curr_x;
					prev_y = pt1_y;
					prev_z = pt1_z;
				}
				else if((init == 0) &&
				(((p_space == 0) && (paper == 0)) || 
				((p_space != 0) && (paper != 0)))){
					init = 1;
					curr_graph = graph_new(pool_idx);
					if (curr_graph){
						
						/*change tickness */
						curr_graph->tick = fix_w;
							
						if (pline_flag & 1){
							closed = 1;
						}
						else {
							closed = 0;
						}
					}
				}
				else if((first != 0) && (curr_graph != NULL)){
					//printf("(%0.2f, %0.2f)-(%0.2f, %0.2f)\n", prev_x, prev_y, curr_x, pt1_y);
					if (prev_bulge == 0){
						line_add(curr_graph, prev_x, prev_y, prev_z, curr_x, pt1_y, pt1_z);
					}
					else{
						graph_arc_bulge(curr_graph, prev_x, prev_y, prev_z, curr_x, pt1_y, pt1_z, prev_bulge);
						//bulge =0;
					}
					prev_x = curr_x;
					prev_y = pt1_y;
					prev_z = pt1_z;
				}
				
				prev_bulge = bulge;
				bulge = 0;
				
				curr_x = pt1_x;
			}
			current = current->next; /* go to the next in the list */
		}
		
		/* last vertex */
		if((first != 0) && (curr_graph != NULL)){
			//printf("(%0.2f, %0.2f)-(%0.2f, %0.2f)\n", prev_x, prev_y, curr_x, pt1_y);
			if (prev_bulge == 0){
				line_add(curr_graph, prev_x, prev_y, prev_z, curr_x, pt1_y, pt1_z);
			}
			else{
				graph_arc_bulge(curr_graph, prev_x, prev_y, prev_z, curr_x, pt1_y, pt1_z, prev_bulge);
				//bulge =0;
			}
			prev_x = curr_x;
			prev_y = pt1_y;
		}
		
		if((closed != 0) && (curr_graph != NULL)){
			if (bulge == 0){
				line_add(curr_graph, prev_x, prev_y, prev_z, last_x, last_y, last_z);
			}
			else{
				graph_arc_bulge(curr_graph, prev_x, prev_y, prev_z, last_x, last_y, last_z, bulge);
			}
		}
		
		/* convert OCS to WCS */
		normal[0] = extru_x;
		normal[1] = extru_y;
		normal[2] = extru_z;
		graph_mod_axis(curr_graph, normal, elev);
		
		return curr_graph;
	}
	return NULL;
}


graph_obj * dxf_spline_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL, *prev;
		graph_obj *curr_graph = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		
		int pline_flag = 0, closed = 0, count, i;
		double prev_x, prev_y, prev_z, curr_x;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
				
		/*flags*/
		int pt1 = 0, init = 0, paper = 0;
		
		int num_cpts, order, num_ret, num_knots;
		double weight = 1.0;
		double ctrl_pts[3 * MAX_SPLINE_PTS], ret[3 * MAX_SPLINE_PTS];
		double weights[MAX_SPLINE_PTS], knots[MAX_SPLINE_PTS];
		int knot_count = 1;

		count =0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			prev = current;
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
	
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 40:
						knots[knot_count] = current->value.d_data;
						knot_count++;
						break;
					case 41:
						weight = current->value.d_data;
						break;
					case 70:
						pline_flag = current->value.i_data;
						break;
					case 71:
						order = current->value.i_data;
						break;
					case 72:
						num_knots = current->value.i_data;
						break;
					case 73:
						num_cpts = current->value.i_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			if (pt1){
				pt1 = 0;
				
				if ((init != 0) && (count < MAX_SPLINE_PTS)){
					ctrl_pts[count*3+1] = curr_x;
					ctrl_pts[count*3+2] = pt1_y;
					ctrl_pts[count*3+3] = pt1_z;
					weights[count+1] = weight;
					count++;
				}
				else if((init == 0) &&
				(((p_space == 0) && (paper == 0)) || 
				((p_space != 0) && (paper != 0)))){
					init = 1;
				}
				
				curr_x = pt1_x;
			}
			current = current->next; /* go to the next in the list */
		}
		
		/* last vertex */
		if ((init != 0) && (count < MAX_SPLINE_PTS)){
			ctrl_pts[count*3+1] = curr_x;
			ctrl_pts[count*3+2] = pt1_y;
			ctrl_pts[count*3+3] = pt1_z;
			weights[count+1] = weight;
			count++;
		}
		
		curr_graph = graph_new(pool_idx);
		if ((curr_graph)&&((count + order)*5 < MAX_SPLINE_PTS)){
			
			if (pline_flag & 1){
				closed = 1;
			}
			else {
				closed = 0;
			}
		
			num_ret = (num_cpts + order)*5; /* num pts on curve */
			
			for(i = 1; i <= 3*num_ret; i++){
				ret[i] = 0.0;
			}
			
			rbspline(num_cpts, order+1, num_ret, ctrl_pts, weights, ret);
			
			prev_x = ret[1];
			prev_y = ret[2];
			prev_z = ret[3];
			
			for(i =4 ; i <= 3*num_ret; i = i+3){
				line_add(curr_graph, prev_x, prev_y, prev_z, ret[i], ret[i+1], ret[i+2]);
				prev_x = ret[i];
				prev_y = ret[i+1];
				prev_z = ret[i+2];
				/*printf(" %f %f %f \n",ret[i],ret[i+1],ret[i+2]);*/
			}
		}
		return curr_graph;
	}
	return NULL;
}

graph_obj * dxf_text_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		graph_obj *curr_graph = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		shape *shx_font = NULL;
		
		double t_size = 0, t_rot = 0;
		
		int t_alin_v = 0, t_alin_h = 0;
		
		double fnt_size, fnt_above, fnt_below, txt_size;
		double t_pos_x, t_pos_y, t_center_x = 0, t_center_y = 0, t_base_x = 0, t_base_y = 0;
		double t_scale_x = 1, t_scale_y = 1, txt_w, txt_h;
		
		char text[DXF_MAX_CHARS], t_style[DXF_MAX_CHARS];
		char tmp_str[DXF_MAX_CHARS];
		char *pos_st, *pos_curr, *pos_tmp, special;
		
		int fnt_idx, i, paper = 0;
		
		/*flags*/
		int pt1 = 0, pt2 = 0;
		int under_l, over_l;
		
		
		/* clear the strings */
		text[0] = 0;
		t_style[0] = 0;
		tmp_str[0] = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 1:
						strcpy(text, current->value.s_data);
						break;
					case 7:
						strcpy(t_style, current->value.s_data);
						break;
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 21:
						pt2_y = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 31:
						pt2_z = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 40:
						t_size = current->value.d_data;
						break;
					case 41:
						t_scale_x = current->value.d_data;
						break;
					case 50:
						t_rot = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 72:
						t_alin_h = current->value.i_data;
						break;
					case 73:
						t_alin_v = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			
			/* find the font index and font*/
			fnt_idx = dxf_font_idx(drawing, t_style);
			shx_font = drawing->text_fonts[fnt_idx].shx_font;
			
			if(shx_font == NULL){ /* if font not loaded*/
				/* use the deafault font*/
				shx_font = drawing->text_fonts[0].shx_font;
			}
			
			/* find the dimentions of SHX font */
			if(shx_font){ /* if the font exists */
				if(shx_font->next){ /* the font descriptor is stored in first iten of list */
					if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
						fnt_above = shx_font->next->cmds[0]; /* size above the base line of text */
						fnt_below = shx_font->next->cmds[1]; /* size below the base line of text */
						if((fnt_above + fnt_below) > 0){
							fnt_size = fnt_above + fnt_below;
						}
					}
				}
			}
			
			/* find and replace special symbols in the text*/
			under_l = 0; /* under line flag*/
			over_l = 0; /* over line flag*/
			pos_curr = strstr(text, "%%");
			pos_st = text;
			pos_tmp = tmp_str;
			while (pos_curr){
				/* copy the part of text until the control string */
				strncpy(pos_tmp, pos_st, pos_curr - pos_st);
				/*control string is stripped in new string */
				pos_tmp += pos_curr - pos_st;
				/*get the control character */
				special = *(pos_curr + 2);
				/* verify the action to do */
				switch (special){
					/* put the  diameter simbol (unicode D8 Hex) in text*/
					case 'c':
						pos_tmp += wctomb(pos_tmp, L'\xd8');
						break;
					case 'C':
						pos_tmp += wctomb(pos_tmp, L'\xd8');
						break;
					/* put the degrees simbol in text*/
					case 'd':
						pos_tmp += wctomb(pos_tmp, L'\xb0');
						break;
					case 'D':
						pos_tmp += wctomb(pos_tmp, L'\xb0');
						break;
					/* put the plus/minus tolerance simbol in text*/
					case 'p':
						pos_tmp += wctomb(pos_tmp, L'\xb1');
						break;
					case 'P':
						pos_tmp += wctomb(pos_tmp, L'\xb1');
						break;
					/* under line */
					case 'u':
						under_l = 1;
						break;
					case 'U':
						under_l = 1;
						break;
					/* over line */
					case 'o':
						over_l = 1;
						break;
					case 'O':
						over_l = 1;
						break;
				}
				/*try to find new  control sequences in the rest of text*/
				pos_curr += 3;
				pos_st = pos_curr;
				pos_curr = strstr(pos_curr, "%%");
			}
			/* copy the rest of text after the last control string */
			strcpy(pos_tmp, pos_st);
			//printf("%s\n", tmp_str);
			
			curr_graph = shx_font_parse(shx_font, pool_idx, tmp_str, NULL);
			
			
			if (curr_graph){
				
				/* find the dimentions of text */
				txt_size = t_size/fnt_above;
				txt_w = fabs(curr_graph->ext_max_x - curr_graph->ext_min_x);
				txt_h = fabs(curr_graph->ext_max_y - curr_graph->ext_min_y);
				
				if (under_l){
					/* add the under line */
					line_add(curr_graph, 
						curr_graph->ext_min_x,
						(double)fnt_size * -0.1,
						pt1_z,
						curr_graph->ext_max_x, 
						(double)fnt_size * -0.1,
						pt1_z);
				}
				if (over_l){
					/* add the over line */
					line_add(curr_graph, 
						curr_graph->ext_min_x,
						(double)fnt_size * 1.1,
						pt1_z,
						curr_graph->ext_max_x, 
						(double)fnt_size * 1.1,
						pt1_z);
				}
				
				t_base_x =  pt2_x;
				t_base_y =  pt2_y;
				
				if ((t_alin_v == 0) && (t_alin_h == 0)){
					t_base_x =  pt1_x;
					t_base_y =  pt1_y;
				}
				
				/* find the insert point of text, in function of its aling */
				else if(t_alin_h < 3){
					t_center_x = (double)t_alin_h * (t_scale_x*txt_w * txt_size/2);
					//t_base_x =  (double)t_alin_h * (pt2_x - pt1_x)/2;
					//t_base_y =  (double)t_alin_h * (pt2_y - pt1_y)/2;
				}
				else{ 
					if(t_alin_h == 4){
						t_center_y = (fnt_above + fnt_below)* txt_size/2;
					}
					else{
						t_scale_x = sqrt(pow((pt2_x - pt1_x), 2) + pow((pt2_y - pt1_y), 2))/(txt_w * txt_size);
						t_base_x =  pt1_x + (pt2_x - pt1_x)/2;
						t_base_y =  pt1_y + (pt2_y - pt1_y)/2;
					}
					
					t_center_x = (t_scale_x*txt_w * txt_size/2);
					//rot = atan2((pt2_y - pt1_y),(pt2_x - pt1_x)) * 180/M_PI;
					
					//printf("alinhamento=%d\n", t_alin_h);
				}
				if(t_alin_v >0){
					if(t_alin_v != 1){
						t_center_y = (double)(t_alin_v - 1) * fnt_above * txt_size/2;
					}
					else{
						t_center_y = - fnt_below * txt_size;
					}
				}
				
				t_pos_x = t_base_x - t_center_x;
				t_pos_y = t_base_y - t_center_y;
				
				/* apply the scales, offsets and rotation to graphs */
				graph_modify(curr_graph, t_pos_x, t_pos_y, t_scale_x*txt_size, txt_size, 0.0);
				graph_rot(curr_graph, t_base_x, t_base_y, t_rot);
				
				/* convert OCS to WCS */
				normal[0] = extru_x;
				normal[1] = extru_y;
				normal[2] = extru_z;
				graph_mod_axis(curr_graph, normal, elev);
				
			}
			return curr_graph;
		}
	}
	return NULL;
}

graph_obj * dxf_attrib_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		graph_obj *curr_graph = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		shape *shx_font = NULL;
		
		double t_size = 0, t_rot = 0;
		
		int t_alin_v = 0, t_alin_h = 0;
		
		double fnt_size, fnt_above, fnt_below, txt_size;
		double t_pos_x, t_pos_y, t_center_x = 0, t_center_y = 0, t_base_x = 0, t_base_y = 0;
		double t_scale_x = 1, t_scale_y = 1, txt_w, txt_h;
		
		char text[DXF_MAX_CHARS], t_style[DXF_MAX_CHARS];
		char tmp_str[DXF_MAX_CHARS];
		char *pos_st, *pos_curr, *pos_tmp, special;
		
		int fnt_idx, i, paper = 0;
		
		/*flags*/
		int pt1 = 0, pt2 = 0;
		int under_l, over_l;
		
		
		/* clear the strings */
		text[0] = 0;
		t_style[0] = 0;
		tmp_str[0] = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 1:
						strcpy(text, current->value.s_data);
						break;
					case 7:
						strcpy(t_style, current->value.s_data);
						break;
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 21:
						pt2_y = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 31:
						pt2_z = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 40:
						t_size = current->value.d_data;
						break;
					case 41:
						t_scale_x = current->value.d_data;
						break;
					case 50:
						t_rot = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 72:
						t_alin_h = current->value.i_data;
						break;
					case 74:
						t_alin_v = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			
			/* find the font index and font*/
			fnt_idx = dxf_font_idx(drawing, t_style);
			shx_font = drawing->text_fonts[fnt_idx].shx_font;
			
			if(shx_font == NULL){ /* if font not loaded*/
				/* use the deafault font*/
				shx_font = drawing->text_fonts[0].shx_font;
			}
			
			/* find the dimentions of SHX font */
			if(shx_font){ /* if the font exists */
				if(shx_font->next){ /* the font descriptor is stored in first iten of list */
					if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
						fnt_above = shx_font->next->cmds[0]; /* size above the base line of text */
						fnt_below = shx_font->next->cmds[1]; /* size below the base line of text */
						if((fnt_above + fnt_below) > 0){
							fnt_size = fnt_above + fnt_below;
						}
					}
				}
			}
			
			/* find and replace special symbols in the text*/
			under_l = 0; /* under line flag*/
			over_l = 0; /* over line flag*/
			pos_curr = strstr(text, "%%");
			pos_st = text;
			pos_tmp = tmp_str;
			while (pos_curr){
				/* copy the part of text until the control string */
				strncpy(pos_tmp, pos_st, pos_curr - pos_st);
				/*control string is stripped in new string */
				pos_tmp += pos_curr - pos_st;
				/*get the control character */
				special = *(pos_curr + 2);
				/* verify the action to do */
				switch (special){
					/* put the  diameter simbol (unicode D8 Hex) in text*/
					case 'c':
						pos_tmp += wctomb(pos_tmp, L'\xd8');
						break;
					case 'C':
						pos_tmp += wctomb(pos_tmp, L'\xd8');
						break;
					/* put the degrees simbol in text*/
					case 'd':
						pos_tmp += wctomb(pos_tmp, L'\xb0');
						break;
					case 'D':
						pos_tmp += wctomb(pos_tmp, L'\xb0');
						break;
					/* put the plus/minus tolerance simbol in text*/
					case 'p':
						pos_tmp += wctomb(pos_tmp, L'\xb1');
						break;
					case 'P':
						pos_tmp += wctomb(pos_tmp, L'\xb1');
						break;
					/* under line */
					case 'u':
						under_l = 1;
						break;
					case 'U':
						under_l = 1;
						break;
					/* over line */
					case 'o':
						over_l = 1;
						break;
					case 'O':
						over_l = 1;
						break;
				}
				/*try to find new  control sequences in the rest of text*/
				pos_curr += 3;
				pos_st = pos_curr;
				pos_curr = strstr(pos_curr, "%%");
			}
			/* copy the rest of text after the last control string */
			strcpy(pos_tmp, pos_st);
			//printf("%s\n", tmp_str);
			
			curr_graph = shx_font_parse(shx_font, pool_idx, tmp_str, NULL);
			
			
			if (curr_graph){
				
				/* find the dimentions of text */
				txt_size = t_size/fnt_above;
				txt_w = fabs(curr_graph->ext_max_x - curr_graph->ext_min_x);
				txt_h = fabs(curr_graph->ext_max_y - curr_graph->ext_min_y);
				
				if (under_l){
					/* add the under line */
					line_add(curr_graph, 
						curr_graph->ext_min_x,
						(double)fnt_size * -0.1,
						pt1_z,
						curr_graph->ext_max_x, 
						(double)fnt_size * -0.1,
						pt1_z);
				}
				if (over_l){
					/* add the over line */
					line_add(curr_graph, 
						curr_graph->ext_min_x,
						(double)fnt_size * 1.1,
						pt1_z,
						curr_graph->ext_max_x, 
						(double)fnt_size * 1.1,
						pt1_z);
				}
				
				t_base_x =  pt2_x;
				t_base_y =  pt2_y;
				
				if ((t_alin_v == 0) && (t_alin_h == 0)){
					t_base_x =  pt1_x;
					t_base_y =  pt1_y;
				}
				
				/* find the insert point of text, in function of its aling */
				else if(t_alin_h < 3){
					t_center_x = (double)t_alin_h * (t_scale_x*txt_w * txt_size/2);
					//t_base_x =  (double)t_alin_h * (pt2_x - pt1_x)/2;
					//t_base_y =  (double)t_alin_h * (pt2_y - pt1_y)/2;
				}
				else{ 
					if(t_alin_h == 4){
						t_center_y = (fnt_above + fnt_below)* txt_size/2;
					}
					else{
						t_scale_x = sqrt(pow((pt2_x - pt1_x), 2) + pow((pt2_y - pt1_y), 2))/(txt_w * txt_size);
						t_base_x =  pt1_x + (pt2_x - pt1_x)/2;
						t_base_y =  pt1_y + (pt2_y - pt1_y)/2;
					}
					
					t_center_x = (t_scale_x*txt_w * txt_size/2);
					//rot = atan2((pt2_y - pt1_y),(pt2_x - pt1_x)) * 180/M_PI;
					
					//printf("alinhamento=%d\n", t_alin_h);
				}
				if(t_alin_v >0){
					if(t_alin_v != 1){
						t_center_y = (double)(t_alin_v - 1) * fnt_above * txt_size/2;
					}
					else{
						t_center_y = - fnt_below * txt_size;
					}
				}
				
				t_pos_x = t_base_x - t_center_x;
				t_pos_y = t_base_y - t_center_y;
				
				/* apply the scales, offsets and rotation to graphs */
				graph_modify(curr_graph, t_pos_x, t_pos_y, t_scale_x*txt_size, txt_size, 0.0);
				graph_rot(curr_graph, t_base_x, t_base_y, t_rot);
				
				/* convert OCS to WCS */
				normal[0] = extru_x;
				normal[1] = extru_y;
				normal[2] = extru_z;
				graph_mod_axis(curr_graph, normal, elev);
				
			}
			return curr_graph;
		}
	}
	return NULL;
}

graph_obj * dxf_solid_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	if(ent){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double pt3_x = 0, pt3_y = 0, pt3_z = 0;
		double pt4_x = 0, pt4_y = 0, pt4_z = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		
		int i, paper = 0;
		
		/*flags*/
		int pt1 = 0, pt2 = 0, pt3 = 0, pt4 = 0;
		
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						pt1_y = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 30:
						pt1_z = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 21:
						pt2_y = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 31:
						pt2_z = current->value.d_data;
						pt2 = 1; /* set flag */
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 12:
						pt3_x = current->value.d_data;
						pt3 = 1; /* set flag */
						break;
					case 22:
						pt3_y = current->value.d_data;
						pt3 = 1; /* set flag */
						break;
					case 32:
						pt3_z = current->value.d_data;
						pt3 = 1; /* set flag */
						break;
					case 13:
						pt4_x = current->value.d_data;
						pt4 = 1; /* set flag */
						break;
					case 23:
						pt4_y = current->value.d_data;
						pt4 = 1; /* set flag */
						break;
					case 33:
						pt4_z = current->value.d_data;
						pt4 = 1; /* set flag */
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			graph_obj *curr_graph = graph_new(pool_idx);
			if (curr_graph){
				/* mark as filled object */
				curr_graph->fill = 1;
				
				/* add the graph */
				//line_add(curr_graph, pt1_x, pt1_y, pt1_z, pt2_x, pt2_y, pt2_z);
				
				if (pt4){
					line_add(curr_graph, pt1_x, pt1_y, pt1_z, pt3_x, pt3_y, pt3_z);
					line_add(curr_graph, pt3_x, pt3_y, pt3_z, pt4_x, pt4_y, pt4_z);
					line_add(curr_graph, pt4_x, pt4_y, pt4_z, pt2_x, pt2_y, pt2_z);
					line_add(curr_graph, pt2_x, pt2_y, pt2_z, pt1_x, pt1_y, pt1_z);
				}
				else{
					line_add(curr_graph, pt2_x, pt2_y, pt2_z, pt3_x, pt3_y, pt3_z);
					line_add(curr_graph, pt3_x, pt3_y, pt3_z, pt1_x, pt1_y, pt1_z);
					line_add(curr_graph, pt1_x, pt1_y, pt1_z, pt2_x, pt2_y, pt2_z);
				}
				
				
				/* convert OCS to WCS */
				normal[0] = extru_x;
				normal[1] = extru_y;
				normal[2] = extru_z;
				graph_mod_axis(curr_graph, normal, elev);
			}
			return curr_graph;
		}
	}
	return NULL;
}

int proc_obj_graph(dxf_drawing *drawing, dxf_node * ent, graph_obj * graph, struct ins_save ins){
	if ((drawing) && (ent) && (graph)){
		dxf_node *layer_obj;
		if (layer_obj = dxf_find_attr2(ent, 8)){
			char layer[DXF_MAX_CHARS];
			strncpy(layer, layer_obj->value.s_data, DXF_MAX_CHARS);
			str_upp(layer);
			
			/* find the layer index */
			ent->obj.layer = dxf_lay_idx(drawing, layer);
		}
		
		graph->color = dxf_colors[dxf_ent_get_color(drawing, ent, ins.color)];
		
		int ltype = dxf_ent_get_ltype(drawing, ent, ins.ltype);
		change_ltype (drawing, graph, ltype);
		
		/*change tickness */
		int lw = dxf_ent_get_lw(drawing, ent, ins.lw);
		//graph->tick = 0;
		if (lw > 0){
			graph->tick = (double)lw * 0.07109;
			graph->thick_const = 1;
		}
		
	}
	
}

int dxf_obj_parse(list_node *list_ret, dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	/* this function is non recursive */
	
	
	dxf_node *current = NULL, *insert_ent = NULL, *blk = NULL , *prev;
	enum dxf_graph ent_type;
	int lay_idx, ltype_idx, mod_idx = 0;
	graph_obj * curr_graph = NULL;
	
	/* for attrib and attdef objects */
	/* initialize the attrib list */
	list_node * att_list = list_new(NULL, FRAME_LIFE);
	
	struct ins_save ins_stack[10];
	int ins_stack_pos = 0;
	
	struct ins_save ins_zero = {
		.ins_ent = ent, .prev = NULL,
		.ofs_x = 0.0, .ofs_y =0.0, .ofs_z =0.0,
		.rot = 0.0, .scale_x = 1.0 , .scale_y = 1.0, .scale_z = 1.0,
		.color = 7, .ltype = 0, .lw =0,
		.normal = {0.0, 0.0, 1.0},
		.elev = 0.0
	};
	
	ins_stack[0] = ins_zero;
	int ins_flag = 0, parse_attdef = 1;
	/* ---- */
	
	double pt1_x = 0, pt1_y = 0, pt1_z = 0;
	double t_rot = 0, rot = 0, tick = 0, elev = 0;
	double scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
	double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0;
	
	char handle[DXF_MAX_CHARS], l_type[DXF_MAX_CHARS], layer[DXF_MAX_CHARS];
	char name1[DXF_MAX_CHARS], name2[DXF_MAX_CHARS], comment[DXF_MAX_CHARS];
	
	int color = 256, paper = 0;
	
	/*flags*/
	int pt1 = 0;
	int i;
	int blk_flag = 0;
	
	if (list_ret){
		current = ent;
	}
	else{
		current = NULL;
	}
	ent_type = DXF_NONE;
	while (current){
		prev = current;
		/* ============================================================= */
		if (current->type == DXF_ENT){
			ent_type = DXF_NONE;
			if (strcmp(current->obj.name, "LINE") == 0){
				ent_type = DXF_LINE;
				curr_graph = dxf_line_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					
					/*curr_graph->color = dxf_colors[dxf_ent_get_color(drawing, current, ins_stack[ins_stack_pos].color)];
					
					int curr_ltype = dxf_ent_get_ltype(drawing, current, ins_stack[ins_stack_pos].ltype);
					change_ltype (drawing, curr_graph, curr_ltype);
					
					
					/*change tickness 
					int curr_lw = dxf_ent_get_lw(drawing, current, ins_stack[ins_stack_pos].lw);
					curr_graph->tick = 0;
					if (curr_lw > 0){
						curr_graph->tick = (double)curr_lw * 0.07109;
						curr_graph->thick_const = 1;
					}
					
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
			}
			else if (strcmp(current->obj.name, "TEXT") == 0){
				curr_graph = dxf_text_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				ent_type = DXF_TEXT;
				
				
			}
			else if (strcmp(current->obj.name, "CIRCLE") == 0){
				ent_type = DXF_CIRCLE;
				curr_graph = dxf_circle_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
			}
			else if (strcmp(current->obj.name, "ARC") == 0){
				ent_type = DXF_ARC;
				curr_graph = dxf_arc_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
			}
			else if (strcmp(current->obj.name, "POLYLINE") == 0){
				ent_type = DXF_POLYLINE;
				curr_graph = dxf_pline_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
			}
			else if (strcmp(current->obj.name, "HATCH") == 0){
				//ent_type = DXF_POLYLINE;
				list_node *hatch_items = list_new(NULL, FRAME_LIFE);
				
				int num_h_items= dxf_hatch_parse(hatch_items, drawing, current, p_space, pool_idx);
				
				if ((hatch_items != NULL) && (num_h_items)){
					list_node *curr_node = hatch_items->next;
					
					// starts the content sweep 
					while (curr_node != NULL){
						if (curr_node->data){
							curr_graph = (graph_obj *)curr_node->data;
							/* store the graph in the return vector */
							list_push(list_ret, list_new((void *)curr_graph, pool_idx));
							proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
							mod_idx++;
						}
						curr_node = curr_node->next;
					}
				}
				list_clear (hatch_items);
			}
			else if (strcmp(current->obj.name, "VERTEX") == 0){
				ent_type = DXF_VERTEX;
				
				
			}
			else if (strcmp(current->obj.name, "TRACE") == 0){
				ent_type = DXF_TRACE;
				
				
			}
			else if (strcmp(current->obj.name, "SOLID") == 0){
				ent_type = DXF_SOLID;
				curr_graph = dxf_solid_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
				
			}
			else if (strcmp(current->obj.name, "LWPOLYLINE") == 0){
				ent_type = DXF_LWPOLYLINE;
				curr_graph = dxf_lwpline_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
			}
			else if (strcmp(current->obj.name, "SPLINE") == 0){
				//ent_type = DXF_LWPOLYLINE;
				curr_graph = dxf_spline_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
			}
			else if (strcmp(current->obj.name, "ATTRIB") == 0){
				ent_type = DXF_ATTRIB;
				curr_graph = dxf_attrib_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos+1]);
					
					/* store the graph in a temporary list.
					this approach avoid to modify the attributes inside INSERTs*/
					list_node * att_el = list_new(curr_graph, FRAME_LIFE);
					if (att_el){
						list_push(att_list, att_el);
					}
				}
			}
			else if ((strcmp(current->obj.name, "ATTDEF") == 0) && parse_attdef){
				ent_type = DXF_ATTRIB;
				curr_graph = dxf_attrib_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos+1]);
					
					/* store the graph in a temporary list.
					this approach avoid to modify the attributes inside INSERTs*/
					list_node * att_el = list_new(curr_graph, FRAME_LIFE);
					if (att_el){
						list_push(att_list, att_el);
					}
				}
			}
			else if (strcmp(current->obj.name, "POINT") == 0){
				ent_type = DXF_POINT;
				
				
			}
			
			else if (strcmp(current->obj.name, "SHAPE") == 0){
				ent_type = DXF_SHAPE;
				
				
			}
			else if (strcmp(current->obj.name, "VIEWPORT") == 0){
				ent_type = DXF_VIEWPORT;
				
				
			}
			else if (strcmp(current->obj.name, "3DFACE") == 0){
				ent_type = DXF_3DFACE;
				
				
			}
			else if (strcmp(current->obj.name, "ELLIPSE") == 0){
				ent_type = DXF_ELLIPSE;
				curr_graph = dxf_ellipse_parse(drawing, current, p_space, pool_idx);
				if (curr_graph){
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr_graph, pool_idx));
					proc_obj_graph(drawing, current, curr_graph, ins_stack[ins_stack_pos]);
					mod_idx++;
				}
				
			}
			else if (strcmp(current->obj.name, "MTEXT") == 0){
				ent_type = DXF_MTEXT;
				
				
			}
			
			/* ============================================================= */
			/* complex entities */
			else if (strcmp(current->obj.name, "INSERT") == 0){
				ent_type = DXF_INSERT;
				insert_ent = current;
				ins_flag = 1;
				parse_attdef = 0;
				
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content->next;
					prev = current;
					continue;
				}
			}
			else if (strcmp(current->obj.name, "BLOCK") == 0){
				ent_type = DXF_BLK;
				blk_flag = 1;
				
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content->next;
					continue;
				}
			}
			else if (strcmp(current->obj.name, "DIMENSION") == 0){
				ent_type = DXF_DIMENSION;
				/* a dimension will draw as a insert entity */
				insert_ent = current;
				ins_flag = 1;
				parse_attdef = 0;
				
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content->next;
					prev = current;
					continue;
				}
			}
		}
		
		/* ============================================================= */
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			//printf("%d\n", current->value.group);
			switch (current->value.group){
				case 2:
					strcpy(name1, current->value.s_data);
					break;
				case 3:
					strcpy(name2, current->value.s_data);
					break;
				case 5:
					strcpy(handle, current->value.s_data);
					break;
				case 6:
					strcpy(l_type, current->value.s_data);
					break;
				case 8:
					strcpy(layer, current->value.s_data);
					break;
				case 10:
					pt1_x = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 20:
					pt1_y = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 30:
					pt1_z = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 38:
					elev = current->value.d_data;
					break;
				case 39:
					tick = current->value.d_data;
					break;
				case 41:
					scale_x = current->value.d_data;
					break;
				case 42:
					scale_y = current->value.d_data;
					break;
				case 43:
					scale_z = current->value.d_data;
					break;
				case 50:
					t_rot = current->value.d_data;
					break;
				case 62:
					color = current->value.i_data;
					break;
				case 67:
					paper = current->value.i_data;
					//printf("Paper %d\n", paper);
					break;
				case 210:
					extru_x = current->value.d_data;
					break;
				case 220:
					extru_y = current->value.d_data;
					break;
				case 230:
					extru_z = current->value.d_data;
					break;
				case 999:
					strcpy(comment, current->value.s_data);
					break;
			}
			
			
		}
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		/* complex entities */
		
		if (((ins_flag != 0) && (current == NULL))||
			((ins_flag != 0) && (current != NULL) && (current != insert_ent) && (current->type == DXF_ENT))){
			ins_flag = 0;
			/* look for block */
			blk = dxf_find_obj_descr2(drawing->blks, "BLOCK", name1);
			if ((blk) && /* block found */
			/* Also check the paper space parameter */
			(((p_space == 0) && (paper == 0)) || 
			((p_space != 0) && (paper != 0)))){
				
				
				
				
					
				/* find the layer index */
				insert_ent->obj.layer = dxf_lay_idx(drawing, layer);
				
				
				
				//printf ("bloco %s\n", name1);
				
				/* save current entity for future process */
				ins_stack_pos++;
				ins_stack[ins_stack_pos].ins_ent = blk;
				ins_stack[ins_stack_pos].prev = prev;
				
				if (ent_type == DXF_INSERT){
					ins_stack[ins_stack_pos].ofs_x = pt1_x;
					ins_stack[ins_stack_pos].ofs_y = pt1_y;
					ins_stack[ins_stack_pos].ofs_z = pt1_z;
					ins_stack[ins_stack_pos].scale_x = scale_x;
					ins_stack[ins_stack_pos].scale_y = scale_y;
					ins_stack[ins_stack_pos].scale_z = scale_z;
					ins_stack[ins_stack_pos].rot = t_rot;
					ins_stack[ins_stack_pos].color = dxf_ent_get_color(drawing, insert_ent, ins_stack[ins_stack_pos -1].color);
					ins_stack[ins_stack_pos].ltype = dxf_ent_get_ltype(drawing, insert_ent, ins_stack[ins_stack_pos - 1].ltype);
					ins_stack[ins_stack_pos].lw = dxf_ent_get_lw(drawing, insert_ent, ins_stack[ins_stack_pos - 1].lw);
					ins_stack[ins_stack_pos].normal[0] = extru_x;
					ins_stack[ins_stack_pos].normal[1] = extru_y;
					ins_stack[ins_stack_pos].normal[2] = extru_z;
					ins_stack[ins_stack_pos].elev = elev;
				}
				else{ /* to draw dimmensions */
					ins_stack[ins_stack_pos].ofs_x = 0.0;
					ins_stack[ins_stack_pos].ofs_y = 0.0;
					ins_stack[ins_stack_pos].ofs_z = 0.0;
					ins_stack[ins_stack_pos].scale_x = 1.0;
					ins_stack[ins_stack_pos].scale_y = 1.0;
					ins_stack[ins_stack_pos].scale_z = 1.0;
					ins_stack[ins_stack_pos].rot = 0.0;
					ins_stack[ins_stack_pos].color = 7;
					ins_stack[ins_stack_pos].normal[0] = 0.0;
					ins_stack[ins_stack_pos].normal[1] = 0.0;
					ins_stack[ins_stack_pos].normal[2] = 1.0;
					ins_stack[ins_stack_pos].elev = 0.0;
				}
				ins_stack[ins_stack_pos].start_idx = mod_idx;
				
				
				/* now, current is the block */
				prev = blk;
				current = blk;
				//p_space = paper;
				
				/*reinit_vars: */
				
				ent_type = DXF_NONE;
					
				pt1_x = 0; pt1_y = 0; pt1_z = 0; rot = 0;
				tick = 0; elev = 0; t_rot = 0;
				scale_x = 1.0; scale_y = 1.0; scale_z = 1.0;
				extru_x = 0.0; extru_y = 0.0; extru_z = 1.0;
				
				/* clear the strings */
				handle[0] = 0;
				l_type[0] = 0;
				layer[0] = 0;
				comment[0] = 0;
				name1[0] = 0;
				name2[0] = 0;
				
				color = 256; paper= 0;
				
				/*clear flags*/
				pt1 = 0;
				continue;
			}
		}
		
		else if (((blk_flag != 0) && (current == NULL))||
			((blk_flag != 0) && (current != NULL) && (current != blk) && (current->type == DXF_ENT))){
			blk_flag = 0;
			//p_space = paper;
			
			//printf("Bloco %0.2f, %0.2f, %0.2f\n", pt1_x, pt1_y, pt1_z);
		}
		
		if (prev == ent){ /* stop the search if back on initial entity */
			//printf("para\n");
			current = NULL;
			break;
		}
		
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			/* try to back in structure hierarchy */
			if (prev == ent){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				//indent --;
				if (prev == ins_stack[ins_stack_pos].ins_ent){/* back on initial entity */
					if (mod_idx > 0){
						graph_list_modify_idx(list_ret,
							ins_stack[ins_stack_pos].ofs_x,
							ins_stack[ins_stack_pos].ofs_y,
							ins_stack[ins_stack_pos].scale_x,
							ins_stack[ins_stack_pos].scale_y,
							ins_stack[ins_stack_pos].rot,
							ins_stack[ins_stack_pos].start_idx,
							mod_idx - 1
							);
						graph_list_mod_ax(list_ret,
							ins_stack[ins_stack_pos].normal,
							ins_stack[ins_stack_pos].elev,
							ins_stack[ins_stack_pos].start_idx,
							mod_idx - 1
							);
					}
					if (ins_stack_pos < 1){
						/* stop the search if back on initial entity */
						current = NULL;
						break;
					}
					else{
						prev = ins_stack[ins_stack_pos].prev;
						ins_stack_pos--;
						//prev = ins_stack[ins_stack_pos].ins_ent;
						//printf("retorna %d\n", ins_stack_pos);
						current = prev;
					}
				}
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	
	/* add attributes graphs at end */
	list_node *att_curr = att_list->next;
	while (att_curr != NULL){
		if (att_curr->data){
			list_push(list_ret, list_new((void *)att_curr->data, pool_idx));
		}
		att_curr = att_curr->next;
	}
	
	list_clear(att_list);
	
	//list_mem_pool(ZERO_LIST, ONE_TIME);
	return 1;
}

int dxf_ents_parse(dxf_drawing *drawing){
	dxf_node *current = NULL;
	list_node *vec_graph;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity 
				
				// -------------------------------------------
				vec_graph = dxf_graph_parse(drawing, current, 0, 0);
				if (vec_graph){
					current->obj.graphics = vec_graph;
				}
				/*if (current->obj.name){
					printf("%s\n", current->obj.name);
				}*/
				//---------------------------------------
			}
			current = current->next;
			//printf("%d\n", current);
		}
	}
}

list_node * dxf_graph_parse(dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	/* Initialize */
	/*create the vector of returned values */
	list_node * list_ret = list_new(NULL, pool_idx);
	dxf_obj_parse(list_ret, drawing, ent, p_space, pool_idx);
	return list_ret;
}

list_node * dxf_list_parse(dxf_drawing *drawing, list_node *list, int p_space, int pool_idx){
	list_node *current = NULL;
	
		
	if (list != NULL){
		/* Initialize */
		/*create the vector of returned values */
		list_node * list_ret = list_new(NULL, pool_idx);
		
		current = list->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->data){
				if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
					
					// -------------------------------------------
					dxf_obj_parse(list_ret, drawing, ((dxf_node *)current->data), p_space, pool_idx);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
		return list_ret;
	}
	return NULL;
}

int dxf_ents_draw(dxf_drawing *drawing, bmp_img * img, double ofs_x, double ofs_y, double scale){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					// -------------------------------------------
					graph_list_draw(current->obj.graphics, img, ofs_x, ofs_y, scale);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

int dxf_list_draw(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color){
	list_node *current = NULL;
		
	if (list != NULL){
		current = list->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->data){
				if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
					// -------------------------------------------
					graph_list_draw_fix(((dxf_node *)current->data)->obj.graphics, img, ofs_x, ofs_y, scale, color);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

int dxf_ents_ext(dxf_drawing *drawing, double * min_x, double * min_y, double * max_x, double * max_y){
	dxf_node *current = NULL;
	int ext_ini = 0;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){
				/*verify if entity layer is thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if (!drawing->layers[current->obj.layer].frozen){
					graph_list_ext(current->obj.graphics, &ext_ini, min_x, min_y, max_x, max_y);
				}
				
				/* -------------------------------------------
				if (ext_ini == 0){
					ext_ini = 1;
					*min_x = current->ext_min_x;
					*min_y = current->ext_min_y;
					*max_x = current->ext_max_x;
					*max_y = current->ext_max_y;
				}
				else{
					*min_x = (*min_x < current->ext_min_x) ? *min_x : current->ext_min_x;
					*min_y = (*min_y < current->ext_min_y) ? *min_y : current->ext_min_y;
					*max_x = (*max_x > current->ext_max_x) ? *max_x : current->ext_max_x;
					*max_y = (*max_y > current->ext_max_y) ? *max_y : current->ext_max_y;
				}
				
				//---------------------------------------*/
			}
			current = current->next;
		}
	}
}

dxf_node * dxf_ents_isect(dxf_drawing *drawing, double rect_pt1[2], double rect_pt2[2]){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity 
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
					// -------------------------------------------
					//graph_list_draw(current->obj.graphics, img, ofs_x, ofs_y, scale);
					
					//graph_obj * graph_list_isect(list_node * vec, double rect_pt1[2], double rect_pt2[2]);
					if(graph_list_isect(current->obj.graphics, rect_pt1, rect_pt2)){
						return current;
					}
					
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
	return NULL;
}

int dxf_ents_isect2(list_node *list, dxf_drawing *drawing, double rect_pt1[2], double rect_pt2[2]){
	/* find all visible elements in drawing that intercept the given rectangle */
	/* return the number of found elements */
	
	dxf_node *current = NULL;
	int num = 0;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && /*verify the integrity of drawing */
	(drawing->main_struct != NULL)
	&& (list != NULL)){ /* and if list exists */
		
		list_clear(list);
		
		/* sweep the drawing content */
		current = drawing->ents->obj.content->next;
		while (current != NULL){
			if (current->type == DXF_ENT){ /* found a DXF entity  */
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) &&
					(!drawing->layers[current->obj.layer].frozen)){
					/* look for a instersect in entity's visible graphics */
					if(graph_list_isect(current->obj.graphics, rect_pt1, rect_pt2)){
						/* append entity,  if it does not already exist in the list */
						if (list_find_data(list, current)){
							//printf ("DEBUG: already exists!\n");
						}
						else{
							list_node * new_el = list_new(current, 1);
							if (new_el){
								list_push(list, new_el);
								num++;
							}
						}
					}
				}
			}
			current = current->next;
		}
	}
	
	return num;
}


int dxf_find_hatch_bound(dxf_node *obj, dxf_node **start, dxf_node **end){
	/* find the range of attributes of boundary data of hatch */
	dxf_node *current;
	int found = 0;
	int ok = 0;
	
	*start = NULL;
	*end = NULL;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				/* try to find the first entry, by matching the attribute 91 -> number of boundaries */
				if (!found){
					if (current->type == DXF_ATTR){
						if(current->value.group == 91){
							found = current->value.i_data; /* found - return number of boundaries*/
							*start = current;
							*end = current;
						}
					}
				}
				else{
					/* after the first entry, look by end */
					if (current->type == DXF_ATTR){
						/* breaks if is found a hatch style entry (code 75) */
						if(current->value.group == 75){
							ok = 1;
							break;
						}
						/* update the end mark */
						*end = current;
					}
					/* breaks if is found a entity */
					else break;
				}
				current = current->next;
			}
			if (!ok) found = 0; /* if is not correct end*/ 
		}
	}
	return found;
}

int dxf_find_hatch_patt(dxf_node *obj, dxf_node **start, dxf_node **end){
	/* find the range of attributes of patterns data of hatch */
	dxf_node *current;
	int found = 0;
	int ok = 0;
	
	*start = NULL;
	*end = NULL;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				/* try to find the first entry, by matching the attribute 78 -> number of lines */
				if (!found){
					if (current->type == DXF_ATTR){
						if(current->value.group == 78){
							found = current->value.i_data; /* found - return number of boundaries*/
							*start = current;
							*end = current;
						}
					}
				}
				else{
					/* after the first entry, look by end */
					if (current->type == DXF_ATTR){
						/* breaks if is found a number seed entry (code 98) */
						if(current->value.group == 98){
							ok = 1;
							break;
						}
						/* update the end mark */
						*end = current;
					}
					/* breaks if is found a entity */
					else break;
				}
				current = current->next;
			}
			//if (!ok) found = 0; /* if is not correct end*/ 
		}
	}
	return found;
}

int dxf_hatch_get_bound(graph_obj **curr_graph, dxf_node * ent, dxf_node **next, int pool_idx){
	int num_bound = 0;
	*next = NULL;
	*curr_graph = NULL;
	
	if(ent){
		dxf_node *current = NULL;
		
		double pt1_x = 0, pt1_y = 0, pt2_x = 0, pt2_y = 0;
		//double start_w = 0, end_w = 0, fix_w = 0;
		double bulge = 0;
		//double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		//int pline_flag = 0;
		int curr_bound = 0, prev_bound = 0;
		int curr_edge = 0, prev_edge = 0;
		int bound_type = 0;
		int first = 0, closed =0, ccw = 0;
		double prev_x, prev_y,  last_x, last_y,  curr_x;
		double radius, start_ang, end_ang;
		double prev_bulge = 0;
		//double elev = 0.0;
		
		int num_cpts, order, num_ret, prev_num_cpts, prev_order;
		double weight = 1.0;
		double ctrl_pts[3 * MAX_SPLINE_PTS], ret[3 * MAX_SPLINE_PTS];
		double weights[MAX_SPLINE_PTS];

		int count =0;
		
		enum Edge_type{
			EDGE_POLY = 0,
			EDGE_LINE = 1,
			EDGE_CIRC_ARC = 2,
			EDGE_ELL_ARC  = 3,
			EDGE_SPLINE = 4,
			EDGE_NONE = 5
		};
		int edge_type = EDGE_NONE;
		int prev_edge_type = EDGE_NONE;
		
		/*flags*/
		int pt1 = 0, init = 0;
		
		current = ent;
		
		
		while (current){
			
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						pt1_x = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 11:
						pt2_x = current->value.d_data; 
						break;
					case 20:
						pt1_y = current->value.d_data; 
						break;
					case 21:
						pt2_y = current->value.d_data; 
						break;
					case 40:
						radius = current->value.d_data;
						break;
					case 42:
						bulge = current->value.d_data;
						weight = current->value.d_data;
						break;
					case 50:
						start_ang = current->value.d_data;
						break;
					case 51:
						end_ang = current->value.d_data;
						break;
					case 72:
						if (!(bound_type & 2)){
							edge_type = current->value.i_data;
							curr_edge++;
						}
						break;
					case 73:
						closed = current->value.i_data;
						ccw = (current->value.i_data != 0)? 1: -1;
						break;
					case 75: /* end of bondary definitions */
						bound_type = 0;
						curr_bound++;
						//pt1 = 1;
						break;
					case 92:
						bound_type = current->value.i_data;
						curr_bound++;
						break;
					case 94:
						order = current->value.i_data;
						break;
					case 96:
						num_cpts = current->value.i_data;
						break;
				}
				
				
			}
			if ((curr_bound != prev_bound) || (current->next == NULL)){
				if(init == 0) {
					init = 1;
					*curr_graph = graph_new(pool_idx);
				}
				
				
				/* ends the previous boundary */
				if (prev_edge_type == EDGE_POLY){
					/* last vertex */
					if((first > 1) && (*curr_graph != NULL)){
						//printf("(%0.2f, %0.2f)-(%0.2f, %0.2f)\n", prev_x, prev_y, curr_x, pt1_y);
						if (prev_bulge == 0){
							line_add(*curr_graph, prev_x, prev_y, 0.0, curr_x, pt1_y, 0.0);
						}
						else{
							graph_arc_bulge(*curr_graph, prev_x, prev_y, 0.0, curr_x, pt1_y, 0.0, prev_bulge);
							//bulge =0;
						}
						prev_x = curr_x;
						prev_y = pt1_y;
					}
					
					if((closed != 0) && (*curr_graph != NULL)){
						if (bulge == 0){
							line_add(*curr_graph, prev_x, prev_y, 0.0, last_x, last_y, 0.0);
						}
						else{
							graph_arc_bulge(*curr_graph, prev_x, prev_y, 0.0, last_x, last_y, 0.0, bulge);
						}
					}
				}
				else if (prev_edge_type == EDGE_LINE){
					
					if (*curr_graph != NULL){
						line_add(*curr_graph, curr_x, pt1_y, 0.0, pt2_x, pt2_y, 0.0);
					}
				}
				else if (prev_edge_type == EDGE_CIRC_ARC){
					
					if (*curr_graph != NULL){
						if (ccw < 0){
							double tmp;
							start_ang = 360 - start_ang;
							end_ang = 360 - end_ang;
						}
						graph_arc(*curr_graph, curr_x, pt1_y, 0.0 , radius, start_ang, end_ang, ccw);
					}
				}
				else if (prev_edge_type == EDGE_ELL_ARC){
					
					if((first > 0) && (*curr_graph != NULL)){
						
						start_ang *= M_PI/180.0;
						end_ang *= M_PI/180.0;
						
						start_ang =  ellipse_par (start_ang, 1.0, radius);
						end_ang =  ellipse_par (end_ang, 1.0, radius);
						
						if (ccw < 0){
							double tmp;
							tmp = start_ang;
							start_ang = 2*M_PI - end_ang;
							end_ang = 2*M_PI - tmp;
						}
						graph_ellipse(*curr_graph, curr_x, pt1_y, 0.0, pt2_x, pt2_y, 0.0, radius, start_ang, end_ang);
					}
				}
				else if (prev_edge_type == EDGE_SPLINE){
					
					if((first > 0) && (*curr_graph != NULL) && (count < MAX_SPLINE_PTS)){
						ctrl_pts[count*3+1] = curr_x;
						ctrl_pts[count*3+2] = pt1_y;
						ctrl_pts[count*3+3] = 0.0;
						weights[count+1] = weight;
						count++;
					}
					if ((*curr_graph) && ((count + prev_order)*5 < MAX_SPLINE_PTS)){
						int i;
						num_ret = (prev_num_cpts + prev_order)*5; /* num pts on curve */
						
						for(i = 1; i <= 3*num_ret; i++){
							ret[i] = 0.0;
						}
						
						rbspline(prev_num_cpts, prev_order+1, num_ret, ctrl_pts, weights, ret);
						
						prev_x = ret[1];
						prev_y = ret[2];
						//prev_z = ret[3];
						
						for(i =4 ; i <= 3*num_ret; i = i+3){
							line_add(*curr_graph, prev_x, prev_y, 0.0, ret[i], ret[i+1], 0.0);
							prev_x = ret[i];
							prev_y = ret[i+1];
							//prev_z = ret[i+2];
							/*printf(" %f %f %f \n",ret[i],ret[i+1],ret[i+2]);*/
						}
					}
				}
				
				
				/* verify if boundary is closed - TODO*/
				if(*curr_graph != NULL){
					num_bound++;
				}
				
				
				/* initialize current boundary*/
				//init = 0;
				//*curr_graph = NULL;
				first = 0; pt1 = 0; closed = 0; prev_bulge = 0;
				radius = 0; start_ang = 0; end_ang = 0;
				count = 0;
				
				edge_type = EDGE_NONE;
				prev_edge_type = EDGE_NONE;
				if (bound_type & 2) edge_type = EDGE_POLY;
				//prev_edge_type = edge_type;
				
				prev_bound = curr_bound;
				
				if ((current->next == NULL) && (edge_type != EDGE_POLY)){
					pt1 = 1;
				}
			}
			
			
			if (pt1){
				pt1 = 0;
				if(first == 1){
					
					//printf("primeiro vertice\n");
					last_x = curr_x;
					last_y = pt1_y;
					prev_x = curr_x;
					prev_y = pt1_y;
				}
				if (prev_edge_type == EDGE_POLY) {
					
					if((first > 1) && (*curr_graph != NULL)){
						//printf("(%0.2f, %0.2f)-(%0.2f, %0.2f)\n", prev_x, prev_y, curr_x, pt1_y);
						if (prev_bulge == 0){
							line_add(*curr_graph, prev_x, prev_y, 0.0, curr_x, pt1_y, 0.0);
						}
						else{
							graph_arc_bulge(*curr_graph, prev_x, prev_y, 0.0, curr_x, pt1_y, 0.0, prev_bulge);
							//bulge =0;
						}
						prev_x = curr_x;
						prev_y = pt1_y;
					}
				}
				else if (prev_edge_type == EDGE_LINE){
					
					if((first > 0) && (*curr_graph != NULL)){
						line_add(*curr_graph, curr_x, pt1_y, 0.0, pt2_x, pt2_y, 0.0);
					}
				}
				else if (prev_edge_type == EDGE_CIRC_ARC){
					
					if((first > 0) && (*curr_graph != NULL)){
						if (ccw < 0){
							start_ang = 360 - start_ang;
							end_ang = 360 - end_ang;
						}
						graph_arc(*curr_graph, curr_x, pt1_y, 0.0 , radius, start_ang, end_ang, ccw);
					}
				}
				else if (prev_edge_type == EDGE_ELL_ARC){
					
					if((first > 0) && (*curr_graph != NULL)){
						
						start_ang *= M_PI/180.0;
						end_ang *= M_PI/180.0;
						
						start_ang =  ellipse_par (start_ang, 1.0, radius);
						end_ang =  ellipse_par (end_ang, 1.0, radius);
						
						if (ccw < 0){
							double tmp;
							tmp = start_ang;
							start_ang = 2*M_PI - end_ang;
							end_ang = 2*M_PI - tmp;
						}
						graph_ellipse(*curr_graph, curr_x, pt1_y, 0.0, pt2_x, pt2_y, 0.0, radius, start_ang, end_ang);
					}
				}
				else if (prev_edge_type == EDGE_SPLINE){
					
					if((first > 0) && (*curr_graph != NULL) && (count < MAX_SPLINE_PTS)){
						ctrl_pts[count*3+1] = curr_x;
						ctrl_pts[count*3+2] = pt1_y;
						ctrl_pts[count*3+3] = 0.0;
						weights[count+1] = weight;
						count++;
					}
					if ((prev_edge != curr_edge) && (*curr_graph) && ((count + prev_order)*5 < MAX_SPLINE_PTS)){
						int i;
						num_ret = (prev_num_cpts + prev_order)*5; /* num pts on curve */
						
						for(i = 1; i <= 3*num_ret; i++){
							ret[i] = 0.0;
						}
						
						rbspline(prev_num_cpts, prev_order+1, num_ret, ctrl_pts, weights, ret);
						
						prev_x = ret[1];
						prev_y = ret[2];
						//prev_z = ret[3];
						
						for(i =4 ; i <= 3*num_ret; i = i+3){
							line_add(*curr_graph, prev_x, prev_y, 0.0, ret[i], ret[i+1], 0.0);
							prev_x = ret[i];
							prev_y = ret[i+1];
							//prev_z = ret[i+2];
							/*printf(" %f %f %f \n",ret[i],ret[i+1],ret[i+2]);*/
						}
						count = 0;
					}
				}
				
				prev_bulge = bulge;
				prev_num_cpts = num_cpts;
				prev_order = order;
				bulge = 0;
				weight = 1.0;
				
				first ++;
				curr_x = pt1_x;
				
				radius = 0; start_ang = 0; end_ang = 0;
				prev_edge = curr_edge;
				prev_edge_type = edge_type;
				
			}
			
			
			/* breaks loop if is found a hatch style entry (code 75) */
			if(current->value.group == 75){
				
				break;
			}
			
			
			/* update the end mark */
			*next = current;
			current = current->next; /* go to the next in the list */
		}
		
		
		
		//return current;
	}
	return num_bound;
}

int dxf_hatch_get_def(list_node *list_ret, graph_obj *bound, dxf_node * ent, dxf_node **next, double p_scale, int pool_idx){
	int num_def = 0;
	*next = NULL;
	graph_obj *curr_graph = NULL;
	
	if(ent){
		dxf_node *current = NULL;
		
		int curr_def = 0, prev_def = 0, init =0;
		double angle = 0.0, orig_x = 0.0, orig_y = 0.0;
		double prev_angle = 0.0;
		double ofs_x = 0.0, ofs_y = 0.0;
		double dash[DXF_MAX_PAT];
		
		dash[0] = 1;
		int num_dash = 0;
		
		current = ent;
		
		
		while (current){
			
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 53:
						angle = current->value.d_data;
						curr_def++;
						break;
					case 43:
						orig_x = current->value.d_data; 
						break;
					case 44:
						orig_y = current->value.d_data; 
						break;
					case 45:
						ofs_x = current->value.d_data; 
						break;
					case 46:
						ofs_y = current->value.d_data; 
						break;
					case 49:
						if (num_dash < DXF_MAX_PAT){
							dash[num_dash] = current->value.d_data;
							num_dash++;
						}
						break;
					case 98: /* end of definition lines */
						curr_def++;
						break;
				}
				
				
			}
			if ((curr_def != prev_def) || (current->next == NULL)){
				/* ends the previous definition line */
				if (!init){
					init = 1;
				}
				else{
					//double hatch_spacing = sqrt(pow(ofs_x, 2) + pow(ofs_y,2));
					//curr_graph = graph_hatch2(bound, prev_angle * M_PI/180,
					//					orig_x, orig_y, hatch_spacing, 0.0, pool_idx);
					curr_graph = graph_hatch(bound, prev_angle * M_PI/180,
										orig_x, orig_y, ofs_x, ofs_y, dash, num_dash,  pool_idx);
					
					/* store the graph in the return vector */
					if ((curr_graph != NULL) && (list_ret != NULL)) list_push(list_ret, list_new((void *)curr_graph, pool_idx));
				}
				
				num_def++;
				
				/* initialize current definition line*/
				orig_x = 0.0; orig_y = 0.0;
				ofs_x = 0.0; ofs_y = 0.0;
				num_dash = 0;
				dash[0] = 1;
				
				prev_def = curr_def;
				prev_angle = angle;
			}
			
			
			/**/
			
			/* breaks loop if is found a hatch style entry (code 75) */
			if(current->value.group == 98){
				
				break;
			}
			
			
			/* update the end mark */
			*next = current;
			current = current->next; /* go to the next in the list */
		}
		
		
		
		//return current;
	}
	return num_def;
}

int dxf_hatch_parse(list_node *list_ret, dxf_drawing *drawing, dxf_node * ent, int p_space, int pool_idx){
	int num_graph = 0;
	if(ent){
		dxf_node *current = NULL;
		dxf_node *next;
		graph_obj *curr_graph;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		double elev = 0.0;
		double p_angle = 0.0, p_scale =1.0;
		
		char name_patt[DXF_MAX_CHARS];
		
		/*flags*/
		int paper = 0, solid = 0, assoc = 0, patt_double = 0;
		
		int num_bound = 0, h_style = 0, h_type = 0, num_def = 0, num_seed = 0;
		
		if (ent->type == DXF_ENT){
			if (ent->obj.content){
				current = ent->obj.content->next;
				//printf("%s\n", ent->obj.name);
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 2:
						strcpy(name_patt, current->value.s_data);
						break;
					case 30:
						elev = current->value.d_data;
						break;
					case 41:
						p_scale = current->value.d_data;
						break;
					case 52:
						p_angle = current->value.d_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 70:
						solid = current->value.i_data;
						break;
					case 71:
						assoc = current->value.i_data;
						break;
					case 75:
						h_style = current->value.i_data;
						break;
					case 76:
						h_type = current->value.i_data;
						break;
					case 77:
						patt_double = current->value.i_data;
						break;
					case 78:
						num_def = current->value.i_data;
						
						//dxf_node *next;
						/*graph_obj *lines_graph;
						dxf_hatch_get_def(&lines_graph, curr_graph, current, &next, pool_idx);
						if (lines_graph){
							curr_graph = lines_graph;
						}*/
						if (!solid){
							dxf_hatch_get_def(list_ret, curr_graph, current, &next, p_scale, pool_idx);
							if (next) current = next;
						}
					
						break;
					case 91:
						num_bound = current->value.i_data;
						
						curr_graph = NULL;
						dxf_hatch_get_bound (&curr_graph, current, &next, pool_idx);
						/* store the graph in the return vector */
						if ((curr_graph != NULL) && (list_ret != NULL)){
							if (solid) curr_graph->fill = 1;
							if (!assoc){
								curr_graph->patt_size = 1;
								curr_graph->pattern[0] = -1.0;
							}
							list_push(list_ret, list_new((void *)curr_graph, pool_idx));
						}
					
						if (next) current = next;
						break;
					
					case 98:
						num_seed = current->value.i_data;
					
						
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (((p_space == 0) && (paper == 0)) || ((p_space != 0) && (paper != 0))){
			/* convert OCS to WCS */
			normal[0] = extru_x;
			normal[1] = extru_y;
			normal[2] = extru_z;
			if (list_ret != NULL){
				list_node *curr_node = list_ret->next;
				
				// starts the content sweep 
				while (curr_node != NULL){
					if (curr_node->data){
						graph_mod_axis((graph_obj *)curr_node->data, normal, elev);
						num_graph++;
					}
					curr_node = curr_node->next;
				}
			}
		}
	}
	return num_graph;
}