#include "dxf_ent.h"


int dxf_lwpline_get_pt(dxf_node * obj, dxf_node ** next,
double *pt1_x, double *pt1_y, double *pt1_z, double *bulge){
	
	dxf_node *current = NULL;
	static dxf_node *last = NULL;
	static double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
	
	/*flags*/
	int first = 0, pt1 = 0, ok = 0;
	static int pline_flag = 0, closed =0, init = 0;
	
	double px = 0.0, py = 0.0, pz = 0.0, bul = 0.0;
	static double last_x, last_y, last_z, curr_x, elev;
	
	double pt[3]; /*return values */
	
	if (*next == NULL){ /* parse object first time */
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
		if(obj){
			if (obj->type == DXF_ENT){
				if (obj->obj.content){
					current = obj->obj.content->next;
					//printf("%s\n", obj->obj.name);
				}
			}
		}
		/* get general parameters of polyline */
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						px = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						py = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 30:
						pz = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 42:
						bul = current->value.d_data;
						break;
					case 70:
						pline_flag = current->value.i_data;
						break;
					case 38:
						elev = current->value.d_data;
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
				}
			}
			if (pt1){
				pt1 = 0;
				if (init == 0){
					init = 1;
					curr_x = px;
					last = current;
				}
				else{
					*next = current;
					break;
				}
			}
			current = current->next; /* go to the next in the list */
			//*next = current->next;
		}
		if (init){
			if (pline_flag & 1){
				closed = 1;
				/* if closed, the last vertex is first */
				last_x = curr_x;
				last_y = py;
				last_z = pz;
			}
			else {
				closed = 0;
				last = NULL;
			}
			
			/* to convert OCS to WCS */
			normal[0] = extru_x;
			normal[1] = extru_y;
			normal[2] = extru_z;			
			
			pt[0] = curr_x;
			pt[1] = py;
			pt[2] = pz;
			*bulge = bul;
			ok = 1;
			
		}
	}
	else if ((init) && (*next != last)){ /* continue search in next point */
		current = *next;
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				switch (current->value.group){
					case 10:
						px = current->value.d_data;
						pt1 = 1; /* set flag */
						break;
					case 20:
						py = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 30:
						pz = current->value.d_data;
						//pt1 = 1; /* set flag */
						break;
					case 42:
						bul = current->value.d_data;
						break;
				}
			}
			if (pt1){
				pt1 = 0;
				if (first == 0){
					first = 1;
					curr_x = px;
				}
				else{
					*next = current;
					break;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (first){
			
			pt[0] = curr_x;
			pt[1] = py;
			pt[2] = pz;
			*bulge = bul;
			ok = 1;
			if (current == NULL){
				if (closed){
					*next = last;
				}
				else {
					*next = NULL;
					pline_flag = 0; closed =0; init = 0;
					last = NULL;
				}
			}
		}
		else if (closed){
			*next = last;
		}
		else {
			*next = NULL;
			pline_flag = 0; closed =0; init = 0;
			last = NULL;
		}
	}
	else { /* last vertex */
		pt[0] = last_x;
		pt[1] = last_y;
		pt[2] = last_z;
		*bulge = 0.0;
		ok = 1;
		*next = NULL;
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
	}
	
	if (ok == 0){
		*next = NULL;
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
	}
	/* convert OCS to WCS */
	else {
		mod_axis(pt, normal , 0.0);
		/* update return values */
		*pt1_x = pt[0];
		*pt1_y = pt[1];
		*pt1_z = pt[2];
	}
	
	return ok;
}

