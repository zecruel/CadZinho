#include "dxf_edit.h"

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
	
	int i, j, ellip = 0;
	
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
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				current = current->obj.content->next;
				ent_type =  dxf_ident_ent_type (obj);
				prev = current;
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
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH){
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
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
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

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z){
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