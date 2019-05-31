#include "dxf_edit.h"
#include "math.h"

int dxf_edit_move2 (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z){
	/* move the object relactive to offset distances */
	if (obj){
		dxf_node *current;
		int i, j;
		for (i = 0; i < 8; i++){ /* sweep in range of DXF points (10-17, 20-27, 30-37) */
			for (j = 0; current = dxf_find_attr_i(obj, 10 + i, j); j++){
				current->value.d_data += ofs_x;
			}
			for (j = 0; current = dxf_find_attr_i(obj, 20 + i, j); j++){
				current->value.d_data += ofs_y;
			}
			for (j = 0; current = dxf_find_attr_i(obj, 30 + i, j); j++){
				current->value.d_data += ofs_z;
			}
		}
		return 1;
	}
	return 0;
}

int dxf_edit_move (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	int ellip = 0;
	
	if (!obj) return 0;
	if (obj->type != DXF_ENT) return 0;
	
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
		ret = 1;
		prev = current;
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				if (current->value.group == 10){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 20){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 30){ 
					current->value.d_data += ofs_z;
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
				if (current->value.group == 11){ 
					if (!ellip) current->value.d_data += ofs_x;
				}
				if (current->value.group == 21){ 
					if (!ellip) current->value.d_data += ofs_y;
					ellip = 0;
				}
				if (current->value.group == 31){ 
					current->value.d_data += ofs_z;
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				if (current->value.group == 11){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 21){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 31){ 
					current->value.d_data += ofs_z;
				}
				if (current->value.group == 12){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 22){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 32){ 
					current->value.d_data += ofs_z;
				}
				if (current->value.group == 13){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 23){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 33){ 
					current->value.d_data += ofs_z;
				}
			}
			else if (ent_type == DXF_DIMENSION){
				if (current->value.group == 13){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 23){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 33){ 
					current->value.d_data += ofs_z;
				}
				if (current->value.group == 14){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 24){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 34){ 
					current->value.d_data += ofs_z;
				}
				if (current->value.group == 15){ 
					current->value.d_data += ofs_x;
				}
				if (current->value.group == 25){ 
					current->value.d_data += ofs_y;
				}
				if (current->value.group == 35){ 
					current->value.d_data += ofs_z;
				}
			}
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
	return ret;
}

int dxf_edit_scale2 (dxf_node * obj, double scale_x, double scale_y, double scale_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	if (obj){ 
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				ent_type =  dxf_ident_ent_type (obj);
				current = obj->obj.content->next;
				prev = current;
			}
		}
	}

	while (current){
		ret = 1;
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				current = current->obj.content->next;
				prev = current;
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			if ((current->value.group >= 10) && (current->value.group < 19)){ 
				current->value.d_data *= scale_x;
			}
			if ((current->value.group >= 20) && (current->value.group < 29)){ 
				current->value.d_data *= scale_y;
			}
			if ((current->value.group >= 30) && (current->value.group < 38)){ 
				current->value.d_data *= scale_z;
			}
			
			switch (ent_type){
				case DXF_CIRCLE:
					if (current->value.group == 40) { 
						current->value.d_data *= scale_x;
					}
					break;
				case DXF_TEXT:
					if (current->value.group == 40) { 
						current->value.d_data *= scale_x;
					}
					break;
				default:
					break;
			}
		}
		
		current = current->next; /* go to the next in the list */
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
				if(prev == obj){
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
	return ret;
}

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	int ellip = 0, arc = 0;
	
	if (!obj) return 0;
	if (obj->type != DXF_ENT) return 0;
	
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
		ret = 1;
		prev = current;
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				if (current->value.group == 10){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 20){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 30){ 
					current->value.d_data *= scale_z;
				}
			}
			if (ent_type == DXF_HATCH){
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 2)
						arc = 1; /* arc */
					else if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
				}
			}
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH || ent_type == DXF_ATTRIB ||
			ent_type == DXF_ELLIPSE){
				if (current->value.group == 11){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 21){ 
					current->value.d_data *= scale_y;
					ellip = 0;
				}
				if (current->value.group == 31){ 
					current->value.d_data *= scale_z;
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				if (current->value.group == 11){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 21){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 31){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 12){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 22){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 32){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 13){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 23){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 33){ 
					current->value.d_data *= scale_z;
				}
			}
			else if (ent_type == DXF_DIMENSION){
				if (current->value.group == 11){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 21){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 31){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 12){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 22){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 32){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 13){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 23){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 33){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 14){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 24){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 34){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 15){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 25){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 35){ 
					current->value.d_data *= scale_z;
				}
				if (current->value.group == 16){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 26){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 36){ 
					current->value.d_data *= scale_z;
				}
			}
			else if (ent_type == DXF_INSERT){
				if (current->value.group == 41){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 42){ 
					current->value.d_data *= scale_y;
				}
				if (current->value.group == 43){ 
					current->value.d_data *= scale_z;
				}
			}
			if (ent_type == DXF_CIRCLE || ent_type == DXF_TEXT ||
			ent_type == DXF_ATTRIB || ent_type == DXF_ARC ||
			(ent_type == DXF_HATCH && arc) || ent_type == DXF_MTEXT){
				if (current->value.group == 40) { 
					current->value.d_data *= scale_x;
					arc = 0;
				}
			}
			if (ent_type == DXF_MTEXT){
				if (current->value.group == 41){ 
					current->value.d_data *= scale_x;
				}
			}
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
	return ret;
}

