#include "dxf_dim.h"

list_node * dxf_dim_linear_make(dxf_drawing *drawing, dxf_node * ent){
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
	/* points to draw terminators */
	double t1[2], t2[2], t3[2], t4[2];
	
	int flags = 32;
	int an_place = 5; /* annotation placement (5 = middle center) */

	char user_txt[DXF_MAX_CHARS+1] = "<>";
	char tmp_str[DXF_MAX_CHARS+1] = "";
	char result_str[DXF_MAX_CHARS+1] = "";
	
	enum { FILLED, OPEN, OPEN30, OPEN90, CLOSED, OBLIQUE, ARCHTICK, NONE } term_typ = FILLED;
	
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
					strncpy(tmp_str, current->value.s_data, DXF_MAX_CHARS);
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
	
	/* base line */
	dxf_node * obj = dxf_new_line (0.0, 0.0, 0.0, -measure, 0.0, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* terminators */
	dir = (measure > 0.0) ? 1.0 : -1.0;
	strncpy(tmp_str, drawing->dimblk, DXF_MAX_CHARS); /* preserve original string */
	str_upp(tmp_str); /*upper case */
	if (strcmp(tmp_str, "OPEN") == 0) term_typ = OPEN;
	else if (strcmp(tmp_str, "OPEN30") == 0) term_typ = OPEN30;
	else if (strcmp(tmp_str, "OPEN90") == 0) term_typ = OPEN90;
	else if (strcmp(tmp_str, "CLOSED") == 0) term_typ = CLOSED;
	else if (strcmp(tmp_str, "OBLIQUE") == 0) term_typ = OBLIQUE;
	else if (strcmp(tmp_str, "ARCHTICK") == 0) term_typ = ARCHTICK;
	else if (strcmp(tmp_str, "NONE") == 0) term_typ = NONE;
	
	if (term_typ == NONE){
		
	}
	else if (term_typ == OPEN || term_typ == OPEN30 || term_typ == OPEN90 || term_typ == CLOSED){
		t1[0] = 0.0; t1[1] = 0.0;
		t2[0] = -drawing->dimscale * dir;
		t2[1] = drawing->dimscale * 0.20;
		if (term_typ == OPEN30) t2[1] = drawing->dimscale * 0.2588;
		else if (term_typ == OPEN90) t2[1] = drawing->dimscale * 0.7071;
		t3[0] = -measure; t3[1] = 0.0;
		t4[0] = drawing->dimscale * dir - measure; t4[1] = t2[1];
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t3[0], t3[1], 0.0, t4[0], t4[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t3[0], t3[1], 0.0, t4[0], -t4[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		if (term_typ == CLOSED){
			obj = dxf_new_line (t2[0], t2[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
			dxf_edit_rot (obj, rot);
			dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
			list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
			
			obj = dxf_new_line (t4[0], t4[1], 0.0, t4[0], -t4[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
			dxf_edit_rot (obj, rot);
			dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
			list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		}
		
	}
	else if (term_typ == OBLIQUE || term_typ == ARCHTICK){
		t1[0] = drawing->dimscale * 0.5;
		t1[1] = drawing->dimscale * 0.5;
		t2[0] = -t1[0];
		t2[1] = -t1[1];
		t3[0] = drawing->dimscale * 0.5 - measure; t3[1] = t1[1];
		t4[0] = -drawing->dimscale * 0.5 - measure; t4[1] = t2[1];
		
		int tick = -2;
		if (term_typ == ARCHTICK) tick = 50;
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", tick, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t3[0], t3[1], 0.0, t4[0], t4[1], 0.0, 0, "0", "BYBLOCK", tick, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	}
	else{ /* FILLED */
		t1[0] = 0.0; t1[1] = 0.0;
		t2[0] = -drawing->dimscale * dir;
		t2[1] = drawing->dimscale * 0.20;
		t3[0] = -measure; t3[1] = 0.0;
		t4[0] = drawing->dimscale * dir - measure; t4[1] = t2[1];
		
		obj = dxf_new_solid (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_solid (t3[0], t3[1], 0.0, t4[0], t4[1], 0.0, t4[0], -t4[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	}
	
	/* anotation */
	char *subst = strstr(user_txt, "<>"); /* try to find string "<>"  in user text to replace with measure */
	if (subst) { /* proceed to replace */
		int len_front = subst - user_txt;
		strncpy(result_str, user_txt, len_front); /* prefix */
		snprintf (tmp_str, DXF_MAX_CHARS, "%.*f", drawing->dimdec, fabs(measure) * drawing->dimlfac); /* measure */
		strncat(result_str, tmp_str, DXF_MAX_CHARS - len_front); /* prefix + measure */
		len_front = strlen(result_str);
		subst += 2;
		strncat(result_str, subst, DXF_MAX_CHARS - len_front); /* prefix + measure + sufix*/
	}
	else if (strlen(user_txt) == 0) { /* if user text  is "", then mesaure will be the annotation */
		snprintf (result_str, DXF_MAX_CHARS, "%.*g", drawing->dimdec, fabs(measure) * drawing->dimlfac);
	}
	else { /* no replace mark - use the entire user text */
		strncpy(result_str, user_txt, DXF_MAX_CHARS);
	}
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){result_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	if (dxf_tstyle_idx (drawing, drawing->dimtxsty) >= 0){
		dxf_attr_change(obj, 7, drawing->dimtxsty);
	}
	dxf_edit_scale (obj, drawing->dimscale, drawing->dimscale, drawing->dimscale);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines */
	dir = ((-sine*(base_pt[0]  - pt1[0])+ cosine*(base_pt[1]  - pt1[1])) > 0.0) ? 1.0 : -1.0;
	
	obj = dxf_new_line (base_pt[0], base_pt[1], base_pt[2], pt1[0], pt1[1], pt1[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_move (obj, -drawing->dimscale * sine * 0.5 * dir, drawing->dimscale * cosine * 0.5 * dir, 0.0);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	obj = dxf_new_line (base_pt[0] - cosine * measure, base_pt[1] - sine * measure, base_pt[2], 
		pt0[0], pt0[1], pt0[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_move (obj, -drawing->dimscale * sine * 0.5 * dir, drawing->dimscale * cosine * 0.5 * dir, 0.0);
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