int dxf_pline_get_pt(dxf_node * obj, dxf_node ** next,
double *pt1_x, double *pt1_y, double *pt1_z, double *bulge){
	
	dxf_node *current = NULL;
	static dxf_node *last = NULL;
	static double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
	
	/*flags*/
	int first = 0, pt1 = 0, ok = 0;
	static int pline_flag = 0, closed =0, init = 0;
	
	double px = 0.0, py = 0.0, pz = 0.0, bul = 0.0;
	static double last_x, last_y, last_z, elev;
	
	double pt[3]; /*return values */
	
	if (*next == NULL){ /* parse object first time */
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
		if(obj){
			if (obj->type == DXF_ENT){
				if (obj->obj.content){
					current = obj->obj.content->next;
				}
			}
		}
		/* get general parameters of polyline */
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 70:
						pline_flag = current->value.i_data;
						break;
					case 38:
						elev = current->value.d_data;
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
				}
			}
			else if (current->type == DXF_ENT){
				if (strcmp(current->obj.name, "VERTEX") == 0 ){
					pt1 = 1; /* found vertex */
				}
			}
			if (pt1){
				pt1 = 0;
				if (init == 0){
					/* get current vertex parameters */
					dxf_node *curr_vtx = current->obj.content;
					while(curr_vtx){
						if (curr_vtx->type == DXF_ATTR){ 
							switch (curr_vtx->value.group){
								case 10:
									px = curr_vtx->value.d_data;
									break;
								case 20:
									py = curr_vtx->value.d_data;
									break;
								case 30:
									pz = curr_vtx->value.d_data;
									break;
								case 42:
									bul = curr_vtx->value.d_data;
									break;
							}
						}
						curr_vtx = curr_vtx->next;
					}
					
					init = 1;
					last = current;
				}
				else{
					*next = current;
					break;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (init){
			if (pline_flag & 1){
				closed = 1;
				/* if closed, the last vertex is first */
				last_x = px;
				last_y = py;
				last_z = pz;
			}
			else {
				closed = 0;
				last = NULL;
			}
			
			/* to convert OCS to WCS */
			normal[0] = extru_x;
			normal[1] = extru_y;
			normal[2] = extru_z;			
			
			pt[0] = px;
			pt[1] = py;
			pt[2] = pz;
			*bulge = bul;
			ok = 1;
			
		}
	}
	else if ((init) && (*next != last)){ /* continue search in next point */
		dxf_node *curr_vtx = NULL;
		current = *next;
		if (*next) curr_vtx = current->obj.content;
		while(curr_vtx){
			if (curr_vtx->type == DXF_ATTR){ /* DXF attibute */
				switch (curr_vtx->value.group){
					case 10:
						px = curr_vtx->value.d_data;
						break;
					case 20:
						py = curr_vtx->value.d_data;
						break;
					case 30:
						pz = curr_vtx->value.d_data;
						break;
					case 42:
						bul = curr_vtx->value.d_data;
						break;
				}
			}
			curr_vtx = curr_vtx->next;
		}
		
		/* get next vertex */
		*next = NULL;
		if (current->next){
			if (current->next->type == DXF_ENT){
				if (strcmp(current->next->obj.name, "VERTEX") == 0 ){
					*next = current->next;
				}
			}
		}
		
		pt[0] = px;
		pt[1] = py;
		pt[2] = pz;
		*bulge = bul;
		ok = 1;
		if (*next == NULL){
			if (closed){
				*next = last;
			}
			else {
				*next = NULL;
				pline_flag = 0; closed =0; init = 0;
				last = NULL;
			}
		}
	}
	else { /* last vertex */
		pt[0] = last_x;
		pt[1] = last_y;
		pt[2] = last_z;
		*bulge = 0.0;
		ok = 1;
		*next = NULL;
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
	}
	
	if (ok == 0){
		*next = NULL;
		pline_flag = 0; closed =0; init = 0;
		last = NULL;
	}
	/* convert OCS to WCS */
	else {
		mod_axis(pt, normal , 0.0);
		/* update return values */
		*pt1_x = pt[0];
		*pt1_y = pt[1];
		*pt1_z = pt[2];
	}
	
	return ok;
}

list_node *  gui_dwg_sel_filter(dxf_drawing *drawing, enum dxf_graph filter, int pool_idx){
	/* scan the drawing and return a list of entities according selection filter, only visible ents */
	dxf_node *current = NULL;
	list_node *list = NULL;
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)){
		/* fail */
		return NULL;
	}
	
	/* init the serch */
	current = drawing->ents->obj.content->next; /* or from begin */
	
	/* sweep entities section */
	while (current != NULL){
		if (current->type == DXF_ENT){ /* look for DXF entity */
			enum dxf_graph ent_type = dxf_ident_ent_type (current);
			
			/*verify if entity layer is on and thaw */
			if ((!drawing->layers[current->obj.layer].off) && 
				(!drawing->layers[current->obj.layer].frozen) ){
				/* and if  is a compatible entity */
				if (ent_type & filter){
					if (!list) list = list_new(NULL, pool_idx);
					/* append to list*/
					list_node * new_node = list_new(current, pool_idx);
					list_push(list, new_node);
				}
				else if (ent_type == DXF_INSERT && (filter & DXF_ATTRIB) ){
					int num_attr = 0;
					dxf_node *attr = NULL;
					
					num_attr = 0;
					while (attr = dxf_find_obj_i(current, "ATTRIB", num_attr)){
						/* add attribute entity to list */
						if (!list) list = list_new(NULL, pool_idx);
						/* append to list*/
						list_node * new_node = list_new(attr, pool_idx);
						list_push(list, new_node);
						
						num_attr++;
					}
				}
				
			}
		}
		current = current->next;
	}
	
	return list;
}