int dxf_edit_rot (dxf_node * obj, double ang){
	/* rotate object and its childrens along the z axis,  relactive to given angle in degrees*/
	dxf_node *current = NULL, *x = NULL, *y = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	int ellip = 0, arc = 0;
	
	double rad = ang * M_PI/180.0;
	double cosine = cos(rad);
	double sine = sin(rad);
	double x_new, y_new;
	
	int i, j;
	
	if (!obj) return 0;
	if (obj->type != DXF_ENT) return 0;
	
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
	
	
	if (ent_type != DXF_POLYLINE){
		//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
		//	y = dxf_find_attr_i(obj, 20, i);
		for (i = 0; x = dxf_find_attr_i2(current, stop, 10, i); i++){
			y = dxf_find_attr_i2(current, stop, 20, i);
			if (y){
				x_new = x->value.d_data * cosine - y->value.d_data * sine;
				y_new = x->value.d_data * sine + y->value.d_data * cosine;
				x->value.d_data = x_new;
				y->value.d_data = y_new;
			}
		}
	}
	if (ent_type == DXF_HATCH){
		/* hatch bondary path type */
		if (current->value.group == 72){ 
			if (current->value.i_data == 2)
				arc = 1; /* arc */
			else if (current->value.i_data == 3)
				ellip = 1; /* ellipse */
		}
	}
	if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
	ent_type == DXF_HATCH || ent_type == DXF_ATTRIB ||
	ent_type == DXF_ELLIPSE || ent_type == DXF_MTEXT){
		//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
		//	y = dxf_find_attr_i(obj, 20, i);
		for (i = 0; x = dxf_find_attr_i2(current, stop, 11, i); i++){
			y = dxf_find_attr_i2(current, stop, 21, i);
			if (y){
				x_new = x->value.d_data * cosine - y->value.d_data * sine;
				y_new = x->value.d_data * sine + y->value.d_data * cosine;
				x->value.d_data = x_new;
				y->value.d_data = y_new;
			}
		}
	}
	else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
		for (j = 1; j < 4; j++){
			//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
			//	y = dxf_find_attr_i(obj, 20, i);
			for (i = 0; x = dxf_find_attr_i2(current, stop, 10 + j, i); i++){
				y = dxf_find_attr_i2(current, stop, 20 + j, i);
				if (y){
					x_new = x->value.d_data * cosine - y->value.d_data * sine;
					y_new = x->value.d_data * sine + y->value.d_data * cosine;
					x->value.d_data = x_new;
					y->value.d_data = y_new;
				}
			}
		}
	}
	else if (ent_type == DXF_DIMENSION){
		for (j = 1; j < 7; j++){
			//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
			//	y = dxf_find_attr_i(obj, 20, i);
			for (i = 0; x = dxf_find_attr_i2(current, stop, 10 + j, i); i++){
				y = dxf_find_attr_i2(current, stop, 20 + j, i);
				if (y){
					x_new = x->value.d_data * cosine - y->value.d_data * sine;
					y_new = x->value.d_data * sine + y->value.d_data * cosine;
					x->value.d_data = x_new;
					y->value.d_data = y_new;
				}
			}
		}
	}
	if (ent_type == DXF_CIRCLE || ent_type == DXF_TEXT ||
	ent_type == DXF_ATTRIB || ent_type == DXF_ARC ||
	(ent_type == DXF_HATCH && arc) || ent_type == DXF_MTEXT ||
	ent_type == DXF_INSERT){
		y = dxf_find_attr_i2(current, stop, 50, 0);
		if (y){
			y->value.d_data += ang;
		}
	}
	if (ent_type == DXF_ARC){
		y = dxf_find_attr_i2(current, stop, 51, 0);
		if (y){
			y->value.d_data += ang;
		}
	}
	
	if (ent_type == DXF_INSERT && (obj->obj.content)){
		while (current){
			if (dxf_ident_ent_type(current) == DXF_ATTRIB){
				for (j = 0; j < 2; j++){
					for (i = 0; x = dxf_find_attr_i(current, 10 + j, i); i++){
						y = dxf_find_attr_i(current, 20 + j, i);
						if (y){
							x_new = x->value.d_data * cosine - y->value.d_data * sine;
							y_new = x->value.d_data * sine + y->value.d_data * cosine;
							x->value.d_data = x_new;
							y->value.d_data = y_new;
						}
					}
				}
				y = dxf_find_attr_i(current, 50, 0);
				if (y){
					y->value.d_data += ang;
				}
			}
			current = current->next; /* go to the next in the list */
		}
	}
	
	if (ent_type == DXF_POLYLINE && (obj->obj.content)){
		while (current){
			if (dxf_ident_ent_type(current) == DXF_VERTEX){
				for (i = 0; x = dxf_find_attr_i(current, 10, i); i++){
					y = dxf_find_attr_i(current, 20, i);
					if (y){
						x_new = x->value.d_data * cosine - y->value.d_data * sine;
						y_new = x->value.d_data * sine + y->value.d_data * cosine;
						x->value.d_data = x_new;
						y->value.d_data = y_new;
					}
				}
			}
			current = current->next; /* go to the next in the list */
		}
	}
	return ret;
}