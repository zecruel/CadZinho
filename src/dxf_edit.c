#include "dxf_edit.h"
#include "dxf_copy.h"
#include "dxf_create.h"
#include "math.h"

double ellip_par (double ang, double a, double b){
	/* find the polar parameter (t) for ellipse */
	double t = atan(a*tan(ang)/b);
	if ((ang > M_PI/2) && (ang < M_PI)){
		t += M_PI; 
	}
	else if ((ang >= M_PI) && (ang <= 3*M_PI/2)){
		t -= M_PI; 
	}
	if (t < 0 ) t += 2*M_PI;
	
	return t;
}

//Normalize to [0,360):

double ang_adjust_360(double x){
    x = fmod(x,360.0);
    if (x < 0)
        x += 360.0;
    return x;
}
/*Normalize to [-180,180):

double constrainAngle(double x){
    x = fmod(x + 180,360);
    if (x < 0)
        x += 360;
    return x - 180;
}*/


void mod_axis(double result[3], double normal[3] , double elev){
	
	double x_axis[3], y_axis[3], point[3], x_col[3], y_col[3], z_col[3];
	double wy_axis[3] = {0.0, 1.0, 0.0};
	double wz_axis[3] = {0.0, 0.0, 1.0};
	
	
	double x0, y0, z0;
	
	if ((fabs(normal[0] < 0.015625)) && (fabs(normal[1] < 0.015625))){
		cross_product(wy_axis, normal, x_axis);
	}
	else{
		cross_product(wz_axis, normal, x_axis);
	}
	cross_product(normal, x_axis, y_axis);
	
	unit_vector(x_axis);
	unit_vector(y_axis);
	unit_vector(normal);
	
	x_col[0] = x_axis[0];
	x_col[1] = y_axis[0];
	x_col[2] = normal[0];
	
	y_col[0] = x_axis[1];
	y_col[1] = y_axis[1];
	y_col[2] = normal[1];
	
	z_col[0] = x_axis[2];
	z_col[1] = y_axis[2];
	z_col[2] = normal[2];
	
	point[0] = result[0];
	point[1] = result[1];
	point[2] = result[2];
	if (fabs(point[2]) < 1e-9) point[2] = elev;
	x0 = dot_product(point, x_col);
	y0 = dot_product(point, y_col);
	z0 = dot_product(point, z_col);
	
	result[0] = x0;
	result[1] = y0;
	result[2] = z0;
	
}

void dxf_get_extru(dxf_node * ent, double result[3]){
	if(!ent) return;
	dxf_node *current = NULL;
	double normal[3];
	double elev = 0.0;
	
	normal[0] = 0.0;
	normal[1] = 0.0;
	normal[2] = 1.0;
	
	if (ent->type == DXF_ENT){
		if (ent->obj.content){
			current = ent->obj.content->next;
		}
	}
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 38:
					elev = current->value.d_data;
					break;
				case 210:
					normal[0] = current->value.d_data;
					break;
				case 220:
					normal[1] = current->value.d_data;
					break;
				case 230:
					normal[2] = current->value.d_data;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	mod_axis(result, normal , elev);
}


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