int dxf_layer_get(dxf_drawing *drawing, dxf_node * ent){
	/* Return the layer index of drawing's layer vector */
	/*if search fails, return -1 */
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)) 
		return -1;
	if (!ent) return -1;
	
	int ok = -1;
	char layer[DXF_MAX_CHARS + 1] = "0"; /* default layer is "0" */
	dxf_node *tmp = NULL;
	/* try to find entity's layer information */
	if(tmp = dxf_find_attr2(ent, 8))
		strncpy (layer, tmp->value.s_data, DXF_MAX_CHARS);
	/* try to find layer index */
	if (strlen(layer) > 0) ok = dxf_lay_idx (drawing, layer);
	else ok = dxf_lay_idx (drawing, "0");
	
	return ok;
}

int dxf_ltype_get(dxf_drawing *drawing, dxf_node * ent){
	/* Return the line type index of drawing's ltype vector */
	/*if search fails, return -1 */
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)) 
		return -1;
	if (!ent) return -1;
	
	int ok = -1;
	char ltyp[DXF_MAX_CHARS + 1] = "ByLayer"; /* default line type is "ByLayer" */
	dxf_node *tmp = NULL;
	/* try to find entity's line type information */
	if(tmp = dxf_find_attr2(ent, 6))
		strncpy (ltyp, tmp->value.s_data, DXF_MAX_CHARS);
	/* try to find line type index */
	if (strlen(ltyp) > 0) ok = dxf_ltype_idx (drawing, ltyp);
	else ok = dxf_ltype_idx  (drawing, "ByLayer");
	
	return ok;
}

int dxf_tstyle_get(dxf_drawing *drawing, dxf_node * ent){
	/* Return the text style index of drawing's vector */
	/*if search fails, return -1 */
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)) 
		return -1;
	if (!ent) return -1;
	
	int ok = -1;
	char tsty[DXF_MAX_CHARS + 1] = "Standard"; /* default style is "Standard" */
	dxf_node *tmp = NULL;
	/* try to find entity's style information */
	if(tmp = dxf_find_attr2(ent, 7))
		strncpy (tsty, tmp->value.s_data, DXF_MAX_CHARS);
	/* try to find style index */
	if (strlen(tsty) > 0) ok = dxf_tstyle_idx (drawing, tsty);
	else ok = dxf_tstyle_idx (drawing, "Standard");
	
	return ok;
}

int dxf_get_near_vert(dxf_node *obj, double pt_x, double pt_y, double clearance){
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int pt = 0;
	enum dxf_graph ent_type = DXF_NONE;
	double x = 0.0, y = 0.0, z = 0.0;
	double point[3];
	
	int ellip = 0;
	
	if (!obj) return -1;
	if (obj->type != DXF_ENT) return -1;
	
	int vert_count = 0;
	double dist = 0.0;
	
	//bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	
	/*
	point[0] = of_x;
	point[1] = of_y;
	point[2] = of_z;
	
	dxf_get_extru(obj, point);
	
	ofs_x = point[0];
	ofs_y = point[1];
	ofs_z = point[2];
	*/
	
	stop = obj;
	ent_type =  dxf_ident_ent_type (obj);
	
	if ((ent_type != DXF_HATCH) && (obj->obj.content)){
		current = obj->obj.content->next;
		prev = current;
	}
	else if ((ent_type == DXF_HATCH) && (obj->obj.content)){
		current = dxf_find_attr_i(obj, 91, 0);
		if (current){
			current = current->next;
			prev = current;
		}
		dxf_node *end_bond = dxf_find_attr_i(obj, 75, 0);
		if (end_bond) stop = end_bond;
	}
	
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/*
			point[0] = of_x;
			point[1] = of_y;
			point[2] = of_z;
			
			dxf_get_extru(obj, point);
			
			ofs_x = point[0];
			ofs_y = point[1];
			ofs_z = point[2];
			*/
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				/* get the vertex coordinate set */
				if (current->value.group == 10){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 20))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 30))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			if (ent_type == DXF_HATCH){
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
				}
			}
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH || ent_type == DXF_ATTRIB){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 12){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 22))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 32))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_DIMENSION){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 14){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 24))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 34))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 15){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 25))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 35))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				/* get the vertex coordinate set */
				if (current->value.group == 16){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 26))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 36))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
		}
		if (pt){
			pt = 0;
			
			dist = sqrt( pow(x - pt_x, 2) + pow(y - pt_y, 2) );
			
			if (dist < clearance) return vert_count;
			
			vert_count++;
		}
		
		if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
			current = NULL;
			break;
		}
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				if(prev == stop){
					current = NULL;
					break;
				}
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	
	return -1;
}

