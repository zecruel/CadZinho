#include "dxf_dim.h"
#include "dxf_attract.h"

list_node * dxf_dim_make(dxf_drawing *drawing, dxf_node * ent){
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	int flags = 0;
	
	dxf_node *flags_obj = dxf_find_attr2(ent, 70); /* get block name */
	if(flags_obj) flags = flags_obj->value.i_data & 7;
	
	if (flags == 0 || flags == 1){
		return dxf_dim_linear_make(drawing, ent);
	}
	else if (flags == 2){
		return dxf_dim_angular_make(drawing, ent);
	}
	else if (flags == 3 || flags == 4){
		return dxf_dim_radial_make(drawing, ent);
	}
	else if (flags == 6){
		return dxf_dim_ordinate_make(drawing, ent);
	}
	return NULL;
}

list_node * dxf_dim_linear_make(dxf_drawing *drawing, dxf_node * ent){
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	dxf_node *current = NULL;
	
	
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
	
	dxf_dimsty dim_sty;
	strncpy(dim_sty.name, "STANDARD", DXF_MAX_CHARS);
	
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
				/* dim style */
				case 3:
					strncpy(dim_sty.name, current->value.s_data, DXF_MAX_CHARS);
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
	
	/*init drawing style */
	dxf_dim_get_sty(drawing, &dim_sty);
	
	/* base line */
	dxf_node * obj = dxf_new_line (0.0, 0.0, 0.0, -measure, 0.0, 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* terminators */
	dir = (measure > 0.0) ? 1.0 : -1.0;
	strncpy(tmp_str, dim_sty.a_type, DXF_MAX_CHARS); /* preserve original string */
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
		t2[0] = -dim_sty.scale * dim_sty.a_size * dir;
		t2[1] = dim_sty.scale * dim_sty.a_size * 0.1667;
		if (term_typ == OPEN30) t2[1] = dim_sty.scale * dim_sty.a_size * 0.2588;
		else if (term_typ == OPEN90) t2[1] = dim_sty.scale * dim_sty.a_size * 0.7071;
		t3[0] = -measure; t3[1] = 0.0;
		t4[0] = dim_sty.scale * dim_sty.a_size * dir - measure; t4[1] = t2[1];
		
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
		t1[0] = dim_sty.scale * dim_sty.a_size * 0.5;
		t1[1] = dim_sty.scale * dim_sty.a_size * 0.5;
		t2[0] = -t1[0];
		t2[1] = -t1[1];
		t3[0] = dim_sty.scale * dim_sty.a_size * 0.5 - measure; t3[1] = t1[1];
		t4[0] = -dim_sty.scale * dim_sty.a_size * 0.5 - measure; t4[1] = t2[1];
		
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
		t2[0] = -dim_sty.scale * dim_sty.a_size * dir;
		t2[1] = dim_sty.scale * dim_sty.a_size * 0.1667;
		t3[0] = -measure; t3[1] = 0.0;
		t4[0] = dim_sty.scale * dim_sty.a_size * dir - measure; t4[1] = t2[1];
		
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
		snprintf (tmp_str, DXF_MAX_CHARS, "%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale); /* measure */
		strncat(result_str, tmp_str, DXF_MAX_CHARS - len_front); /* prefix + measure */
		len_front = strlen(result_str);
		subst += 2;
		strncat(result_str, subst, DXF_MAX_CHARS - len_front); /* prefix + measure + sufix*/
	}
	else if (strlen(user_txt) == 0) { /* if user text  is "", then measure will be the annotation */
		snprintf (result_str, DXF_MAX_CHARS, "%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale);
	}
	else { /* no replace mark - use the entire user text */
		strncpy(result_str, user_txt, DXF_MAX_CHARS);
	}
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){result_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	if (dim_sty.tstyle >= 0){
		dxf_attr_change(obj, 7, drawing->text_styles[dim_sty.tstyle].name);
	}
	dxf_edit_scale (obj, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines */
	dir = ((-sine*(base_pt[0]  - pt1[0])+ cosine*(base_pt[1]  - pt1[1])) > 0.0) ? 1.0 : -1.0;
	
	t1[0] = base_pt[0] - dim_sty.scale * dim_sty.ext_e * dir * sine;
	t1[1] = base_pt[1] + dim_sty.scale * dim_sty.ext_e * dir * cosine;
	t2[0] = pt1[0] - dim_sty.scale * dim_sty.ext_ofs * dir * sine;
	t2[1] = pt1[1] + dim_sty.scale * dim_sty.ext_ofs * dir * cosine;
	obj = dxf_new_line (t1[0], t1[1], base_pt[2], t2[0], t2[1], pt1[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	t1[0] = base_pt[0] - cosine * measure - dim_sty.scale * dim_sty.ext_e * dir * sine;
	t1[1] = base_pt[1] - sine * measure + dim_sty.scale * dim_sty.ext_e * dir * cosine;
	t2[0] = pt0[0] - dim_sty.scale * dim_sty.ext_ofs * dir * sine;
	t2[1] = pt0[1] + dim_sty.scale * dim_sty.ext_ofs * dir * cosine;
	obj = dxf_new_line (t1[0], t1[1], base_pt[2], t2[0], t2[1], pt0[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	
	return list;
}

list_node * dxf_dim_angular_make(dxf_drawing *drawing, dxf_node * ent){
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	dxf_node *current = NULL;
	
	
	double base_pt[3] = {0.0, 0.0, 0.0}; /* dim base point */
	double an_pt[3] = {0.0, 0.0, 0.0}; /* annotation point */
	double measure = 0.0;
	/* two lines to measure angle */
	double pt0[3] = {0.0, 0.0, 0.0};
	double pt1[3] = {0.0, 0.0, 0.0};
	double pt2[3] = {0.0, 0.0, 0.0};
	double pt3[3] = {0.0, 0.0, 0.0};
	/* center point */
	double center[3] = {0.0, 0.0, 0.0};
	
	/* points to draw terminators */
	double t1[2], t2[2], t3[2], t4[2];
	
	int flags = 32;
	int an_place = 5; /* annotation placement (5 = middle center) */
	
	int dec_places = 0; /* decimal places */

	char user_txt[DXF_MAX_CHARS+1] = "<>";
	char tmp_str[DXF_MAX_CHARS+1] = "";
	char result_str[DXF_MAX_CHARS+1] = "";
	
	dxf_dimsty dim_sty;
	strncpy(dim_sty.name, "STANDARD", DXF_MAX_CHARS);
	
	enum { FILLED, OPEN, OPEN30, OPEN90, CLOSED, OBLIQUE, ARCHTICK, NONE } term_typ = FILLED;
	
	current = ent->obj.content->next;
	
	/* get DIMENSION parameters */
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				/* dim base point */
				case 16:
					base_pt[0] = current->value.d_data;
					break;
				case 26:
					base_pt[1] = current->value.d_data;
					break;
				case 36:
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
				/* first line */
				case 15:
					pt0[0] = current->value.d_data;
					break;
				case 25:
					pt0[1] = current->value.d_data;
					break;
				case 35:
					pt0[2] = current->value.d_data;
					break;
				case 10:
					pt1[0] = current->value.d_data;
					break;
				case 20:
					pt1[1] = current->value.d_data;
					break;
				case 30:
					pt1[2] = current->value.d_data;
					break;
				/* second line */
				case 13:
					pt2[0] = current->value.d_data;
					break;
				case 23:
					pt2[1] = current->value.d_data;
					break;
				case 33:
					pt2[2] = current->value.d_data;
					break;
				case 14:
					pt3[0] = current->value.d_data;
					break;
				case 24:
					pt3[1] = current->value.d_data;
					break;
				case 34:
					pt3[2] = current->value.d_data;
					break;
				/* user text */
				case 1:
					strncpy(user_txt, current->value.s_data, DXF_MAX_CHARS);
					break;
				/* dim style */
				case 3:
					strncpy(dim_sty.name, current->value.s_data, DXF_MAX_CHARS);
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
	
	if ((flags & 7) != 2) return NULL; /* verify type of dimension - angular 2 line dimension */
	
	/* obtain the center point from 2 lines intersect */
	seg_inter2(pt0[0], pt0[1], pt1[0], pt1[1],
		pt2[0], pt2[1], pt3[0], pt3[1],
		&center[0], &center[1]);
	
	double radius = sqrt(pow((center[0] - base_pt[0]), 2) + pow((center[1] - base_pt[1]), 2));
	double start = atan2((pt1[1] - center[1]), (pt1[0] - center[0]));
	double end = atan2((pt3[1] - center[1]), (pt3[0] - center[0]));
	double dir = 1.0;
	
	list_node * list = list_new(NULL, FRAME_LIFE);
	
	/*init drawing style */
	dxf_dim_get_sty(drawing, &dim_sty);
	
	/* base arc */
	dxf_node * obj = dxf_new_arc (center[0], center[1], center[2], /* center */
		radius, start * 180/M_PI, end * 180/M_PI,/* radius, start angle, end angle */
		 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* terminators */
	//dir = (measure > 0.0) ? 1.0 : -1.0;
	strncpy(tmp_str, dim_sty.a_type, DXF_MAX_CHARS); /* preserve original string */
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
		t1[0] = center[0] + radius * cos(end);
		t1[1] = center[1] + radius * sin(end);
		t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		if (term_typ == OPEN30) {
			t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.2588) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
			t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.2588) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		else if (term_typ == OPEN90) {
			t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.7071) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
			t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.7071) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		if (term_typ == OPEN30) {
			t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.2588) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
			t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.2588) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		else if (term_typ == OPEN90) {
			t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.7071) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
			t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.7071) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		if (term_typ == CLOSED){
			obj = dxf_new_line (t2[0], t2[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
			list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		}
		
		t1[0] = center[0] + radius * cos(start);
		t1[1] = center[1] + radius * sin(start);
		t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		if (term_typ == OPEN30) {
			t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.2588) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
			t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.2588) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		else if (term_typ == OPEN90) {
			t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.7071) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
			t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.7071) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		if (term_typ == OPEN30) {
			t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.2588) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
			t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.2588) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		else if (term_typ == OPEN90) {
			t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.7071) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
			t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.7071) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		}
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		if (term_typ == CLOSED){
			obj = dxf_new_line (t2[0], t2[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
			list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		}
		
	}
	else if (term_typ == OBLIQUE || term_typ == ARCHTICK){
		int tick = -2;
		if (term_typ == ARCHTICK) tick = 50;
		
		t1[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.5) * cos(end + dim_sty.scale * dim_sty.a_size*0.5/radius);
		t1[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.5) * sin(end + dim_sty.scale * dim_sty.a_size*0.5/radius);
		t2[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.5) * cos(end - dim_sty.scale * dim_sty.a_size*0.5/radius * dir);
		t2[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.5) * sin(end - dim_sty.scale * dim_sty.a_size*0.5/radius * dir);
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", tick, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		t1[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.5) * cos(start + dim_sty.scale * dim_sty.a_size*0.5/radius);
		t1[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.5) * sin(start + dim_sty.scale * dim_sty.a_size*0.5/radius);
		t2[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.5) * cos(start - dim_sty.scale * dim_sty.a_size*0.5/radius);
		t2[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.5) * sin(start - dim_sty.scale * dim_sty.a_size*0.5/radius);
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", tick, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
	}
	else{ /* FILLED */
		t1[0] = center[0] + radius * cos(end);
		t1[1] = center[1] + radius * sin(end);
		t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * cos(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * sin(end - dim_sty.scale * dim_sty.a_size/radius * dir);
		obj = dxf_new_solid (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		t1[0] = center[0] + radius * cos(start);
		t1[1] = center[1] + radius * sin(start);
		t2[0] = center[0] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		t2[1] = center[1] + (radius+dim_sty.scale * dim_sty.a_size * 0.1667) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[0] = center[0] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * cos(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		t3[1] = center[1] + (radius-dim_sty.scale * dim_sty.a_size * 0.1667) * sin(start + dim_sty.scale * dim_sty.a_size/radius * dir);
		obj = dxf_new_solid (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, t3[0], t3[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	}
	
	/* anotation */
	char *subst = strstr(user_txt, "<>"); /* try to find string "<>"  in user text to replace with measure */
	if (subst) { /* proceed to replace */
		int len_front = subst - user_txt;
		strncpy(result_str, user_txt, len_front); /* prefix */
		snprintf (tmp_str, DXF_MAX_CHARS, "%.*f°", dec_places, fabs(measure * 180.0/M_PI)); /* measure */
		strncat(result_str, tmp_str, DXF_MAX_CHARS - len_front); /* prefix + measure */
		len_front = strlen(result_str);
		subst += 2;
		strncat(result_str, subst, DXF_MAX_CHARS - len_front); /* prefix + measure + sufix*/
	}
	else if (strlen(user_txt) == 0) { /* if user text  is "", then measure will be the annotation */
		snprintf (result_str, DXF_MAX_CHARS, "%.*f°", dec_places, fabs(measure) * dim_sty.an_scale);
	}
	else { /* no replace mark - use the entire user text */
		strncpy(result_str, user_txt, DXF_MAX_CHARS);
	}
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){result_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	if (dim_sty.tstyle >= 0){
		dxf_attr_change(obj, 7, drawing->text_styles[dim_sty.tstyle].name);
	}
	dxf_edit_scale (obj, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size);
	//dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines - to points pt1 and pt3 */
	dir = ( ((pt3[0] - (center[0] +radius*cos(end)))*cos(end) + (pt3[1] - (center[1] +radius*sin(end)))*sin(end)) < 0.0) ? 1.0 : -1.0;
	t1[0] = center[0] + (radius+dim_sty.scale * dim_sty.ext_e * dir) * cos(end);
	t1[1] = center[1] + (radius+dim_sty.scale * dim_sty.ext_e * dir) * sin(end);
	t2[0] = pt3[0] + dim_sty.scale * dim_sty.ext_ofs * dir * cos(end);
	t2[1] = pt3[1] + dim_sty.scale * dim_sty.ext_ofs * dir * sin(end);
	obj = dxf_new_line (t1[0], t1[1], pt3[2], t2[0], t2[1], pt3[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	dir = ( ((pt1[0] - (center[0] +radius*cos(start)))*cos(start) + (pt1[1] - (center[0] +radius*sin(start)))*sin(start)) < 0.0) ? 1.0 : -1.0;
	t1[0] = center[0] + (radius+dim_sty.scale * dim_sty.ext_e * dir) * cos(start);
	t1[1] = center[1] + (radius+dim_sty.scale * dim_sty.ext_e * dir) * sin(start);
	t2[0] = pt1[0] + dim_sty.scale * dim_sty.ext_ofs * dir * cos(start);
	t2[1] = pt1[1] + dim_sty.scale * dim_sty.ext_ofs * dir * sin(start);
	obj = dxf_new_line (t1[0], t1[1], pt1[2], t2[0], t2[1], pt1[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	return list;
}

list_node * dxf_dim_radial_make(dxf_drawing *drawing, dxf_node * ent){
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	dxf_node *current = NULL;
	
	
	double base_pt[3] = {0.0, 0.0, 0.0}; /* dim base point */
	double an_pt[3] = {0.0, 0.0, 0.0}; /* annotation point */
	double measure = 0.0;
	/* center or opposite point - "measure" to the base point */
	double pt0[3] = {0.0, 0.0, 0.0};
	double lead_len = 0.0; /* leader length */
	/* points to draw terminators */
	double t1[2], t2[2];
	double rot = 0.0;
	
	int flags = 32;
	int an_place = 5; /* annotation placement (5 = middle center) */

	char user_txt[DXF_MAX_CHARS+1] = "<>";
	char tmp_str[DXF_MAX_CHARS+1] = "";
	char result_str[DXF_MAX_CHARS+1] = "";
	
	dxf_dimsty dim_sty;
	strncpy(dim_sty.name, "STANDARD", DXF_MAX_CHARS);
	
	enum { FILLED, OPEN, OPEN30, OPEN90, CLOSED, OBLIQUE, ARCHTICK, NONE } term_typ = FILLED;
	
	current = ent->obj.content->next;
	
	/* get DIMENSION parameters */
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				/* dim base point */
				case 15:
					base_pt[0] = current->value.d_data;
					break;
				case 25:
					base_pt[1] = current->value.d_data;
					break;
				case 35:
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
				/* center or opposite point */
				case 10:
					pt0[0] = current->value.d_data;
					break;
				case 20:
					pt0[1] = current->value.d_data;
					break;
				case 30:
					pt0[2] = current->value.d_data;
					break;
				/* user text */
				case 1:
					strncpy(user_txt, current->value.s_data, DXF_MAX_CHARS);
					break;
				/* dim style */
				case 3:
					strncpy(dim_sty.name, current->value.s_data, DXF_MAX_CHARS);
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
				/* leader length */
				case 40:
					lead_len = current->value.d_data;
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
	
	if ((flags & 7) != 3 && (flags & 7) != 4) return NULL; /* verify type of dimension - radial and diametric dimensions */
	int diametric = ((flags & 7) == 3) ? 1 : 0;
	
	rot = atan2(an_pt[1] - base_pt[1], an_pt[0] - base_pt[0]);
	double cosine = cos(rot);
	double sine = sin(rot);
	
	rot *= 180.0/M_PI;
	
	list_node * list = list_new(NULL, FRAME_LIFE);
	
	/*init drawing style */
	dxf_dim_get_sty(drawing, &dim_sty);
	
	/* leader line */
	t1[0] = an_pt[0] - cosine * dim_sty.scale * dim_sty.gap;
	t1[1] = an_pt[1] - sine * dim_sty.scale * dim_sty.gap;
	dxf_node * obj = dxf_new_line (t1[0], t1[1], an_pt[2], base_pt[0], base_pt[1], base_pt[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* terminator */
	strncpy(tmp_str, dim_sty.a_type, DXF_MAX_CHARS); /* preserve original string */
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
		t2[0] = dim_sty.scale * dim_sty.a_size;
		t2[1] = dim_sty.scale * dim_sty.a_size * 0.1667;
		if (term_typ == OPEN30) t2[1] = dim_sty.scale * dim_sty.a_size * 0.2588;
		else if (term_typ == OPEN90) t2[1] = dim_sty.scale * dim_sty.a_size * 0.7071;
		
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
		if (term_typ == CLOSED){
			obj = dxf_new_line (t2[0], t2[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
			dxf_edit_rot (obj, rot);
			dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
			list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		}
		
	}
	else if (term_typ == OBLIQUE || term_typ == ARCHTICK){
		t1[0] = dim_sty.scale * dim_sty.a_size * 0.5;
		t1[1] = dim_sty.scale * dim_sty.a_size * 0.5;
		t2[0] = -t1[0];
		t2[1] = -t1[1];
		
		int tick = -2;
		if (term_typ == ARCHTICK) tick = 50;
		
		obj = dxf_new_line (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, 0, "0", "BYBLOCK", tick, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	}
	else{ /* FILLED */
		t1[0] = 0.0; t1[1] = 0.0;
		t2[0] = dim_sty.scale * dim_sty.a_size;
		t2[1] = dim_sty.scale * dim_sty.a_size * 0.1667;
		
		obj = dxf_new_solid (t1[0], t1[1], 0.0, t2[0], t2[1], 0.0, t2[0], -t2[1], 0.0, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
		dxf_edit_rot (obj, rot);
		dxf_edit_move (obj, base_pt[0], base_pt[1], base_pt[2]);
		list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
	}
	
	/* anotation */
	char *subst = strstr(user_txt, "<>"); /* try to find string "<>"  in user text to replace with measure */
	if (subst) { /* proceed to replace */
		int len_front = subst - user_txt;
		strncpy(result_str, user_txt, len_front); /* prefix */
		if (diametric) snprintf (tmp_str, DXF_MAX_CHARS, "%%%%c%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale); /* measure */
		else snprintf (tmp_str, DXF_MAX_CHARS, "R%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale); /* measure */
		strncat(result_str, tmp_str, DXF_MAX_CHARS - len_front); /* prefix + measure */
		len_front = strlen(result_str);
		subst += 2;
		strncat(result_str, subst, DXF_MAX_CHARS - len_front); /* prefix + measure + sufix*/
	}
	else if (strlen(user_txt) == 0) { /* if user text  is "", then measure will be the annotation */
		if (diametric) snprintf (result_str, DXF_MAX_CHARS, "%%%%c%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale);
		else snprintf (result_str, DXF_MAX_CHARS, "R%.*f", dim_sty.dec, fabs(measure) * dim_sty.an_scale);
	}
	else { /* no replace mark - use the entire user text */
		strncpy(result_str, user_txt, DXF_MAX_CHARS);
	}
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){result_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	if (dim_sty.tstyle >= 0){
		dxf_attr_change(obj, 7, drawing->text_styles[dim_sty.tstyle].name);
	}
	dxf_edit_scale (obj, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines */
	
	
	
	return list;
}

list_node * dxf_dim_ordinate_make(dxf_drawing *drawing, dxf_node * ent){
	if(!ent) return NULL;
	if (ent->type != DXF_ENT) return NULL;
	if (!ent->obj.content) return NULL;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return NULL;
	
	dxf_node *current = NULL;
	
	
	double base_pt[3] = {0.0, 0.0, 0.0}; /* dim base point */
	double an_pt[3] = {0.0, 0.0, 0.0}; /* annotation point */
	double measure = 0.0;
	/* center or opposite point - "measure" to the base point */
	double pt0[3] = {0.0, 0.0, 0.0};
	double lead_len = 0.0; /* leader length */
	/* points to draw terminators */
	double t1[2], t2[2];
	double rot = 0.0;
	
	int flags = 32;
	int an_place = 5; /* annotation placement (5 = middle center) */

	char user_txt[DXF_MAX_CHARS+1] = "<>";
	char tmp_str[DXF_MAX_CHARS+1] = "";
	char result_str[DXF_MAX_CHARS+1] = "";
	
	dxf_dimsty dim_sty;
	strncpy(dim_sty.name, "STANDARD", DXF_MAX_CHARS);
	
	enum { FILLED, OPEN, OPEN30, OPEN90, CLOSED, OBLIQUE, ARCHTICK, NONE } term_typ = FILLED;
	
	current = ent->obj.content->next;
	
	/* get DIMENSION parameters */
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				/* dim base point */
				case 14:
					base_pt[0] = current->value.d_data;
					break;
				case 24:
					base_pt[1] = current->value.d_data;
					break;
				case 34:
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
				/* center or opposite point */
				case 13:
					pt0[0] = current->value.d_data;
					break;
				case 23:
					pt0[1] = current->value.d_data;
					break;
				case 33:
					pt0[2] = current->value.d_data;
					break;
				/* user text */
				case 1:
					strncpy(user_txt, current->value.s_data, DXF_MAX_CHARS);
					break;
				/* dim style */
				case 3:
					strncpy(dim_sty.name, current->value.s_data, DXF_MAX_CHARS);
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
	
	if ((flags & 7) != 6) return NULL; /* verify type of dimension - ordinate */
	int x_dir = (flags & 64) ? 0 : 1;
	
	double dir = (((x_dir) ? base_pt[0] - pt0[0] : base_pt[1] - pt0[1]) > 0.0) ? 1.0 : -1.0;
	
	double cosine = (x_dir) ? 1.0 : 0.0;
	double sine = (x_dir) ? 0.0 : 1.0;
	
	rot = (x_dir) ? 0.0 : 90.0;
	
	list_node * list = list_new(NULL, FRAME_LIFE);
	
	/*init drawing style */
	dxf_dim_get_sty(drawing, &dim_sty);
	
	/* leader line */
	t1[0] = pt0[0] + dir * cosine * dim_sty.scale * dim_sty.ext_ofs;
	t1[1] = pt0[1] + dir * sine * dim_sty.scale * dim_sty.ext_ofs;
	dxf_node * obj = dxf_new_line (t1[0], t1[1], pt0[2], base_pt[0], base_pt[1], base_pt[2], 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
		
	/* anotation */
	char *subst = strstr(user_txt, "<>"); /* try to find string "<>"  in user text to replace with measure */
	if (subst) { /* proceed to replace */
		int len_front = subst - user_txt;
		strncpy(result_str, user_txt, len_front); /* prefix */
		snprintf (tmp_str, DXF_MAX_CHARS, "%.*f", dim_sty.dec, measure * dim_sty.an_scale); /* measure */
		strncat(result_str, tmp_str, DXF_MAX_CHARS - len_front); /* prefix + measure */
		len_front = strlen(result_str);
		subst += 2;
		strncat(result_str, subst, DXF_MAX_CHARS - len_front); /* prefix + measure + sufix*/
	}
	else if (strlen(user_txt) == 0) { /* if user text  is "", then measure will be the annotation */
		snprintf (result_str, DXF_MAX_CHARS, "%.*f", dim_sty.dec, measure * dim_sty.an_scale);
	}
	else { /* no replace mark - use the entire user text */
		strncpy(result_str, user_txt, DXF_MAX_CHARS);
	}
	obj = dxf_new_mtext (0.0, 0.0, 0.0, 1.0, (char*[]){result_str}, 1, 0, "0", "BYBLOCK", -2, 0, FRAME_LIFE);
	dxf_attr_change(obj, 71, &an_place);
	if (dim_sty.tstyle >= 0){
		dxf_attr_change(obj, 7, drawing->text_styles[dim_sty.tstyle].name);
	}
	dxf_edit_scale (obj, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size, dim_sty.scale * dim_sty.txt_size);
	dxf_edit_rot (obj, rot);
	dxf_edit_move (obj, an_pt[0], an_pt[1], an_pt[2]);
	list_push(list, list_new((void *)obj, FRAME_LIFE)); /* store entity in list */
	
	/* extension lines */
	
	
	
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

int dxf_dim_get_blk (dxf_drawing *drawing, dxf_node * ent, dxf_node **blk, dxf_node **blk_rec){
	/* verify if passed parameters are valid */
	if(!blk) return 0;
	if(!blk_rec) return 0;
	/* init returned values */
	*blk = NULL;
	*blk_rec = NULL;
	/* verify if passed parameters are valid */
	if(!ent) return 0;
	if (ent->type != DXF_ENT) return 0;
	if (!ent->obj.content) return 0;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return 0;
	
	dxf_node *blk_name = dxf_find_attr2(ent, 2); /* get block name */
	
	/* find relative block */
	if(!blk_name) return 0;
	*blk = dxf_find_obj_descr2(drawing->blks, "BLOCK", blk_name->value.s_data);
	if(!*blk) return 0;
	
	*blk_rec = dxf_find_obj_descr2(drawing->blks_rec, "BLOCK_RECORD", blk_name->value.s_data);
	return 1;
}

int dxf_dim_rewrite (dxf_drawing *drawing, dxf_node *ent, dxf_node **blk, dxf_node **blk_rec, dxf_node **blk_old, dxf_node **blk_rec_old){
	/* verify if passed parameters are valid */
	if(!blk) return 0;
	if(!blk_rec) return 0;
	/* init returned values */
	*blk = NULL;
	*blk_rec = NULL;
	*blk_old = NULL;
	*blk_rec_old = NULL;
	/* verify if passed parameters are valid */
	if(!ent) return 0;
	if (ent->type != DXF_ENT) return 0;
	if (!ent->obj.content) return 0;
	if (strcmp(ent->obj.name, "DIMENSION") != 0) return 0;
	
	/* create dimension block contents as a list of entities ("render" the dimension "picture") */
	list_node *list = dxf_dim_make(drawing, ent);
	if(!list) return 0;
	
	dxf_node *blk_name = dxf_find_attr2(ent, 2); /* get block name */
	
	/* find relative block */
	if(!blk_name) return 0;
	*blk_old = dxf_find_obj_descr2(drawing->blks, "BLOCK", blk_name->value.s_data);
	if(!*blk_old) return 0;
	dxf_obj_subst(*blk_old, NULL); /* detach block from its structure */
	
	*blk_rec_old = dxf_find_obj_descr2(drawing->blks_rec, "BLOCK_RECORD", blk_name->value.s_data);
	if(*blk_rec_old) dxf_obj_subst(*blk_rec_old, NULL); /* detach block record from its structure */
	
	char tmp_str[DXF_MAX_CHARS + 1];
	int last_dim = dxf_find_last_dim (drawing); /* get last dim number available*/
	snprintf(tmp_str, DXF_MAX_CHARS, "*D%d", last_dim); /* block name - "*D" + sequential number*/
	if (dxf_new_block (drawing, tmp_str, "",
		(double []){0.0, 0.0, 0.0},
		0, "", "", "", "",
		"0", list, blk_rec, blk, DWG_LIFE))
	{	
		dxf_attr_change(*blk, 70, (void*)(int[]){1}); /* set block to annonimous */
		/* atach block to dimension ent */
		dxf_attr_change(ent, 2, (void*)tmp_str);
		ent->obj.graphics = dxf_graph_parse(drawing, ent, 0 , DWG_LIFE);
		
		return 1;
	}
	
	return 0;
}

int dxf_dim_get_sty(dxf_drawing *drawing, dxf_dimsty *dim_sty){
	if(!dim_sty) return 0;
	if (!drawing) return 0;
	if (!drawing->t_dimst) return 0;
	
	dxf_node *current = NULL, *dsty_obj;
	
	/* init dim style with default values */
	strncpy(dim_sty->post, "<>", DXF_MAX_CHARS); /* custom text (sufix/prefix) for annotation */
	strncpy(dim_sty->a_type,"FILLED", DXF_MAX_CHARS); /* arrow type */
	dim_sty->scale = 1.0; /* global scale for render */
	dim_sty->a_size = 0.18; /* arrow size */
	dim_sty->ext_ofs = 0.0625; /* extension line offset (gap) between measure point */
	dim_sty->ext_e = 0.18; /*extension of extention line :D */
	dim_sty->txt_size = 0.18; /* annotation text size */
	dim_sty->an_scale = 1.0; /* annotation scale - apply to measure */
	dim_sty->gap = 0.0625; /* space between text and base line */
	dim_sty->dec = 4; /* number of decimal places */
	dim_sty->tstyle = dxf_tstyle_idx(drawing, "STANDARD"); /* text style (index) */
	dim_sty->obj = NULL;
	
	if (!(dsty_obj = dxf_find_obj_descr2(drawing->t_dimst, "DIMSTYLE", dim_sty->name)))
		return -1; /* not found */
	
	dim_sty->obj = dsty_obj;
	
	current = dsty_obj->obj.content->next;
	
	/* get DIMSTYLE parameters */
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 3:
					strncpy(dim_sty->post, current->value.s_data, DXF_MAX_CHARS);
					break;
				case 5:
					strncpy(dim_sty->a_type, current->value.s_data, DXF_MAX_CHARS);
					break;
				case 40:
					dim_sty->scale = current->value.d_data;
					break;
				case 41:
					dim_sty->a_size = current->value.d_data;
					break;
				case 42:
					dim_sty->ext_ofs = current->value.d_data;
					break;
				case 44:
					dim_sty->ext_e = current->value.d_data;
					break;
				case 140:
					dim_sty->txt_size = current->value.d_data;
					break;
				case 144:
					dim_sty->an_scale = current->value.d_data;
					break;
				case 147:
					dim_sty->gap = current->value.d_data;
					break;
				case 271:
					dim_sty->dec = current->value.i_data;
					break;
				case 340: {
					long id = strtol(current->value.s_data, NULL, 16); /* convert string handle to integer */
					/* look for correspondent style object */
					dxf_node *t_obj = dxf_find_handle(drawing->t_style, id);
					if (t_obj){
						int j;
						for (j=0; j < drawing->num_tstyles; j++){ /* sweep drawing's text styles */
							if (drawing->text_styles[j].obj == t_obj){ /* verify if object matchs */
								dim_sty->tstyle = j; /* get index */
								break;
							}
						}
					}
					else{
						dim_sty->tstyle = dxf_tstyle_idx(drawing, "STANDARD"); /* text style (index) */
					}
				}
					break;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	
	return 1;
}

int dxf_dim_update_sty(dxf_drawing *drawing, dxf_dimsty *dim_sty){
	if(!dim_sty) return 0;
	if (!drawing) return 0;
	if (!drawing->t_dimst) return 0;
	
	dxf_node *dsty_obj;
	
	if (!(dsty_obj = dxf_find_obj_descr2(drawing->t_dimst, "DIMSTYLE", dim_sty->name)))
		return -1; /* not found */
	
	//dim_sty->obj = dsty_obj;
	
	
	/* update DIMSTYLE parameters */
	dxf_attr_change(dsty_obj, 3, dim_sty->post);
	dxf_attr_change(dsty_obj, 5, dim_sty->a_type);
	dxf_attr_change(dsty_obj, 40, &dim_sty->scale);
	dxf_attr_change(dsty_obj, 41, &dim_sty->a_size);
	dxf_attr_change(dsty_obj, 42, &dim_sty->ext_ofs);
	dxf_attr_change(dsty_obj, 44, &dim_sty->ext_e);
	dxf_attr_change(dsty_obj, 140, &dim_sty->txt_size);
	dxf_attr_change(dsty_obj, 144, &dim_sty->an_scale);
	dxf_attr_change(dsty_obj, 147, &dim_sty->gap);
	dxf_attr_change(dsty_obj, 271, &dim_sty->dec);
	
	if (dim_sty->tstyle >=0) {
		dxf_node *tsty_obj = drawing->text_styles[dim_sty->tstyle].obj;
		tsty_obj = dxf_find_attr2(tsty_obj, 5);
		if (tsty_obj){
			dxf_attr_change(dsty_obj, 340, tsty_obj->value.s_data);
		}
	}
	
	
	return 1;
}

int dxf_dimsty_use(dxf_drawing *drawing){
	/* count dimstyle in use in drawing */
	if (!drawing) return 0;
	if (!drawing->t_dimst) return 0;
	
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[2];
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	/* init dimstyles count */
	current = drawing->t_dimst->obj.content;
	while (current){ /* sweep elements in section */
		if (current->type == DXF_ENT){
			if (strcmp(current->obj.name, "DIMSTYLE") == 0){
				/* uses DIMSTYLE's layer index to count */
				current->obj.layer= 0;
				
				/* get name of current DIMSTYLE */
				dxf_node * blk_nm = dxf_find_attr2(current, 2);
				if (blk_nm){
					char name[DXF_MAX_CHARS + 1];
					strncpy(name, blk_nm->value.s_data, DXF_MAX_CHARS);
					str_upp(name);
					/* mark used if is a system DIMSTYLE*/
					if (strcmp(name, "STANDARD") == 0) current->obj.layer= 1;
				}
			}
		}
		current = current->next; /* go to the next in the list*/
	}
	
	for (i = 0; i< 2; i++){ /* look in BLOCKS and ENTITIES sections */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section */
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				if (strcmp(current->obj.name, "DIMENSION") == 0){
					dxf_node *dsty = NULL, *dsty_name = NULL;
					dsty_name = dxf_find_attr2(current, 3);
					if(dsty_name) {
						dsty = dxf_find_obj_descr2(drawing->t_dimst, "DIMSTYLE", dsty_name->value.s_data);
						if(dsty) {
							/* uses DIMSTYLE's layer index to count */
							dsty->obj.layer++;
						}
					}
				}
				/* search also in sub elements */
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content;
					continue;
				}
			}
				
			current = current->next; /* go to the next in the list*/
			
			if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
				current = NULL;
				break;
			}

			/* ============================================================= */
			while (current == NULL){
				/* end of list sweeping */
				if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
					//printf("para\n");
					current = NULL;
					break;
				}
				/* try to back in structure hierarchy */
				prev = prev->master;
				if (prev){ /* up in structure */
					/* try to continue on previous point in structure */
					current = prev->next;
					
				}
				else{ /* stop the search if structure ends */
					current = NULL;
					break;
				}
			}
		}
	}
	
	return ok;
}