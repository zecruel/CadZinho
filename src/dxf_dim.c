#include "dxf_dim.h"

list_node * dxf_dim_linear_make(dxf_drawing *drawing, dxf_node * ent, double scale, 
	double an_scale, int an_format, 
	int term_type,
	int tol_type, double tol_up, double tol_low)
{
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	dxf_node *current = NULL, *nxt_atr = NULL, *nxt_ent = NULL, *vertex = NULL;
	graph_obj *curr_graph = NULL;
	
	double base_pt[3] = {0.0, 0.0, 0.0}; /* dim base point */
	double an_pt[3] = {0.0, 0.0, 0.0}; /* annotation point */
	double measure = 0.0;
	/* two placement points - "measure" */
	double pt0[3] = {0.0, 0.0, 0.0};
	double pt1[3] = {0.0, 0.0, 0.0};
	double rot = 0.0; /* dimension rotation */
	
	int flags = 32;
	int an_place = 5; /* annotation placement (5 = middle center) */

	char user_txt[DXF_MAX_CHARS+1] = "<>";
	char tmp_str[21] = "";
	
	current = ent->obj.content->next;
	
	/* get DIMENSION parameters */
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				/* dim base point */
				case 10:
					base_pt[0] = current->value.d_data;
					break;
				case 20:
					base_pt[1] = current->value.d_data;
					break;
				case 30:
					base_pt[2] = current->value.d_data;
					break;
				/* annotation point */
				case 11:
					an_pt[0] = current->value.d_data;
					break;
				case 21:
					an_pt[1] = current->value.d_data;
					break;
				case 31:
					an_pt[2] = current->value.d_data;
					break;
				/* two placement points - "measure" */
				case 13:
					pt0[0] = current->value.d_data;
					break;
				case 23:
					pt0[1] = current->value.d_data;
					break;
				case 33:
					pt0[2] = current->value.d_data;
					break;
				case 14:
					pt1[0] = current->value.d_data;
					break;
				case 24:
					pt1[1] = current->value.d_data;
					break;
				case 34:
					pt1[2] = current->value.d_data;
					break;
				/* user text */
				case 1:
					strncpy(user_txt, current->value.s_data, DXF_MAX_CHARS);
					break;
				/* flags*/
				case 70:
					flags = current->value.i_data;
					break;
				/* annotation placement */
				case 71:
					an_place = current->value.i_data;
					break;
				/* measure */
				case 42:
					measure = current->value.d_data;
					break;
				/* rotation - angle in degrees */
				case 50:
					rot = current->value.d_data;
					break;
				
				case 101:
					strncpy(tmp_str, current->value.s_data, 20);
					str_upp(tmp_str);
					char *tmp = trimwhitespace(tmp_str);
					if (strcmp (tmp, "EMBEDDED OBJECT") == 0 ){
						current = NULL;
						continue;
					}
			}
		}
		current = current->next; /* go to the next in the list */
	}
	
	if ((flags & 7) > 1) return NULL; /* verify type of dimension - linear dimension */
	
	double cosine = cos(rot*M_PI/180.0);
	double sine = sin(rot*M_PI/180.0);
	double dir = 1.0;
	
	list_node * list = list_new(NULL, FRAME_LIFE);
	//list_push(list, list_new((void *)curr_graph, FRAME_LIFE)); /* store entity in list */
	//dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z)
	
	/* base line */
	dxf_node * obj = dxf_new_line (0.0, 0.0, 0.0, -measure, 0.0, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* terminators */
	dir = (measure > 0.0) ? 1.0 : -1.0;
	
	obj = dxf_new_line (0.0, 0.0, 0.0, -scale*dir, scale*0.25, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	obj = dxf_new_line (0.0, 0.0, 0.0, -scale*dir, -scale*0.25, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	obj = dxf_new_line (-measure, 0.0, 0.0, scale*dir-measure, scale*0.25, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	obj = dxf_new_line (-measure, 0.0, 0.0, scale*dir-measure, -scale*0.25, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* anotation */
	snprintf (tmp_str, 20, "%f", fabs(measure) * an_scale);
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){tmp_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	dxf_edit_scale (obj, scale, scale, scale);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines */
	dir = ((-sine*(base_pt[0]  - pt1[0])+ cosine*(base_pt[1]  - pt1[1])) > 0.0) ? 1.0 : -1.0;
	
	obj = dxf_new_line (base_pt[0], base_pt[1], base_pt[2], pt1[0], pt1[1], pt1[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_move (obj, -scale * sine * 0.5 * dir, scale * cosine * 0.5 * dir, 0.0);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	obj = dxf_new_line (base_pt[0] - cosine * measure, base_pt[1] - sine * measure, base_pt[2], 
		pt0[0], pt0[1], pt0[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_move (obj, -scale * sine * 0.5 * dir, scale * cosine * 0.5 * dir, 0.0);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	
	return list;
}

int dxf_find_last_dim (dxf_drawing *drawing){
	if (!drawing) return 0;
	if (!drawing->blks) return 0;
	char test_descr[DXF_MAX_CHARS+1];
	
	dxf_node *current, *descr_attr;
	int last = 1;
	int curr_num;
	
	/* sweep blocks list contents */
	current = drawing->blks->obj.content->next;
	while (current){
		if (current->type == DXF_ENT){ /* look for dxf entities */
			if(strcmp(current->obj.name, "BLOCK") == 0){ /* match blocks */
				descr_attr = dxf_find_attr2(current, 2); /* look for descriptor in group 2 attribute */
				if (descr_attr){ /* found attribute */
					/* copy strings for secure manipulation */
					strncpy(test_descr, descr_attr->value.s_data, DXF_MAX_CHARS);
					/* change to upper case */
					str_upp(test_descr);
					/* look for Block name starting by "*D"*/
					if (strlen(test_descr) > 2){
						if (test_descr[0] == '*' && test_descr[1] == 'D'){
							/* convert the remain string to number */
							curr_num = atoi(test_descr+2);
							if (curr_num >= last){ /* update the last with greater number */
								last = curr_num + 1;
							}
						}
					}
				}
			}
		}
		current = current->next;
	}
	
	return last;
}