int dxf_get_vert_idx(dxf_node *obj, int idx, dxf_node ** vert_x, dxf_node ** vert_y, dxf_node ** vert_z, dxf_node ** vert_b){
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int pt = 0;
	enum dxf_graph ent_type = DXF_NONE;
	dxf_node *x = NULL, *y = NULL, *z = NULL, *bulge = NULL;
	double point[3];
	
	int ellip = 0;
	
	if (!obj) return 0;
	if (!vert_x) return 0; if (!vert_y) return 0; if (!vert_z) return 0;
	if (obj->type != DXF_ENT) return 0;
	
	int vert_count = 0;
	double dist = 0.0;
	
	*vert_x = NULL;
	*vert_y = NULL;
	*vert_z = NULL;
	*vert_b = NULL;
	
	//bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	
	/*
	point[0] = of_x;
	point[1] = of_y;
	point[2] = of_z;
	
	dxf_get_extru(obj, point);
	
	ofs_x = point[0];
	ofs_y = point[1];
	ofs_z = point[2];
	*/
	
	stop = obj;
	ent_type =  dxf_ident_ent_type (obj);
	
	if ((ent_type != DXF_HATCH) && (obj->obj.content)){
		current = obj->obj.content->next;
		prev = current;
	}
	else if ((ent_type == DXF_HATCH) && (obj->obj.content)){
		current = dxf_find_attr_i(obj, 91, 0);
		if (current){
			current = current->next;
			prev = current;
		}
		dxf_node *end_bond = dxf_find_attr_i(obj, 75, 0);
		if (end_bond) stop = end_bond;
	}
	
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/*
			point[0] = of_x;
			point[1] = of_y;
			point[2] = of_z;
			
			dxf_get_extru(obj, point);
			
			ofs_x = point[0];
			ofs_y = point[1];
			ofs_z = point[2];
			*/
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				/* get the vertex coordinate set */
				if (current->value.group == 10){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 20))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 30))
						{
							current = current->next; /* update position in list */
							z = current;
						}
						
						/* get bulge - optional */
						bulge = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 42))
						{
							current = current->next; /* update position in list */
							bulge = current;
						}
					}
				}
			}
			if (ent_type == DXF_HATCH){
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
				}
			}
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH || ent_type == DXF_ATTRIB){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 12){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 22))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 32))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
			}
			else if (ent_type == DXF_DIMENSION){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 14){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 24))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 34))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 15){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 25))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 35))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
				
				/* get the vertex coordinate set */
				if (current->value.group == 16){ /* x coordinate - start set */
					x = current;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 26))
					{
						current = current->next; /* update position in list */
						y = current;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = NULL;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 36))
						{
							current = current->next; /* update position in list */
							z = current;
						}
					}
				}
			}
		}
		if (pt){
			pt = 0;
			
			if( vert_count == idx){
				*vert_x = x;
				*vert_y = y;
				*vert_z = z;
				*vert_b = bulge;
				
				return 1;
			}
			
			vert_count++;
		}
		
		if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
			current = NULL;
			break;
		}
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				if(prev == stop){
					current = NULL;
					break;
				}
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	
	return 0;
}