int dxf_edit_move (dxf_node * obj, double of_x, double of_y, double of_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	double ofs_x = 0.0, ofs_y = 0.0, ofs_z = 0.0;
	double point[3];
	
	int ellip = 0;
	
	if (!obj) return 0;
	if (obj->type != DXF_ENT) return 0;
	
	point[0] = of_x;
	point[1] = of_y;
	point[2] = of_z;
	
	dxf_get_extru(obj, point);
	
	ofs_x = point[0];
	ofs_y = point[1];
	ofs_z = point[2];
	
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
			
			point[0] = of_x;
			point[1] = of_y;
			point[2] = of_z;
			
			dxf_get_extru(obj, point);
			
			ofs_x = point[0];
			ofs_y = point[1];
			ofs_z = point[2];
			
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
	
	if ((ent_type == DXF_INSERT) && (obj->obj.content)){
		dxf_node *scale_attr = dxf_find_attr_i(obj, 41, 0);
		if (!scale_attr){
			dxf_node *last_attr = dxf_find_attr_i(obj, 30, 0);
			last_attr = dxf_attr_insert_after(last_attr, 41, (void *) (double[]){1.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 42, (void *) (double[]){1.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 43, (void *) (double[]){1.0}, obj->obj.pool);
		}
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
			else if (ent_type == DXF_IMAGE){
				/* width axis*/
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
				
				/* height axis*/
				if (current->value.group == 12){ 
					current->value.d_data *= scale_x;
				}
				if (current->value.group == 22){ 
					current->value.d_data *= scale_y;
					ellip = 0;
				}
				if (current->value.group == 33){ 
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
	
	
	if ((ent_type == DXF_INSERT) && (obj->obj.content)){
		dxf_node *rot_attr = dxf_find_attr_i(obj, 50, 0);
		if (!rot_attr){
			dxf_node *scale_attr = dxf_find_attr_i(obj, 41, 0);
			if (!scale_attr){
				dxf_node *last_attr = dxf_find_attr_i(obj, 30, 0);
				last_attr = dxf_attr_insert_after(last_attr, 41, (void *) (double[]){1.0}, obj->obj.pool);
				last_attr = dxf_attr_insert_after(last_attr, 42, (void *) (double[]){1.0}, obj->obj.pool);
				last_attr = dxf_attr_insert_after(last_attr, 43, (void *) (double[]){1.0}, obj->obj.pool);
			}
			scale_attr = dxf_find_attr_i(obj, 43, 0);
			dxf_attr_insert_after(scale_attr, 50, (void *) (double[]){0.0}, obj->obj.pool);
		}
	}
	
	if ((ent_type == DXF_MTEXT) && (obj->obj.content)){
		dxf_node *rot_attr = dxf_find_attr_i(obj, 50, 0);
		if (!rot_attr) rot_attr = dxf_find_attr_i(obj, 11, 0);
		if (!rot_attr){
			dxf_node *last_attr = dxf_find_attr_i(obj, 230, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 7, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 3, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 1, 0);
			last_attr = dxf_attr_insert_after(last_attr, 11, (void *) (double[]){1.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 21, (void *) (double[]){0.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 31, (void *) (double[]){0.0}, obj->obj.pool);
		}
	}
	
	if (ent_type == DXF_TEXT || ent_type == DXF_ATTRIB){
		dxf_node *rot_attr = dxf_find_attr_i(obj, 50, 0);
		if (!rot_attr){
			dxf_node *last_attr = dxf_find_attr_i(obj, 1, 0);
			dxf_attr_insert_after(last_attr, 50, (void *) (double[]){0.0}, obj->obj.pool);
		}
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
	else if (ent_type == DXF_IMAGE){
		x = dxf_find_attr_i2(current, stop, 11, 0);
		y = dxf_find_attr_i2(current, stop, 21, 0);
		if (x && y){
			x_new = x->value.d_data * cosine - y->value.d_data * sine;
			y_new = x->value.d_data * sine + y->value.d_data * cosine;
			x->value.d_data = x_new;
			y->value.d_data = y_new;
		}
		
		x = dxf_find_attr_i2(current, stop, 12, 0);
		y = dxf_find_attr_i2(current, stop, 22, 0);
		if (x && y){
			x_new = x->value.d_data * cosine - y->value.d_data * sine;
			y_new = x->value.d_data * sine + y->value.d_data * cosine;
			x->value.d_data = x_new;
			y->value.d_data = y_new;
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
				dxf_node *rot_attr = dxf_find_attr_i(current, 50, 0);
				if (!rot_attr){
					dxf_node *last_attr = dxf_find_attr_i(current, 1, 0);
					rot_attr = dxf_attr_insert_after(last_attr, 50, (void *) (double[]){0.0}, current->obj.pool);
				}
				if (rot_attr){
					rot_attr->value.d_data += ang;
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

int dxf_edit_mirror (dxf_node * obj, double x0, double y0, double x1, double y1){
	/* mirror the object and its childrens,  relactive to a line given to (x0,y0)-(x1,y1) */
	dxf_node *current = NULL, *x = NULL, *y = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	
	
	/* reflection line parameters*/
	double dx = x1 - x0;
	double dy = y1 - y0;
	double modulus = sqrt(dx*dx + dy*dy);
	
	if (modulus < 1e-9) return 0;
	
	double rad = atan2(-dx, dy)*2; /* angle of normal reflection line */
	
	if (rad > M_PI) rad -= 2 * M_PI;
	if (rad < -M_PI) rad += 2 * M_PI;
	
	double ang = rad * 180/M_PI;
	double x_new, y_new;
	
	double dist = 0;
	
	int i, j;
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
	
	if ((ent_type == DXF_INSERT) && (obj->obj.content)){
		dxf_node *rot_attr = dxf_find_attr_i(obj, 50, 0);
		if (!rot_attr){
			dxf_node *scale_attr = dxf_find_attr_i(obj, 41, 0);
			if (!scale_attr){
				dxf_node *last_attr = dxf_find_attr_i(obj, 30, 0);
				last_attr = dxf_attr_insert_after(last_attr, 41, (void *) (double[]){1.0}, obj->obj.pool);
				last_attr = dxf_attr_insert_after(last_attr, 42, (void *) (double[]){1.0}, obj->obj.pool);
				last_attr = dxf_attr_insert_after(last_attr, 43, (void *) (double[]){1.0}, obj->obj.pool);
			}
			scale_attr = dxf_find_attr_i(obj, 43, 0);
			dxf_attr_insert_after(scale_attr, 50, (void *) (double[]){0.0}, obj->obj.pool);
		}
	}
	
	if ((ent_type == DXF_MTEXT) && (obj->obj.content)){
		dxf_node *rot_attr = dxf_find_attr_i(obj, 50, 0);
		if (!rot_attr) rot_attr = dxf_find_attr_i(obj, 11, 0);
		if (!rot_attr){
			dxf_node *last_attr = dxf_find_attr_i(obj, 230, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 7, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 3, 0);
			if (!last_attr) last_attr = dxf_find_attr_i(obj, 1, 0);
			last_attr = dxf_attr_insert_after(last_attr, 11, (void *) (double[]){1.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 21, (void *) (double[]){0.0}, obj->obj.pool);
			last_attr = dxf_attr_insert_after(last_attr, 31, (void *) (double[]){0.0}, obj->obj.pool);
		}
	}
	
	if (ent_type != DXF_POLYLINE){
		for (i = 0; x = dxf_find_attr_i2(current, stop, 10, i); i++){
			y = dxf_find_attr_i2(current, stop, 20, i);
			if (y){
				/* calcule distance between point and reflection line */
				dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
				
				x_new = x->value.d_data + 2 * dist * (dy/modulus);
				y_new = y->value.d_data + 2 * dist * (-dx/modulus);
				x->value.d_data = x_new;
				y->value.d_data = y_new;
			}
		}
	}
	if (ent_type == DXF_LWPOLYLINE){
		for (i = 0; x = dxf_find_attr_i2(current, stop, 42, i); i++){
			x->value.d_data *= -1;
		}
	}
	
	if (ent_type == DXF_HATCH && (obj->obj.content)){
		int bound_type = 0;
		int poly = 0;
		
		double el_ang = 0.0, el_ang2 = 0.0;
		
		dxf_node *p1x = NULL, *p1y = NULL, 
			*p2x = NULL, *p2y = NULL,
			*r = NULL, *b = NULL,
			*sa = NULL, *ea = NULL;
		
		while (current && current != stop){
			if (current->value.group == 92){
				poly = current->value.i_data & 2;
			}
			else if (current->value.group == 72){
				if (!poly) bound_type  = current->value.i_data;
			}
			else if (current->value.group == 11){
				p2x = current;
			}
			else if (current->value.group == 21){
				p2y = current;
				if (p2x){
					el_ang = atan2(p2y->value.d_data, p2x->value.d_data) * 180/M_PI;
					el_ang = ang_adjust_360(el_ang);
					
					
					/* calcule distance between point and reflection line */
					if (bound_type == 3)
						dist = (-dy*p2x->value.d_data + dx*p2y->value.d_data)/modulus;
					else
						dist = (-dy*p2x->value.d_data + dx*p2y->value.d_data + dy*x1 - dx*y1)/modulus;
					
					x_new = p2x->value.d_data + 2 * dist * (dy/modulus);
					y_new = p2y->value.d_data + 2 * dist * (-dx/modulus);
					p2x->value.d_data = x_new;
					p2y->value.d_data = y_new;
					
					el_ang2 = atan2(p2y->value.d_data, p2x->value.d_data) * 180/M_PI;
					el_ang2 = ang_adjust_360(el_ang2);
				}
				p2x = NULL;
				p2y = NULL;
			}
			else if (current->value.group == 40){
				r = current;
			}
			else if (current->value.group == 42){
				b = current;
			}
			else if (current->value.group == 50){
				sa = current;
			}
			else if (current->value.group == 51){
				ea = current;
				if (sa){
					if (bound_type == 3){
						
						double angle = atan2(dy, dx) * 180/M_PI;
						angle = ang_adjust_360(angle);
						
						double begin = -(sa->value.d_data + el_ang - angle) + angle - el_ang2;
						begin = ang_adjust_360(begin);
						double end = -(ea->value.d_data + el_ang - angle) + angle - el_ang2;
						end = ang_adjust_360(end);
						sa->value.d_data = end;
						ea->value.d_data = begin;
					}
					else {
						double angle = atan2(dy, dx) * 180/M_PI;
						angle = ang_adjust_360(angle);
						
						double begin = -(sa->value.d_data - angle) + angle;
						begin = ang_adjust_360(begin);
						double end = -(ea->value.d_data - angle) + angle;
						end = ang_adjust_360(end);
						sa->value.d_data = end;
						ea->value.d_data = begin;
					}
					
					
				}
				sa = NULL;
				ea = NULL;
			}
			
			current = current->next; /* go to the next in the list */
		}
	}
	if (ent_type == DXF_LINE || ent_type == DXF_TEXT){
	//||(ent_type == DXF_HATCH && !ellip)){
		//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
		//	y = dxf_find_attr_i(obj, 20, i);
		for (i = 0; x = dxf_find_attr_i2(current, stop, 11, i); i++){
			y = dxf_find_attr_i2(current, stop, 21, i);
			if (y){
				/* calcule distance between point and reflection line */
				dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
				
				x_new = x->value.d_data + 2 * dist * (dy/modulus);
				y_new = y->value.d_data + 2 * dist * (-dx/modulus);
				x->value.d_data = x_new;
				y->value.d_data = y_new;
			}
		}
	}
	if (ent_type == DXF_ELLIPSE){
		
		/* reflect main vector */
		x = dxf_find_attr_i2(current, stop, 11, 0);
		y = dxf_find_attr_i2(current, stop, 21, 0);
		if (x && y){
			/* calcule distance between point and reflection line */
			dist = (-dy*x->value.d_data + dx*y->value.d_data)/modulus;
			
			x_new = x->value.d_data + 2 * dist * (dy/modulus);
			y_new = y->value.d_data + 2 * dist * (-dx/modulus);
			x->value.d_data = x_new;
			y->value.d_data = y_new;
		}
		
		/* adjust the angles */
		x = dxf_find_attr_i2(current, stop, 41, 0);
		y = dxf_find_attr_i2(current, stop, 42, 0);
		if (x && y){
			double begin = 2*M_PI - x->value.d_data;
			double end = 2*M_PI -y->value.d_data;
			
			x->value.d_data = end;
			y->value.d_data = begin;
		}
	}
	if (ent_type == DXF_IMAGE){
		
		/* reflect width vector */
		x = dxf_find_attr_i2(current, stop, 11, 0);
		y = dxf_find_attr_i2(current, stop, 21, 0);
		if (x && y){
			/* calcule distance between point and reflection line */
			dist = (-dy*x->value.d_data + dx*y->value.d_data)/modulus;
			
			x_new = x->value.d_data + 2 * dist * (dy/modulus);
			y_new = y->value.d_data + 2 * dist * (-dx/modulus);
			x->value.d_data = x_new;
			y->value.d_data = y_new;
		}
		
		/* reflect height vector */
		x = dxf_find_attr_i2(current, stop, 12, 0);
		y = dxf_find_attr_i2(current, stop, 22, 0);
		if (x && y){
			/* calcule distance between point and reflection line */
			dist = (-dy*x->value.d_data + dx*y->value.d_data)/modulus;
			
			x_new = x->value.d_data + 2 * dist * (dy/modulus);
			y_new = y->value.d_data + 2 * dist * (-dx/modulus);
			x->value.d_data = x_new;
			y->value.d_data = y_new;
		}
	}
	if (ent_type == DXF_MTEXT){
		x = dxf_find_attr_i2(current, stop, 71, 0);
		if (x){
			int t_alin = x->value.i_data;
			int t_alin_v[10] = {0, 3, 3, 3, 2, 2, 2, 1, 1, 1};
			int t_alin_h[10] = {0, 0, 1, 2, 0, 1, 2, 0, 1, 2};
			int t_al_v = t_alin_v[t_alin], t_al_h = t_alin_h[t_alin];
			
			if(fabs(rad) < M_PI/2) t_al_h = 2 - t_al_h;
			else	t_al_v = 4 - t_al_v;
			
			
			x->value.i_data = (3 - t_al_v) * 3 + t_al_h +1;
		}
		
		for (i = 0; x = dxf_find_attr_i2(current, stop, 11, i); i++){
			y = dxf_find_attr_i2(current, stop, 21, i);
			if (y){
				double angle = rad;
				if(fabs(rad) > M_PI/2){
					angle -= M_PI;
				}
				double cosine = cos(angle);
				double sine = sin(angle);
				x_new = x->value.d_data * cosine - y->value.d_data * sine;
				y_new = x->value.d_data * sine + y->value.d_data * cosine;
				x->value.d_data = x_new;
				y->value.d_data = y_new;
			}
		}
	}
	if (ent_type == DXF_TEXT ){//|| ent_type == DXF_ATTRIB){
		x = dxf_find_attr_i2(current, stop, 72, 0);
		if (x){
			int t_alin = x->value.i_data;
			if(fabs(rad) < M_PI/2 && x->value.i_data < 3) 		
			
			x->value.i_data = 2 - x->value.i_data;
		}
		x = dxf_find_attr_i2(current, stop, 73, 0);
		if (x){
			int t_alin = x->value.i_data;
			if(fabs(rad) >= M_PI/2 && x->value.i_data > 0) 		
			
			x->value.i_data = 4 - x->value.i_data;
		}
		
		/* for ATTRIB entity */
		x = dxf_find_attr_i2(current, stop, 74, 0);
		if (x){
			int t_alin = x->value.i_data;
			if(fabs(rad) >= M_PI/2 && x->value.i_data > 0) 		
			
			x->value.i_data = 4 - x->value.i_data;
		}
		
		x = dxf_find_attr_i2(current, stop, 50, 0);
		if (x){
			double angle = ang;
			if(fabs(rad) > M_PI/2){
				angle -=180.0;
			}
			x->value.d_data += angle;
		}
	}
	else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
		for (j = 1; j < 4; j++){
			//for (i = 0; x = dxf_find_attr_i(obj, 10, i); i++){
			//	y = dxf_find_attr_i(obj, 20, i);
			for (i = 0; x = dxf_find_attr_i2(current, stop, 10 + j, i); i++){
				y = dxf_find_attr_i2(current, stop, 20 + j, i);
				if (y){
					/* calcule distance between point and reflection line */
					dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
					
					x_new = x->value.d_data + 2 * dist * (dy/modulus);
					y_new = y->value.d_data + 2 * dist * (-dx/modulus);
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
					/* calcule distance between point and reflection line */
					dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
					
					x_new = x->value.d_data + 2 * dist * (dy/modulus);
					y_new = y->value.d_data + 2 * dist * (-dx/modulus);
					x->value.d_data = x_new;
					y->value.d_data = y_new;
				}
			}
		}
	}
	if (ent_type == DXF_CIRCLE || ent_type == DXF_MTEXT ||
	ent_type == DXF_INSERT){
		x = dxf_find_attr_i2(current, stop, 50, 0);
		if (x){
			double angle = ang;
			if(fabs(rad) > M_PI/2){
				angle -=180.0;
			}
			x->value.d_data += angle;
		}
	}
	if (ent_type == DXF_ARC){
		
		x = dxf_find_attr_i2(current, stop, 50, 0);
		y = dxf_find_attr_i2(current, stop, 51, 0);
		if (x && y){
			double angle = atan2(dy, dx) * 180/M_PI;
			angle = ang_adjust_360(angle);
			
			double begin = -(x->value.d_data - angle) + angle;
			begin = ang_adjust_360(begin);
			double end = -(y->value.d_data - angle) + angle;
			end = ang_adjust_360(end);
			x->value.d_data = end;
			y->value.d_data = begin;
		}
		
	}
	
	if (ent_type == DXF_INSERT && (obj->obj.content)){
		x = dxf_find_attr_i2(current, stop, 41, 0);
		if (x){
			if(fabs(rad) < M_PI/2) x->value.d_data *= -1;
		}
		x = dxf_find_attr_i2(current, stop, 42, 0);
		if (x){
			int t_alin = x->value.i_data;
			if(fabs(rad) >= M_PI/2) x->value.d_data *= -1;
		}
		
		while (current){
			if (dxf_ident_ent_type(current) == DXF_ATTRIB){
				for (j = 0; j < 2; j++){
					for (i = 0; x = dxf_find_attr_i(current, 10 + j, i); i++){
						y = dxf_find_attr_i(current, 20 + j, i);
						if (y){
							/* calcule distance between point and reflection line */
							dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
							
							x_new = x->value.d_data + 2 * dist * (dy/modulus);
							y_new = y->value.d_data + 2 * dist * (-dx/modulus);
							x->value.d_data = x_new;
							y->value.d_data = y_new;
						}
					}
				}
				x = dxf_find_attr2(current, 72);
				if (x){
					if(fabs(rad) < M_PI/2 && x->value.i_data < 3)
						x->value.i_data = 2 - x->value.i_data;
				}
				
				/* for ATTRIB entity */
				x = dxf_find_attr2(current, 74);
				if (x){
					if(fabs(rad) >= M_PI/2 && x->value.i_data > 0)
						x->value.i_data = 4 - x->value.i_data;
				}
				
				dxf_node *rot_attr = dxf_find_attr_i(current, 50, 0);
				if (!rot_attr){
					dxf_node *last_attr = dxf_find_attr_i(current, 1, 0);
					rot_attr = dxf_attr_insert_after(last_attr, 50, (void *) (double[]){0.0}, current->obj.pool);
				}
				if (rot_attr){
					double angle = ang;
					if(fabs(rad) > M_PI/2){
						angle -=180.0;
					}
					rot_attr->value.d_data += angle;
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
						/* calcule distance between point and reflection line */
						dist = (-dy*x->value.d_data + dx*y->value.d_data + dy*x1 - dx*y1)/modulus;
						
						x_new = x->value.d_data + 2 * dist * (dy/modulus);
						y_new = y->value.d_data + 2 * dist * (-dx/modulus);
						x->value.d_data = x_new;
						y->value.d_data = y_new;
					}
				}
				for (i = 0; x = dxf_find_attr_i(current, 42, i); i++){
					x->value.d_data *= -1;
				}
			}
			current = current->next; /* go to the next in the list */
		}
	}
	return ret;
}

int mtext_change_text (dxf_node *obj, char *text, int len, int pool){
	/* change text in MTEXT DXF entity (groups 3- multiple - and 1 - mandatory single) */
	
	/* verify if obj is an MTEXT entity */
	if (dxf_ident_ent_type(obj) != DXF_MTEXT) return 0;
	if (!text) return 0; /* check if has text */
	
	int i = 0, j = 0, pos = 0, num_ignore = 0;
	for (i = 0; i < len; i++){ /* count chars that will ignored  (line breaks)*/
		if (text[i] == '\v' || text[i] == '\r' || text[i] == '\n')
			num_ignore++;
	}
	
	char curr_text[DXF_MAX_CHARS], curr_char;
	
	/* calcule how many extra groups are necessary to whole text */
	int extra_str = (len - num_ignore)/(DXF_MAX_CHARS - 1);
	/* get existent extra text groups in obj */
	int exist_str = dxf_count_attr(obj, 3);
	
	dxf_node *curr_str = NULL;
	/*get the mandatory text group in obj*/
	dxf_node *final_str = dxf_find_attr2(obj, 1);
	
	/* adjust the number of groups in obj */
	if (extra_str > exist_str){ /* add groups */
		for (i = 0; i < (extra_str - exist_str); i++){
			dxf_attr_insert_before(final_str, 3, (void *)"", pool);
		}
	}
	else if (extra_str < exist_str){ /* remove groups */
		for (i = 0; i < (exist_str - extra_str); i++){
			curr_str = dxf_find_attr2(obj, 3);
			dxf_obj_detach(curr_str);
		}
	}
	/* edit text in each group */
	int text_pos = 0;
	for (i = 0; i <= extra_str; i++){
		pos = 0;
		j = 0;
		/* construct string to group */
		while (pos < DXF_MAX_CHARS - 1 && text_pos < len){
			curr_char = text[text_pos];
			/* ignore the break line chars */
			if (curr_char != '\n' && curr_char != '\v' && curr_char != '\r'){
				curr_text[pos] = curr_char;
				pos ++;
			}
			j++;
			text_pos++;
		}
		curr_text[pos] = 0; /*terminate string*/
		/* modify current group */
		if (i < extra_str) dxf_attr_change_i(obj, 3, curr_text, i);
		else dxf_attr_change(obj, 1, curr_text);
	}
	return 1;
}

dxf_node * dxf_attr2text (dxf_node *attrib, int mode, int pool){
	/*create new attdef ent by copying parameters of an text ent */
	/* offset position of attdef by x0, y0, z0*/
	if(attrib){
		if (attrib->type != DXF_ENT){
			return NULL; /*Fail - wrong type */
		}
		if (strcmp(attrib->obj.name, "ATTRIB") != 0){
			return NULL; /*Fail - wrong entity */
		}
		
		double x1= 0.0, y1 = 0.0, z1 = 0.0;
		double x2= 0.0, y2 = 0.0, z2 = 0.0;
		double h = 1.0, rot = 0.0, t_scale_x = 1.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0;
		char txt[DXF_MAX_CHARS], tag[DXF_MAX_CHARS+1];
		char layer[DXF_MAX_CHARS], l_type[DXF_MAX_CHARS];
		char t_style[DXF_MAX_CHARS];
		int color = 0, paper = 0, lw = -2;
		int t_alin_v = 0, t_alin_h = 0;
		
		
		/* clear the strings */
		txt[0] = 0;
		tag[0] = '#';
		tag[1] = 0;
		layer[0] = 0;
		l_type[0] = 0;
		t_style[0] = 0;
		
		dxf_node *current = NULL;
		if (attrib->obj.content){
			current = attrib->obj.content->next;
		}
		while (current){
			if (current->type == DXF_ATTR){ /* scan parameters */
				switch (current->value.group){
					case 1:
						strcpy(txt, current->value.s_data);
						break;
					case 2:
						strcpy(tag+1, current->value.s_data);
						break;
					case 7:
						strcpy(t_style, current->value.s_data);
						break;
					case 6:
						strcpy(l_type, current->value.s_data);
						break;
					case 8:
						strcpy(layer, current->value.s_data);
						break;
					case 10:
						x1 = current->value.d_data;
						break;
					case 11:
						x2 = current->value.d_data;
						break;
					case 20:
						y1 = current->value.d_data;
						break;
					case 21:
						y2 = current->value.d_data;
						break;
					case 30:
						z1 = current->value.d_data;
						break;
					case 31:
						z2 = current->value.d_data;
						break;
					case 40:
						h = current->value.d_data;
						break;
					case 41:
						t_scale_x = current->value.d_data;
						break;
					case 50:
						rot = current->value.d_data;
						break;
					case 62:
						color = current->value.i_data;
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
						break;
					case 370:
						lw = current->value.i_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		
		dxf_node * text = NULL;
		
		if (mode & EXPL_VALUE)
			text = dxf_new_text (x1, y1, z1, h, txt, color, layer, l_type, lw, paper, pool);
		if (mode & EXPL_TAG)
			text = dxf_new_text (x1, y1, z1, h, tag, color, layer, l_type, lw, paper, pool);
		
		dxf_attr_change(text, 11, (void *)(double[]){x2});
		dxf_attr_change(text, 21, (void *)(double[]){y2});
		dxf_attr_change(text, 31, (void *)(double[]){z2});
		dxf_attr_change(text, 50, (void *)&rot);
		dxf_attr_change(text, 7, (void *)t_style);
		dxf_attr_change(text, 41, (void *)&t_scale_x);
		dxf_attr_change(text, 72, (void *)&t_alin_h);
		dxf_attr_change(text, 73, (void *)&t_alin_v);
		dxf_attr_change(text, 210, (void *)&extru_x);
		dxf_attr_change(text, 220, (void *)&extru_y);
		dxf_attr_change(text, 230, (void *)&extru_z);
		
		return text;
	}

	return NULL;
}

list_node * dxf_edit_expl_ins(dxf_drawing *drawing, dxf_node * ins_ent, int mode){
	if (!drawing) return NULL;
	if (!ins_ent) return NULL;
	
	list_node *list = NULL;
	dxf_node *current = NULL;
	dxf_node *block = NULL, *blk_name = NULL;
	
	double x = 0.0, y = 0.0, z = 0.0, angle = 0.0;
	double scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
	
	/* get insert parameters */
	current = ins_ent->obj.content;
	while (current){
		if (current->type == DXF_ATTR){
			switch (current->value.group){
				case 10:
					x = current->value.d_data;
					break;
				case 20:
					y = current->value.d_data;
					break;
				case 30:
					z = current->value.d_data;
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
					angle = current->value.d_data;
					break;
				case 2:
					blk_name = current;
					break;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	
	/* find relative block */
	if(blk_name) {
		block = dxf_find_obj_descr2(drawing->blks, "BLOCK", blk_name->value.s_data);
		if(block) {
			list = list_new(NULL, FRAME_LIFE);
			current = block->obj.content;
			while (current){ /* sweep elements in block */
				if (current->type == DXF_ENT){
					if (strcmp(current->obj.name, "ATTDEF") != 0){ /* skip ATTDEF elements */
						dxf_node *new_ent = dxf_ent_copy(current, FRAME_LIFE);
						/* apply modifications */
						dxf_edit_scale(new_ent, scale_x, scale_y, scale_z);
						dxf_edit_rot(new_ent, angle);
						dxf_edit_move(new_ent, x, y, z);
						
						/* append to list*/
						list_node * new_node = list_new(new_ent, FRAME_LIFE);
						list_push(list, new_node);
						
					}
				}
				current = current->next; /* go to the next in the list*/
			}
		}
	}
	
	if (!(mode & (EXPL_VALUE | EXPL_TAG))) return list;
	
	/* get insert attributes */
	current = ins_ent->obj.content;
	while (current){
		if (current->type == DXF_ENT){ /* look for DXF entity */
			if (strcmp(current->obj.name, "ATTRIB") == 0){
				/* init return list, if it needed */
				if (!list) list = list_new(NULL, FRAME_LIFE);
				if (mode & EXPL_VALUE){ /* convert value to text */
					dxf_node *new_ent = dxf_attr2text (current, EXPL_VALUE, FRAME_LIFE);
					/* append to list*/
					list_node * new_node = list_new(new_ent, FRAME_LIFE);
					list_push(list, new_node);
				}
				if (mode & EXPL_TAG){ /* convert tag to text */
					dxf_node *new_ent = dxf_attr2text (current, EXPL_TAG, FRAME_LIFE);
					/* append to list*/
					list_node * new_node = list_new(new_ent, FRAME_LIFE);
					list_push(list, new_node);
				}
			}
		}
		current = current->next; /* go to the next in the list */
	}
	
	return list;
	
}