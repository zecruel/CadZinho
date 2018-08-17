
#include "dxf_attract.h"
#include "list.h"

#define IN_BOUNDS(x,y,p1x,p1y,p2x,p2y) ((((x <= p1x) && (x >= p2x))||((x <= p2x) && (x >= p1x))) && (((y <= p1y) && (y >= p2y))||((y <= p2y) && (y >= p1y))))
#define TOL 1e-6
#define NEAR_LN(x,y,p1x,p1y,p2x,p2y,s) (((fabs(p1x-p2x)<TOL) && (fabs(p1x - x) < s) && (((y <= p1y) && (y >= p2y))||((y <= p2y) && (y >= p1y)))) || ((fabs(p1y-p2y)<TOL) && (fabs(p1y - y) < s) && (((x <= p1x) && (x >= p2x))||((x <= p2x) && (x >= p1x)))))
#define MAX_CAND 50

#define SIG(x) (x >= 0) ? 1 : -1


int point_lies_seg(double p1x, double p1y, double p2x, double p2y,
double x, double y){
	if(fabs(p1x - p2x) < TOL){
		if ((fabs(x - p2x) < TOL) &&
		(((y >= p1y) && (y <= p2y)) || 
		((y <= p1y) && (y >= p2y)))){
			return 1;
		}
	}
	else if(fabs(p1y - p2y) < TOL){
		if ((fabs(y - p2y) < TOL) &&
		(((x >= p1x) && (x <= p2x)) || 
		((x <= p1x) && (x >= p2x)))){
			return 1;
		}
	}
	else if ((((x >= p1x) && (x <= p2x)) ||
	((x <= p1x) && (x >= p2x))) &&
	(((y >= p1y) && (y <= p2y)) || 
	((y <= p1y) && (y >= p2y)))){
		return 1;
	}
	return 0;
}

void arc_bulge(double pt1_x, double pt1_y,
double pt2_x, double pt2_y, double bulge,
double *radius, double *ang_start, double *ang_end,
double *center_x, double *center_y){
	
	double theta, alfa, d, ang_c, start, end;
	/* some math to find radius and center point */
	
	theta = 2 * atan(bulge);
	alfa = atan2(pt2_y-pt1_y, pt2_x-pt1_x);
	d = sqrt((pt2_y-pt1_y)*(pt2_y-pt1_y) + (pt2_x-pt1_x)*(pt2_x-pt1_x)) / 2;
	*radius = d*(bulge*bulge + 1)/(2*bulge);
	
	ang_c = M_PI+(alfa - M_PI/2 - theta);
	*center_x = *radius*cos(ang_c) + pt1_x;
	*center_y = *radius*sin(ang_c) + pt1_y;
	
	/* start and end angles found by points coordinates*/
	start = atan2(pt1_y - *center_y, pt1_x - *center_x);
	end = atan2(pt2_y - *center_y, pt2_x - *center_x);
	
	/* set angle range to 0-2*pi */
	if (start < 0){
		start += 2*M_PI;
	}
	if (end < 0){
		end += 2*M_PI;
	}
	if (*radius >= 0){
		*ang_start = start;
		*ang_end = end;
	}
	else{
		*ang_start = end;
		*ang_end = start;
		*radius = fabs(*radius);
	}
}

int axis_transform(double *x, double *y, double *z, double normal[3]){
	if ((x != NULL) && (y != NULL) && (z != NULL)){
		double x0, y0, z0;
		double x_axis[3], y_axis[3], z_axis[3], point[3];
		double wy_axis[3] = {0.0, 1.0, 0.0};
		double wz_axis[3] = {0.0, 0.0, 1.0};
		
		/* axis tranform */
		/* choose absolute world axis, according DXF spec, and obtain x axis */
		if ((fabs(normal[0] < 0.015625)) && (fabs(normal[1] < 0.015625))){
			cross_product(wy_axis, normal, x_axis);
		}
		else{
			cross_product(wz_axis, normal, x_axis);
		}
		unit_vector(x_axis); /* normalize axis */
		cross_product(normal, x_axis, y_axis); /*obtain y axis */
		unit_vector(y_axis); /* normalize axis */
		
		/* z_axis is the same normal, but normalized*/
		z_axis[0] = normal[0]; z_axis[1] = normal[1]; z_axis[2] = normal[2];
		unit_vector(z_axis); /* normalize axis */
		
		/*obtain result point */
		point[0] = *x;
		point[1] = *y;
		point[2] = *z;
		*x = dot_product(point, x_axis);
		*y = dot_product(point, y_axis);
		*z = dot_product(point, z_axis);
		
		return 1;
	}
	return 0;
}

int transform(double *x, double *y, struct ins_space space){
	if ((x != NULL) && (y != NULL)){
		double x0, y0;
		
		/* rotation constants */
		double cosine = cos(space.rot * M_PI/180);
		double sine = sin(space.rot * M_PI/180);
		
		/* scale and translation transformations */
		*x = *x * space.scale_x + space.ofs_x;
		*y = *y * space.scale_y + space.ofs_y;
		
		/* rotation transform */ 
		x0 = cosine*(*x - space.ofs_x) - sine*(*y - space.ofs_y) + space.ofs_x;
		y0 = sine*(*x - space.ofs_x) + cosine*(*y - space.ofs_y) + space.ofs_y;

		/* update return values*/ 
		*x = x0;
		*y = y0;
		
		return 1;
	}
	return 0;
}

int ellipse_transf(double *center_x, double *center_y, double *center_z, 
double *axis, double *ratio, double *rot, double normal[3]){
	int ok = 0;
	
	if ((center_x != NULL) && (center_y != NULL) && (center_z != NULL) && (axis != NULL) && (ratio != NULL) && (rot != NULL) && (normal != NULL)){
		double pt1_x, pt1_y, pt1_z;
		/* convert OCS to WCS */
		axis_transform(center_x, center_y, center_z, normal);
		
		/* transform the circle in ellipse, by the OCS */
		pt1_x = 1.0;
		pt1_y = 1.0;
		pt1_z = 0.0;
		axis_transform(&pt1_x, &pt1_y, &pt1_z, normal);
		*axis *= fabs(pt1_x);
		*ratio *= fabs(pt1_y / pt1_x);
		*rot += atan2(normal[0], normal[1]);
		ok = 1;
	}
	return ok;
}

double ellipse_par (double ang, double a, double b){
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

int in_ellip_bound(double pos_x, double pos_y, double sensi,
double center_x, double center_y, 
double axis, double ratio, double rot){
	
	/* rotation constants */
	double cosine = cos(rot);
	double sine = sin(rot);
	
	/* calc the half width and height of rectangle of ellipse bounds */
	/*https://math.stackexchange.com/questions/91132/how-to-get-the-limits-of-rotated-ellipse
	xa = Sqrt[a^2 Cos[th]^2 + b^2 Sin[th]^2]
	ya = Sqrt[a^2 Sin[th]^2 + b^2 Cos[th]^2]*/
	double x = sqrt(pow(axis, 2)*pow(cosine, 2) + pow(axis*ratio, 2)*pow(sine, 2));
	double y = sqrt(pow(axis, 2)*pow(sine, 2) + pow(axis*ratio, 2)*pow(cosine, 2));
	
	/*determine the rectangle coordinates, considering the sensibility */
	double tr_x = center_x + x + sensi;
	double tr_y = center_y + y + sensi;
	double bl_x = center_x - x - sensi;
	double bl_y = center_y - y - sensi;
	
	/* verify if point is in bounds */
	if((pos_x >= bl_x) && (pos_x <= tr_x) && 
	(pos_y >= bl_y) && (pos_y <= tr_y)){
		return 1;
	}
	return 0;
}

void angle_range(double *ang){
	/* set angle range to 0-2*pi */
	if (fabs(*ang) > 2*M_PI) *ang = fmod(*ang, 2*M_PI);
	if (*ang < 0) *ang += 2*M_PI;
}

int arc_near(double axis, double ratio, double rot,
double ang_start, double ang_end,
double center_x, double center_y, 
double pos_x, double pos_y,
double *ret_x, double *ret_y){
	/* elliptical arc */
	
	int ret = ATRC_NONE;
	double curr_dist;
	
	double test, start, end;
	
	/* rotation constants */
	double cosine = cos(rot);
	double sine = sin(rot);
	
	/*ellipse's parameters */
	double a = axis, b = axis * ratio;
	
	/* find the angle of point, referenced to arc center */
	double ang_pt = atan2(pos_y - center_y, pos_x - center_x);
	
	angle_range(&rot); /* set angle range to 0-2*pi */
	
	/* update by rotation */
	ang_pt = ang_pt - rot;
	angle_range(&ang_pt); /* set angle range to 0-2*pi */
	
	/* find the ellipse polar parameter (t) for  point*/
	double t = ellipse_par(ang_pt, a, b);
	
	/* verify if arc is a full circle */
	if ((fabs(ang_end - ang_start) < 1e-6) || (fabs(ang_end - ang_start) >= (2*M_PI - 1e-6))){
		ang_start = 0.0;
		ang_end = 2*M_PI;
		
		start = ang_start;
		end = ang_end;
		test = 0.0;
	}
	else{
		angle_range(&ang_start); /* set angle range to 0-2*pi */
		angle_range(&ang_end); /* set angle range to 0-2*pi */
		
		/* verify if point angle is between start and end of arc */
		test = t - ang_start;
		if (test < 0 ) test += 2*M_PI;
		end = ang_end - ang_start;
		if (end < 0 ) end += 2*M_PI;
	}
	
	if (test <= end){		
		/* point in ellipse near current position */
		*ret_x = center_x + a*cos(t)*cosine - b*sin(t)*sine;
		*ret_y = center_y + a*cos(t)*sine + b*sin(t)*cosine;
		return 1;
	}
	return 0;
}

double dxf_text_width(shape *font, char *text){
	if ((text!= NULL) && (font!=NULL)) {
		graph_obj *curr_graph = shx_font_parse(font, ONE_TIME, text, NULL);
		if (curr_graph){
			double txt_w;
			txt_w = fabs(curr_graph->ext_max_x - curr_graph->ext_min_x);
			return txt_w;
		}
		graph_mem_pool(ZERO_GRAPH, ONE_TIME);
		graph_mem_pool(ZERO_LINE, ONE_TIME);
	}
	return 0;
}

int dxf_line_get (dxf_drawing *drawing, dxf_node * obj, 
double *pt1_x, double *pt1_y, double *pt1_z, 
double *pt2_x, double *pt2_y, double *pt2_z){
	
	dxf_node *current = NULL;
	
	/*flags*/
	int pt1 = 0, pt2 = 0, ok = 0;
	
	if (obj){ 
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
				*pt1_x = 0, *pt1_y = 0, *pt1_z = 0;
				*pt2_x = 0, *pt2_y = 0, *pt2_z = 0;
			}
		}
	}

	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 10:
					*pt1_x = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 11:
					*pt2_x = current->value.d_data;
					pt2 = 1; /* set flag */
					break;
				case 20:
					*pt1_y = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 21:
					*pt2_y = current->value.d_data;
					pt2 = 1; /* set flag */
					break;
				case 30:
					*pt1_z = current->value.d_data;
					pt1 = 1; /* set flag */
					break;
				case 31:
					*pt2_z = current->value.d_data;
					pt2 = 1; /* set flag */
					break;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	
	if((pt1 !=0) && (pt2 !=0)) ok = 1;
	
	return ok;
}

int dxf_circle_get(dxf_drawing *drawing, dxf_node * obj,
double *center_x, double *center_y, double *center_z, 
double *axis, double *ratio, double *rot){
	int ok = 0;
	
	if(obj){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0, radius = 0;
		double elev = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		/*flags*/
		int pt1 = 0;
				
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
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
					case 40:
						radius = current->value.d_data;
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
			current = current->next; /* go to the next in the list */
		}
		
		/* convert OCS to WCS, transform the circle in ellipse */
		normal[0] = extru_x;
		normal[1] = extru_y;
		normal[2] = extru_z;
		*center_x = pt1_x;
		*center_y = pt1_y;
		*center_z = pt1_z;
		*axis = radius;
		*ratio = 1.0;
		*rot = 0.0;
		ok = ellipse_transf(center_x, center_y, center_z, axis, ratio, rot, normal);
	}
	return ok;
}

int dxf_arc_get(dxf_drawing *drawing, dxf_node * obj,
double *center_x, double *center_y, double *center_z, 
double *axis, double *ratio, double *rot, 
double *start_ang, double *end_ang){
	int ok = 0;
	
	if(obj){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0, radius;
		double elev = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		/*flags*/
		int pt1 = 0;
				
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
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
					case 40:
						radius = current->value.d_data;
						break;
					case 50:
						*start_ang = current->value.d_data;
						break;
					case 51:
						*end_ang = current->value.d_data;
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
			current = current->next; /* go to the next in the list */
		}
		
		/* convert OCS to WCS, transform the circle in ellipse */
		normal[0] = extru_x;
		normal[1] = extru_y;
		normal[2] = extru_z;
		*center_x = pt1_x;
		*center_y = pt1_y;
		*center_z = pt1_z;
		*axis = radius;
		*ratio = 1.0;
		*rot = 0.0;
		ok = ellipse_transf(center_x, center_y, center_z, axis, ratio, rot, normal);
		
		/*convert to radians */
		*start_ang *= M_PI/180;
		*end_ang *= M_PI/180;
	}
	return ok;
}

int dxf_ellipse_get(dxf_drawing *drawing, dxf_node * obj,
double *center_x, double *center_y, double *center_z, 
double *axis, double *ratio, double *rot, 
double *start_ang, double *end_ang){
	int ok = 0;
	
	if(obj){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double elev = 0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		
		/*flags*/
		int pt1 = 0, pt2 = 0;
				
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
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
						*ratio = current->value.d_data;
						break;
					case 41:
						*start_ang = current->value.d_data;
						break;
					case 42:
						*end_ang = current->value.d_data;
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
			current = current->next; /* go to the next in the list */
		}
		
		/* convert OCS to WCS */
		normal[0] = extru_x;
		normal[1] = extru_y;
		normal[2] = extru_z;
		*center_x = pt1_x;
		*center_y = pt1_y;
		*center_z = pt1_z;
		*axis = sqrt(pow(pt2_x, 2) + pow(pt2_y, 2));
		*rot = atan2(pt2_y, pt2_x);
		ok = ellipse_transf(center_x, center_y, center_z, axis, ratio, rot, normal);
		
		/*convert to radians */
		//*start_ang *= M_PI/180;
		//*end_ang *= M_PI/180;
	}
	return ok;
}

int dxf_lwpline_get_pt(dxf_drawing *drawing,
dxf_node * obj, dxf_node ** next,
double *pt1_x, double *pt1_y, double *pt1_z, double *bulge){
	
	dxf_node *current = NULL;
	static dxf_node *last = NULL;
	static double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
	
	/*flags*/
	int first = 0, pt1 = 0, ok = 0;
	static int pline_flag = 0, closed =0, init = 0;
	
	double px = 0.0, py = 0.0, pz = 0.0, bul = 0.0;
	static double last_x, last_y, last_z, curr_x, elev;
	
	double x, y, z; /*return values */
	
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
			
			x = curr_x;
			y = py;
			z = pz;
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
			
			x = curr_x;
			y = py;
			z = pz;
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
		x = last_x;
		y = last_y;
		z = last_z;
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
	else if (axis_transform(&x, &y, &z, normal)){
		/* update return values */
		*pt1_x = x;
		*pt1_y = y;
		*pt1_z = z;
	}
	
	return ok;
}

int dxf_text_get(dxf_drawing *drawing, dxf_node * obj,
char *text, int *fnt_idx, double *above, double *below,
double *ins_x, double *ins_y, double *ins_z, 
double *alin_x, double *alin_y, double *alin_z, 
double *w, double *h, double *rot, 
int *alin_v, int *alin_h){
	int ok = 0;
	
	if(obj){
		dxf_node *current = NULL;
		double pt1_x = 0, pt1_y = 0, pt1_z = 0;
		double pt2_x = 0, pt2_y = 0, pt2_z = 0;
		double elev = 0, size = 0, scale_x = 1.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0, normal[3];
		char t_style[DXF_MAX_CHARS];
		char tmp_str[DXF_MAX_CHARS];
		
		char *pos_st, *pos_curr, *pos_tmp, special;
		int under_l, over_l;
		double fnt_size, txt_size;
		shape *shx_font = NULL;
		
		t_style[0] = 0;
		tmp_str[0] = 0;
		
		/*flags*/
		int pt1 = 0, pt2 = 0;
				
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
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
					case 40:
						size = current->value.d_data;
						break;
					case 41:
						scale_x = current->value.d_data;
						break;
					case 50:
						*rot = current->value.d_data;
						break;
					case 38:
						elev = current->value.d_data;
						break;
					case 72:
						*alin_h = current->value.i_data;
						break;
					case 73:
						*alin_v = current->value.i_data;
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
			current = current->next; /* go to the next in the list */
		}
		
		/* find the font index and font*/
		*fnt_idx = dxf_font_idx(drawing, t_style);
		shx_font = drawing->text_fonts[*fnt_idx].shx_font;
		
		if(shx_font == NULL){ /* if font not loaded*/
			/* use the deafault font*/
			shx_font = drawing->text_fonts[0].shx_font;
		}
		
		/* find the dimentions of SHX font */
		if(shx_font){ /* if the font exists */
			if(shx_font->next){ /* the font descriptor is stored in first iten of list */
				if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
					*above = shx_font->next->cmds[0]; /* size above the base line of text */
					*below = shx_font->next->cmds[1]; /* size below the base line of text */
					if((*above + *below) > 0){
						fnt_size = *above + *below;
						ok = 1;
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
		
		/* find the dimentions of text */
		txt_size = size/(*above);
		
		
		/* convert OCS to WCS */
		normal[0] = extru_x;
		normal[1] = extru_y;
		normal[2] = extru_z;
		*ins_x = pt1_x;
		*ins_y = pt1_y;
		*ins_z = pt1_z;
		*alin_x = pt2_x;
		*alin_y = pt2_y;
		*alin_z = pt2_z;
		axis_transform(ins_x, ins_y, ins_z, normal);
		axis_transform(alin_x, alin_y, alin_z, normal);
		pt1_x = 1.0;
		pt1_y = 1.0;
		pt1_z = 0.0;
		axis_transform(&pt1_x, &pt1_y, &pt1_z, normal);
		scale_x *= fabs(pt1_x);
		txt_size *= fabs(pt1_y / pt1_x);
		*rot += atan2(normal[0], normal[1]);
		*w = dxf_text_width(shx_font, tmp_str) * txt_size * scale_x;
		*h = size * fabs(pt1_y / pt1_x);
		*above *= txt_size;
		*below *= txt_size;
	}
	return ok;
}

int dxf_line_attract(double pt1_x, double pt1_y, 
double pt2_x, double pt2_y, enum attract_type type,
double pos_x, double pos_y, double ref_x, double ref_y, double sensi, 
double *ret_x, double *ret_y,
int *init_dist, double *min_dist){
	
	int ret = ATRC_NONE;
	double curr_dist;
			
	if(type & ATRC_END){ /* if type of attractor is flaged as endpoint */
		/* check if points of the line pass on distance criteria */
		curr_dist = sqrt(pow(pt1_x - pos_x, 2) + pow(pt1_y - pos_y, 2));
		if (curr_dist < sensi){
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = pt1_x;
				*ret_y = pt1_y;
				ret = ATRC_END;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = pt1_x;
				*ret_y = pt1_y;
				ret = ATRC_END;
			}
		}
		curr_dist = sqrt(pow(pt2_x - pos_x, 2) + pow(pt2_y - pos_y, 2));
		if (curr_dist < sensi){
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = pt2_x;
				*ret_y = pt2_y;
				ret = ATRC_END;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = pt2_x;
				*ret_y = pt2_y;
				ret = ATRC_END;
			}
		}
		
	}
	if(type & ATRC_MID){ /* if type of attractor is flaged as midpoint */
		double mid_x = (pt1_x + pt2_x)/2;
		double mid_y = (pt1_y + pt2_y)/2;
		curr_dist = sqrt(pow(mid_x - pos_x, 2) + pow(mid_y - pos_y, 2));
		if (curr_dist < sensi){
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = mid_x;
				*ret_y = mid_y;
				ret = ATRC_MID;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = mid_x;
				*ret_y = mid_y;
				ret = ATRC_MID;
			}
		}
	}
	
	if(type & ATRC_PERP){ /* if type of attractor is flaged as perpenticular */
		double perp_x, perp_y;
		double a1, b1, c1, a2, b2, c2;
		/* parameters of line in general form ax+by = c */
		a1 = pt1_y - pt2_y;
		b1 = pt2_x - pt1_x;
		c1 = (pt2_x - pt1_x)*pt1_y + (pt1_y - pt2_y)*pt1_x;
		
		/* parameters of perpenticular line passing through the reference point */
		a2 = -b1;
		b2 = a1;
		c2 = a1*ref_y - b1*ref_x;
		
		/* find the intersection */
		if (line_inter(a1, b1, c1, a2, b2, c2, &perp_x, &perp_y)){
			/* check if pass on distance criteria */
			curr_dist = sqrt(pow(perp_x - pos_x, 2) + pow(perp_y - pos_y, 2));
			if (curr_dist < sensi){
				if (*init_dist == 0){
					*init_dist = 1;
					*min_dist = curr_dist;
					*ret_x = perp_x;
					*ret_y = perp_y;
					ret = ATRC_PERP;
				}
				else if (curr_dist < *min_dist){
					*min_dist = curr_dist;
					*ret_x = perp_x;
					*ret_y = perp_y;
					ret = ATRC_PERP;
				}
			}
		}
	}
	
	if(type & ATRC_ANY){ /* if type of attractor is flaged as any point */
		/*consider line equation ax + by + c = 0 */
		double a = pt2_y - pt1_y;
		double b = -(pt2_x - pt1_x);
		double c = pt2_x * pt1_y - pt2_y * pt1_x;
		
		if ((a != 0) || (b != 0)){
			/* calcule distance between point  and line */
			curr_dist = fabs(a*pos_x + b*pos_y + c)/
					sqrt(pow(a, 2) + pow(b, 2));
			if (curr_dist < sensi){
				/* look the closest point on line */
				/* equation from https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line */
				double any_x, any_y;
				if (b != 0){
					any_x = (b * (b*pos_x - a*pos_y) - a*c)/
							(pow(a, 2) + pow(b, 2));
				}
				else{
					any_x = pt1_x;
				}
				if (a != 0){
					any_y = (a * (-b*pos_x + a*pos_y) - b*c)/
							(pow(a, 2) + pow(b, 2));
				}
				else {
					any_y = pt1_y;
				}
				/* verify if point is in segment */
				if ((((any_x <= pt1_x) && (any_x >= pt2_x))||
					((any_x <= pt2_x) && (any_x >= pt1_x))) &&
					(((any_y <= pt1_y) && (any_y >= pt2_y))||
					((any_y <= pt2_y) && (any_y >= pt1_y)))){
					
					if (*init_dist == 0){
						*init_dist = 1;
						*min_dist = curr_dist;
						*ret_x = any_x;
						*ret_y = any_y;
						ret = ATRC_ANY;
					}
					else if (curr_dist < *min_dist){
						*min_dist = curr_dist;
						*ret_x = any_x;
						*ret_y = any_y;
						ret = ATRC_ANY;
					}
				}
			}
		}
		
	}
	return ret;
}

int dxf_text_attract(double pt1_x, double pt1_y, 
double pt2_x, double pt2_y,
double w, double h, double rot,
int alin_v, int alin_h, double above, double below,
enum attract_type type,
double pos_x, double pos_y, double sensi, 
double *ret_x, double *ret_y,
int *init_dist, double *min_dist){
	
	int ret = ATRC_NONE;
	double curr_dist;
			
	if(type & ATRC_INS){ /* if type of attractor is flaged as insert */
		/* check if points of the line pass on distance criteria */
		curr_dist = sqrt(pow(pt1_x - pos_x, 2) + pow(pt1_y - pos_y, 2));
		if (curr_dist < sensi){
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = pt1_x;
				*ret_y = pt1_y;
				ret = ATRC_INS;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = pt1_x;
				*ret_y = pt1_y;
				ret = ATRC_INS;
			}
		}
	}
	if(type & ATRC_NODE){ /* if type of attractor is flaged as node*/
		curr_dist = sqrt(pow(pt2_x - pos_x, 2) + pow(pt2_y - pos_y, 2));
		if (curr_dist < sensi){
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = pt2_x;
				*ret_y = pt2_y;
				ret = ATRC_NODE;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = pt2_x;
				*ret_y = pt2_y;
				ret = ATRC_NODE;
			}
		}
		
	}
		
	if(type & ATRC_ANY){ /* if type of attractor is flaged as any point */
		double t_base_x, t_base_y, t_center_x = 0, t_center_y = 0;
		double t_pos_x, t_pos_y, t_scale_x = 1, any_x, any_y;
		int i, j;
		
		t_base_x =  pt2_x;
		t_base_y =  pt2_y;
		
		if ((alin_v == 0) && (alin_h == 0)){
			t_base_x =  pt1_x;
			t_base_y =  pt1_y;
		}
		
		/* find the insert point of text, in function of its aling */
		else if(alin_h < 3){
			t_center_x = (double)alin_h * w/2;
		}
		else{ 
			if(alin_h == 4){
				t_center_y = h/2;
			}
			else{
				t_scale_x = sqrt(pow((pt2_x - pt1_x), 2) + pow((pt2_y - pt1_y), 2))/w;
				t_base_x =  pt1_x + (pt2_x - pt1_x)/2;
				t_base_y =  pt1_y + (pt2_y - pt1_y)/2;
			}
			
			t_center_x = t_scale_x*w/2;
		}
		if(alin_v >0){
			if(alin_v != 1){
				t_center_y = (double)(alin_v - 1) * h/2;
			}
			else{
				t_center_y = -below;
			}
		}
		
		/* rotation constants */
		double cosine = cos(rot*M_PI/180);
		double sine = sin(rot*M_PI/180);
		
		for(i = 0; i < 3; i++){
			for(j = 0; j < 3; j++){
				t_pos_x = (t_base_x - t_center_x) + (double)i * w/2;
				t_pos_y = (t_base_y - t_center_y) + (double)j * h/2;
				
				any_x = cosine*(t_pos_x - t_base_x) - sine*(t_pos_y - t_base_y) + t_base_x;
				any_y = sine*(t_pos_x - t_base_x) + cosine*(t_pos_y - t_base_y) + t_base_y;
		
				curr_dist = sqrt(pow(any_x - pos_x, 2) + pow(any_y - pos_y, 2));
				if (curr_dist < sensi){
					if (*init_dist == 0){
						*init_dist = 1;
						*min_dist = curr_dist;
						*ret_x = any_x;
						*ret_y = any_y;
						ret = ATRC_ANY;
					}
					else if (curr_dist < *min_dist){
						*min_dist = curr_dist;
						*ret_x = any_x;
						*ret_y = any_y;
						ret = ATRC_ANY;
					}
				}
			}
		}
	}
	return ret;
}

int dxf_arc_attract(double radius, double ang_start, double ang_end,
double center_x, double center_y, double ratio, double rot,
enum attract_type type,
double pos_x, double pos_y, double ref_x, double ref_y, double sensi, 
double *ret_x, double *ret_y,
int *init_dist, double *min_dist){
	/* elliptical arc */
	int i, j, count;
	int ret = ATRC_NONE;
	double curr_dist;
	
	double test, start, end;
	
	/* rotation constants */
	double cosine = cos(rot);
	double sine = sin(rot);
	
	/*ellipse's parameters */
	double a = radius, b = radius * ratio;
	
	/* find the angle of point, referenced to arc center */
	double ang_pt = atan2(pos_y - center_y, pos_x - center_x);
	
	angle_range(&rot); /* set angle range to 0-2*pi */
	
	/* update by rotation */
	ang_pt = ang_pt - rot;
	angle_range(&ang_pt); /* set angle range to 0-2*pi */
	
	/* find the ellipse polar parameter (t) for  point*/
	double t = ellipse_par(ang_pt, a, b);
	
	/* verify if arc is a full circle */
	if ((fabs(ang_end - ang_start) < 1e-6) || (fabs(ang_end - ang_start) >= (2*M_PI - 1e-6))){
		ang_start = 0.0;
		ang_end = 2*M_PI;
		type &= ~ATRC_END; /* disable the endpoint attractor */
		
		start = ang_start;
		end = ang_end;
		test = 0.0;
	}
	else{
		angle_range(&ang_start); /* set angle range to 0-2*pi */
		angle_range(&ang_end); /* set angle range to 0-2*pi */
		
		/* verify if point angle is between start and end of arc */
		test = t - ang_start;
		if (test < 0 ) test += 2*M_PI;
		end = ang_end - ang_start;
		if (end < 0 ) end += 2*M_PI;
	}
	
	if (test <= end){		
		/* point in ellipse near current position */
		double near_x = center_x + a*cos(t)*cosine - b*sin(t)*sine;
		double near_y = center_y + a*cos(t)*sine + b*sin(t)*cosine;
		
		if(type & ATRC_END){ /* if type of attractor is flaged as endpoint */
			/* start and end points of arc, in cartesian coodinates */
			double pt1_x = center_x + a*cos(ang_start)*cosine - b*sin(ang_start)*sine;
			double pt1_y = center_y + a*cos(ang_start)*sine + b*sin(ang_start)*cosine;
			double pt2_x = center_x + a*cos(ang_end)*cosine - b*sin(ang_end)*sine;
			double pt2_y = center_y + a*cos(ang_end)*sine + b*sin(ang_end)*cosine;
			
			/* check if points of the arc pass on distance criteria */
			curr_dist = sqrt(pow(pt1_x - pos_x, 2) + pow(pt1_y - pos_y, 2));
			if (curr_dist < sensi){
				if (*init_dist == 0){
					*init_dist = 1;
					*min_dist = curr_dist;
					*ret_x = pt1_x;
					*ret_y = pt1_y;
					ret = ATRC_END;
				}
				else if (curr_dist < *min_dist){
					*min_dist = curr_dist;
					*ret_x = pt1_x;
					*ret_y = pt1_y;
					ret = ATRC_END;
				}
			}
			curr_dist = sqrt(pow(pt2_x - pos_x, 2) + pow(pt2_y - pos_y, 2));
			if (curr_dist < sensi){
				if (*init_dist == 0){
					*init_dist = 1;
					*min_dist = curr_dist;
					*ret_x = pt2_x;
					*ret_y = pt2_y;
					ret = ATRC_END;
				}
				else if (curr_dist < *min_dist){
					*min_dist = curr_dist;
					*ret_x = pt2_x;
					*ret_y = pt2_y;
					ret = ATRC_END;
				}
			}
		}
		if(type & ATRC_QUAD){ /* if type of attractor is flaged as quadrant */
			
			double quad_x[4], quad_y[4], t_q;
			quad_x[0] = center_x + a*cosine;
			quad_y[0] = center_y + a*sine;
			
			//quad_x[1] = center_x - b*sine;
			//quad_y[1] = center_y + b*cosine;
			t_q = ellipse_par(M_PI/2, a, b);
			quad_x[1] = center_x + a*cos(t_q)*cosine - b*sin(t_q)*sine;
			quad_y[1] = center_y + a*cos(t_q)*sine + b*sin(t_q)*cosine;
			
			
			quad_x[2] = center_x - a*cosine;
			quad_y[2] = center_y - a*sine;
			
			//quad_x[3] = center_x + b*sine;
			//quad_y[3] = center_y - b*cosine;
			t_q = ellipse_par(3*M_PI/2, a, b);
			quad_x[3] = center_x + a*cos(t_q)*cosine - b*sin(t_q)*sine;
			quad_y[3] = center_y + a*cos(t_q)*sine + b*sin(t_q)*cosine;
			
			/* check if point pass on distance criteria */
			int i;
			for (i = 0; i < 4; i++){
				curr_dist = sqrt(pow(quad_x[i] - pos_x, 2) + pow(quad_y[i] - pos_y, 2));
				if (curr_dist < sensi){
					if (*init_dist == 0){
						*init_dist = 1;
						*min_dist = curr_dist;
						*ret_x = quad_x[i];
						*ret_y = quad_y[i];
						ret = ATRC_QUAD;
					}
					else if (curr_dist < *min_dist){
						*min_dist = curr_dist;
						*ret_x = quad_x[i];
						*ret_y = quad_y[i];
						ret = ATRC_QUAD;
					}
				}
			}
		}
		if ((type & ATRC_ANY) && (!ret)){ /* if type of attractor is flaged as any */
			/* check if point pass on distance criteria */
			curr_dist = sqrt(pow(near_x - pos_x, 2) + pow(near_y - pos_y, 2));
			if (curr_dist < sensi){
				if (*init_dist == 0){
					*init_dist = 1;
					*min_dist = curr_dist;
					*ret_x = near_x;
					*ret_y = near_y;
					ret = ATRC_ANY;
				}
				else if (curr_dist < *min_dist){
					*min_dist = curr_dist;
					*ret_x = near_x;
					*ret_y = near_y;
					ret = ATRC_ANY;
				}
			}
		}
		if ((type & ATRC_CENTER) && (!ret)){ /* if type of attractor is flaged as center */
			/* check if point pass on distance criteria */
			curr_dist = fabs(sqrt(pow(near_x - pos_x, 2) + pow(near_y - pos_y, 2)));
			if (curr_dist < sensi){
				if (*init_dist == 0){
					*init_dist = 1;
					*min_dist = curr_dist;
					*ret_x = center_x;
					*ret_y = center_y;
					ret = ATRC_CENTER;
				}
				else if (curr_dist < *min_dist){
					*min_dist = curr_dist;
					*ret_x = center_x;
					*ret_y = center_y;
					ret = ATRC_CENTER;
				}
			}
		}
		if (type & ATRC_TAN){ /* if type of attractor is flaged as tangent */
			double t_x[2], t_y[2];
			int num_inter = 0;
			double ln_a, ln_b, ln_c, x, y;
			
			/* rotation constants */
			cosine = cos(-rot);
			sine = sin(-rot);
			
			/* rotation of ref point to align to ellipse axis */
			x = cosine*(ref_x - center_x) - sine*(ref_y - center_y) + center_x;
			y = sine*(ref_x - center_x) + cosine*(ref_y - center_y) + center_y;
			
			/* translate point to ellipse center */
			x -= center_x;
			y -= center_y;
			
			/* chord of contact points - line in form ax+ bx =c */
			ln_a = x/pow(a, 2);
			ln_b = y/pow(b, 2);
			ln_c = 1.0;
			
			num_inter = el_ln_inter(0, 0, radius, ratio, 0, ln_a,  ln_b,  ln_c, t_x, t_y);
			
			/* rotate and translate back the result points */
			cosine = cos(rot);
			sine = sin(rot);
			
			for (i = 0; i < num_inter; i++){
				x = t_x[i];
				y = t_y[i];
				
				x += center_x;
				y += center_y;
				
				/* rotation of ref point to align to ellipse axis */ 
				t_x[i] = cosine*(x - center_x) - sine*(y - center_y) + center_x;
				t_y[i] = sine*(x - center_x) + cosine*(y - center_y) + center_y;
				
				/* check if point pass on distance criteria */
				curr_dist = sqrt(pow(t_x[i] - pos_x, 2) + pow(t_y[i] - pos_y, 2));
				if (curr_dist < sensi){
					if (*init_dist == 0){
						*init_dist = 1;
						*min_dist = curr_dist;
						*ret_x = t_x[i];
						*ret_y = t_y[i];
						ret = ATRC_TAN;
					}
					else if (curr_dist < *min_dist){
						*min_dist = curr_dist;
						*ret_x = t_x[i];
						*ret_y = t_y[i];
						ret = ATRC_TAN;
					}
				}
			}
		}
		if (type & ATRC_PERP){ /* if type of attractor is flaged as perpenticular */
			/*find normal to an external point of ellipse*/
			double f[2], dx[2], dy[2], x, y, xnew, ynew, den;
			int num_inter = 0;
			
			/* rotation constants */
			cosine = cos(-rot);
			sine = sin(-rot);
			
			/* rotation of ref point to align to ellipse axis */
			x = cosine*(ref_x - center_x) - sine*(ref_y - center_y) + center_x;
			y = sine*(ref_x - center_x) + cosine*(ref_y - center_y) + center_y;
			
			/* translate point to ellipse center */
			x -= center_x;
			y -= center_y;
			
			/* equations parameters */
			double a2 = pow(a, 2);
			double b2 = pow(b, 2);
			double c1 = -b2*y;
			double c2 = b2-a2;
			double c3 = a2*x;
			double c4 = 1/a2;
			double c5 = 1/b2;
			
			/* Newton-Raphson method, in ten iterations*/
			for (i = 0; i < 10; i++){
				/* normal to ellipse*/
				f[0] = c1*x + c2*x*y + c3*y;
				/* partial derivates in point */
				dx[0] = c1 + c2*y;
				dy[0] = c2*x + c3;
				
				/*ellipse equation */
				f[1] = c4*x*x + c5*y*y -1;
				/* partial derivates in point */
				dx[1] = 2*c4*x;
				dy[1] = 2*c5*y;
				
				/* denominator of inverse jacobian matrix*/
				den = dx[0]*dy[1] - dy[0]*dx[1];
				if (fabs(den) > TOL){
					/* Newton-Raphson core => Pnew = P - F(P)*invJacob(P) */
					xnew = x - (f[0]*dy[1] - f[1]*dy[0])/den;
					ynew = y - (-f[0]*dx[1] + f[1]*dx[0])/den;
				}
				else break;
				if ((fabs(xnew - x) < TOL) && (fabs(ynew - y) < TOL)){
					/* has convergence*/
					num_inter = 1;
					x = xnew;
					y = ynew;
					break;
				}
				/* for next iteration */
				x = xnew;
				y = ynew;
			}
			if (num_inter){	
				
				/*rotate and translate back */
				/* rotation constants */
				cosine = cos(rot);
				sine = sin(rot);
				/* rotation of ref point to align to ellipse axis */ 
				xnew = cosine*x - sine*y + center_x;
				ynew = sine*x + cosine*y + center_y;
					
				//printf("perp  =  %0.2f, %0.2f\n", xnew, ynew);
			
				/* check if point pass on distance criteria */
				curr_dist = sqrt(pow(xnew - pos_x, 2) + pow(ynew - pos_y, 2));
				if (curr_dist < sensi){
					if (*init_dist == 0){
						*init_dist = 1;
						*min_dist = curr_dist;
						*ret_x = xnew;
						*ret_y = ynew;
						ret = ATRC_PERP;
					}
					else if (curr_dist < *min_dist){
						*min_dist = curr_dist;
						*ret_x = xnew;
						*ret_y = ynew;
						ret = ATRC_PERP;
					}
				}
			}
		}
	}
	return ret;
}

int seg_inter(struct inter_obj obj1, struct inter_obj obj2,
double *ret_x, double *ret_y){
	if ((obj1.type == DXF_LINE) && (obj2.type == DXF_LINE)){
			
		/* calcule the intersection point*/
		/*from https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection*/
		double den = (obj1.line.p1x - obj1.line.p2x) * (obj2.line.p1y - obj2.line.p2y) -
				(obj1.line.p1y - obj1.line.p2y) * (obj2.line.p1x - obj2.line.p2x);
		
		if (fabs(den) > TOL){
			*ret_x = ((obj1.line.p1x*obj1.line.p2y - obj1.line.p1y*obj1.line.p2x)*(obj2.line.p1x - obj2.line.p2x) -
					(obj1.line.p1x - obj1.line.p2x)*(obj2.line.p1x*obj2.line.p2y - obj2.line.p1y*obj2.line.p2x)) / den;
			*ret_y = ((obj1.line.p1x*obj1.line.p2y - obj1.line.p1y*obj1.line.p2x)*(obj2.line.p1y - obj2.line.p2y) -
					(obj1.line.p1y - obj1.line.p2y)*(obj2.line.p1x*obj2.line.p2y - obj2.line.p1y*obj2.line.p2x)) / den;
			/* verify if point is in segments */
			if (point_lies_seg(obj1.line.p1x, obj1.line.p1y, obj1.line.p2x, obj1.line.p2y, *ret_x, *ret_y) &&
			point_lies_seg(obj2.line.p1x, obj2.line.p1y, obj2.line.p2x, obj2.line.p2y, *ret_x, *ret_y)){
				return 1;
			}
		}
	}
	return 0;
}

int seg_inter2(double l1p1x, double l1p1y, double l1p2x, double l1p2y, 
double l2p1x, double l2p1y, double l2p2x, double l2p2y, 
double *ret_x, double *ret_y){
			
	/* calcule the intersection point*/
	/*from https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection*/
	double den = (l1p1x - l1p2x) * (l2p1y - l2p2y) -
			(l1p1y - l1p2y) * (l2p1x - l2p2x);
	
	if (fabs(den) > TOL){
		*ret_x = ((l1p1x*l1p2y - l1p1y*l1p2x)*(l2p1x - l2p2x) -
				(l1p1x - l1p2x)*(l2p1x*l2p2y - l2p1y*l2p2x)) / den;
		*ret_y = ((l1p1x*l1p2y - l1p1y*l1p2x)*(l2p1y - l2p2y) -
				(l1p1y - l1p2y)*(l2p1x*l2p2y - l2p1y*l2p2x)) / den;
		/* verify if point is in segments */
		if (point_lies_seg(l1p1x, l1p1y, l1p2x, l1p2y, *ret_x, *ret_y) &&
		point_lies_seg(l2p1x, l2p1y, l2p2x, l2p2y, *ret_x, *ret_y)){
			return 1;
		}
	}
	
	return 0;
}

int line_inter(double a1, double b1, double c1,
double a2, double b2, double c2,
double *ret_x, double *ret_y){
	double den =b1*a2-b2*a1;
	if (fabs(den) > TOL){
		*ret_x = (b1*c2-b2*c1)/den;
		*ret_y = (a1*c2-a2*c1)/-den;
		return 1;
	}
	return 0;
}

int el_ln_inter( double center_x, double center_y, 
double axis, double ratio, double rot,
double ln_a, double ln_b, double ln_c, /*line in general form ax+bx = c*/
double inter_x[2], double inter_y[2]){
	
	int num_inter = 0;
	
	/* ellipse's rotation constants */
	double cosine = cos(-rot);
	double sine = sin(-rot);
	/*ellipse's parameters */
	double a = axis, b = axis * ratio;
	
	/*calc params*/
	double k, l, m, n, p, q, r, delta = -1.0, s, d;
	
	if ((fabs(ln_b) < TOL) && (fabs(ln_a) > TOL)) { /* vertical line*/
		d = ln_c/ln_a;
		k = -sine; 
		m = cosine;
		l = center_y*sine - center_x*cosine + d*cosine;
		n = -center_y*cosine - center_x*sine + d*sine;
		
		p = pow(b,2)*pow(k,2) + pow(a,2)*pow(m,2);
		q = 2*(pow(b,2)*k*l + pow(a,2)*m*n);
		r = pow(b,2)*pow(l,2) + pow(a,2)*pow(n,2) - pow(a,2)*pow(b,2);
		delta = pow(q,2) - 4*p*r;
		
		if (p != 0.0){ /* exist intersection */
			if (delta > 0.0){ /* 2 intersections */
				num_inter = 2;
				inter_y[0] = (-q + sqrt(delta))/(2*p);
				inter_y[1] = (-q - sqrt(delta))/(2*p);
				inter_x[0] = d;
				inter_x[1] = d;
			}
			else if (fabs(delta) < TOL){ /* 1 intersection */
				num_inter = 1;
				inter_y[1] = (-q)/(2*p);
				inter_x[0] = d;
			}
		}
	}
	else if(fabs(ln_b) > TOL){
		s = -ln_a / ln_b;
		d = ln_c / ln_b;
		
		k = cosine - s*sine; 
		m = sine + s*cosine;
		l = center_y*sine - center_x*cosine - d*sine;
		n = -center_y*cosine - center_x*sine + d*cosine;
		
		p = pow(b,2)*pow(k,2) + pow(a,2)*pow(m,2);
		q = 2*(pow(b,2)*k*l + pow(a,2)*m*n);
		r = pow(b,2)*pow(l,2) + pow(a,2)*pow(n,2) - pow(a,2)*pow(b,2);
		delta = pow(q,2) - 4*p*r;
		
		if (p != 0.0){ /* exist intersection */
			if (delta > 0.0){ /* 2 intersections */
				num_inter = 2;
				inter_x[0] = (-q + sqrt(delta))/(2*p);
				inter_x[1] = (-q - sqrt(delta))/(2*p);
				inter_y[0] = s*inter_x[0] + d;
				inter_y[1] = s*inter_x[1] + d;
			}
			else if (fabs(delta) < TOL){ /* 1 intersection */
				num_inter = 1;
				inter_x[0] = (-q)/(2*p);
				inter_y[0] = s*inter_x[0] + d;
			}
		}
	}
	return num_inter;
}

int dxf_inter_attract(struct inter_obj obj1, struct inter_obj obj2,
double pos_x, double pos_y, double sensi, 
double *ret_x, double *ret_y,
int *init_dist, double *min_dist){
	int ret = ATRC_NONE;
	double curr_dist = sensi;
	double inter_x[2];
	double inter_y[2];
	int num_inter = 0, i;
	
	if ((obj1.type == DXF_LINE) && (obj2.type == DXF_LINE)){
		if (seg_inter(obj1, obj2, &inter_x[0], &inter_y[0])) num_inter = 1;
		
		//p1y - p2y, p2x - p1x, (p2x - p1x)*p1y + (p1y - p2y)*p1x,
		//if (line_inter(obj1.line.p1y - obj1.line.p2y, obj1.line.p2x - obj1.line.p1x, (obj1.line.p2x - obj1.line.p1x)*obj1.line.p1y + (obj1.line.p1y - obj1.line.p2y)*obj1.line.p1x,
		//obj2.line.p1y - obj2.line.p2y, obj2.line.p2x - obj2.line.p1x, (obj2.line.p2x - obj2.line.p1x)*obj2.line.p1y + (obj2.line.p1y - obj2.line.p2y)*obj2.line.p1x,
		//&inter_x[1], &inter_y[1])) printf("inter = %0.2f, %0.2f\n", inter_x[1], inter_y[1]);
	}
	else if (((obj1.type == DXF_LINE) && (obj2.type == DXF_ARC)) || ((obj1.type == DXF_ARC) && (obj2.type == DXF_LINE))){
		if (obj1.type == DXF_ARC){ /*swap objects (obj1 is always a line and obj2 is always an arc*/
			struct inter_obj tmp =obj1;
			obj1 = obj2;
			obj2 = tmp;
		}
		
		num_inter = el_ln_inter( obj2.arc.cx, obj2.arc.cy, 
		obj2.arc.axis, obj2.arc.ratio, obj2.arc.rot,
		obj1.line.p1y - obj1.line.p2y, obj1.line.p2x - obj1.line.p1x, (obj1.line.p2x - obj1.line.p1x)*obj1.line.p1y + (obj1.line.p1y - obj1.line.p2y)*obj1.line.p1x,
		inter_x, inter_y);
		
	}
	else if ((obj1.type == DXF_ARC) && (obj2.type == DXF_ARC)){
		double ang_cmp = obj2.arc.rot + M_PI;
		angle_range(&ang_cmp);
		
		/* verify if objects are not coincident */
		if (!(fabs(obj1.arc.axis - obj2.arc.axis) < TOL &&
		fabs(obj1.arc.ratio - obj2.arc.ratio) < TOL &&
		(fabs(obj1.arc.rot - obj2.arc.rot) < TOL || fabs(obj1.arc.rot - ang_cmp) < TOL)&&
		fabs(obj1.arc.cx - obj2.arc.cx) < TOL &&
		fabs(obj1.arc.cy - obj2.arc.cy) < TOL)){
			num_inter = 0;
			double x = pos_x, y = pos_y, xnew, ynew;
			
			double c1[2], c2[2], c3[2], c4[2], c5[2], c6[2];
			double a2, b2, cos1, cos2, sin1, sin2, xr, yr;
			double f[2], dx[2], dy[2], den;
			
			/* first arc parameters calculation */
			a2 = pow(obj1.arc.axis, 2);
			b2 = pow(obj1.arc.axis*obj1.arc.ratio, 2);
			cos1 = cos(-obj1.arc.rot); cos2 = pow(cos1, 2);
			sin1 = sin(-obj1.arc.rot); sin2 = pow(sin1, 2);
			xr = obj1.arc.cx*cos1 - obj1.arc.cy*sin1;
			yr = obj1.arc.cx*sin1 + obj1.arc.cy*cos1;
			/* parameters for ellipse equation in general form */
			c1[0] = cos2/a2 + sin2/b2;
			c2[0] = 2*sin1*cos1*(1/b2 - 1/a2);
			c3[0] = -2*(cos1*xr/a2 + sin1*yr/b2);
			c4[0] = cos2/b2 + sin2/a2;
			c5[0] = 2*(sin1*xr/a2 - cos1*yr/b2);
			c6[0] = pow(xr, 2)/a2 + pow(yr, 2)/b2 - 1;
			
			/* second arc parameters calculation */
			a2 = pow(obj2.arc.axis, 2);
			b2 = pow(obj2.arc.axis*obj2.arc.ratio, 2);
			cos1 = cos(-obj2.arc.rot); cos2 = pow(cos1, 2);
			sin1 = sin(-obj2.arc.rot); sin2 = pow(sin1, 2);
			xr = obj2.arc.cx*cos1 - obj2.arc.cy*sin1;
			yr = obj2.arc.cx*sin1 + obj2.arc.cy*cos1;
			/* parameters for ellipse equation in general form */
			c1[1] = cos2/a2 + sin2/b2;
			c2[1] = 2*sin1*cos1*(1/b2 - 1/a2);
			c3[1] = -2*(cos1*xr/a2 + sin1*yr/b2);
			c4[1] = cos2/b2 + sin2/a2;
			c5[1] = 2*(sin1*xr/a2 - cos1*yr/b2);
			c6[1] = pow(xr, 2)/a2 + pow(yr, 2)/b2 - 1;
			
			/* Newton-Raphson method, in ten iterations*/
			for (i = 0; i < 10; i++){
				/* first arc - ellipse equation */
				f[0] = c1[0]*x*x + c2[0]*x*y + c3[0]*x+ c4[0]*y*y + c5[0]*y + c6[0];
				/* partial derivates in point */
				dx[0] = 2*c1[0]*x + c2[0]*y + c3[0];
				dy[0] = c2[0]*x + 2*c4[0]*y + c5[0];
				
				/* second arc - ellipse equation */
				f[1] = c1[1]*x*x + c2[1]*x*y + c3[1]*x+ c4[1]*y*y + c5[1]*y + c6[1];
				/* partial derivates in point */
				dx[1] = 2*c1[1]*x + c2[1]*y + c3[1];
				dy[1] = c2[1]*x + 2*c4[1]*y + c5[1];
				
				/* denominator of inverse jacobian matrix*/
				den = dx[0]*dy[1] - dy[0]*dx[1];
				if (fabs(den) > TOL){
					/* Newton-Raphson core => Pnew = P - F(P)*invJacob(P) */
					xnew = x - (f[0]*dy[1] - f[1]*dy[0])/den;
					ynew = y - (-f[0]*dx[1] + f[1]*dx[0])/den;
				}
				else break;
				if ((fabs(xnew - x) < TOL) && (fabs(ynew - y) < TOL)){
					/* has convergence*/
					num_inter = 1;
					inter_x[0] = xnew;
					inter_y[0] = ynew;
					break;
				}
				/* for next iteration */
				x = xnew;
				y = ynew;
			}
		}
	}
	
	for (i = 0; i < num_inter; i++){
		curr_dist = sqrt(pow(inter_x[i] - pos_x, 2) + pow(inter_y[i] - pos_y, 2));
		if (curr_dist < sensi){
			//printf("INTER %0.2f, %0.2f\n", inter_x[i], inter_y[i]);
			if (*init_dist == 0){
				*init_dist = 1;
				*min_dist = curr_dist;
				*ret_x = inter_x[i];
				*ret_y = inter_y[i];
				ret = ATRC_INTER;
			}
			else if (curr_dist < *min_dist){
				*min_dist = curr_dist;
				*ret_x = inter_x[i];
				*ret_y = inter_y[i];
				ret = ATRC_INTER;
			}
		}
	}
	
	return ret;
}

int dxf_ent_attract (dxf_drawing *drawing, dxf_node * obj_hilite, enum attract_type type,
double pos_x, double pos_y, double ref_x, double ref_y, double sensi, double *ret_x, double *ret_y){
	dxf_node *current = NULL, *obj = NULL;
	dxf_node *prev = NULL;
	int ret = ATRC_NONE, found = 0;
	
	int init_dist = 0;
	double min_dist;
	
	enum dxf_graph ent_type = DXF_NONE;
	
	char name1[DXF_MAX_CHARS], name2[DXF_MAX_CHARS];
	name1[0] = 0; name2[0] = 0;
	double pt1_x = 0, pt1_y = 0, pt1_z = 0;
	int pt1 = 0; /* flag */
	double t_rot = 0, rot = 0, elev = 0;
	double scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
	double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0;
	
	/* for insert objects */
	dxf_node *insert_ent = NULL, *blk = NULL;
	
	struct ins_space ins_stack[10];
	int ins_stack_pos = 0;
	
	struct ins_space ins_zero = {
		.ins_ent = obj, .prev = NULL,
		.ofs_x = 0.0, .ofs_y =0.0, .ofs_z =0.0,
		.rot = 0.0, .scale_x = 1.0 , .scale_y = 1.0, .scale_z = 1.0,
		.normal = {0.0, 0.0, 1.0}
	};
	ins_stack[0] = ins_zero;
	
	int ins_flag = 0;
	/* ---- */
	
	double rect_pt1[2]; double rect_pt2[2];
	int num_el = 0;
	
	rect_pt1[0] = pos_x - sensi;
	rect_pt1[1] = pos_y - sensi;
	rect_pt2[0] = pos_x + sensi;
	rect_pt2[1] = pos_y + sensi;
	
	list_node * list = list_new(NULL, 1);
	list_node *list_el = NULL;
	
	num_el = dxf_ents_isect2(list, drawing, rect_pt1, rect_pt2);
	
	if ((list != NULL) && (num_el > 0)){
		list_el = list->next;
	}
	
	struct inter_obj inter_cand[MAX_CAND];
	int num_inter = 0;
		
	
	while (list_el){/* ###### LIST LOOP ########### */
		
		obj = (dxf_node *)list_el->data; /* current obj */
		
		/* reset variables for complex entities (DXF_INSERT) */
		insert_ent = NULL;
		blk = NULL;
		ins_stack_pos = 0;
		ins_zero.ins_ent = obj;
		ins_stack[0] = ins_zero;
		ins_flag = 0;
		ent_type = DXF_NONE;
		name1[0] = 0; name2[0] = 0;
		pt1_x = 0; pt1_y = 0; pt1_z = 0;
		pt1 = 0;
		t_rot = 0; rot = 0; elev = 0;
		scale_x = 1.0; scale_y = 1.0; scale_z = 1.0;
		extru_x = 0.0; extru_y = 0.0; extru_z = 1.0;
		
		current = obj;
		while (current){ /* ########### OBJ LOOP ########*/
			prev = current;
			if (current->type == DXF_ENT){
				ent_type =  dxf_ident_ent_type (current);
				if (ent_type == DXF_LINE){
					double pt1_x = 0, pt1_y = 0, pt1_z = 0;
					double pt2_x = 0, pt2_y = 0, pt2_z = 0;
					
					if (dxf_line_get (drawing, current, &pt1_x, &pt1_y, &pt1_z, &pt2_x, &pt2_y, &pt2_z)){
						/* transform coordinates, according insert space */
						transform(&pt1_x, &pt1_y, ins_stack[ins_stack_pos]);
						transform(&pt2_x, &pt2_y, ins_stack[ins_stack_pos]);
					
						if (found = dxf_line_attract (pt1_x, pt1_y, pt2_x, pt2_y, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
							ret = found;
						}
						if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
						(IN_BOUNDS(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y) ||
						NEAR_LN(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y, sensi))){
							
							inter_cand[num_inter].type = ent_type;
							inter_cand[num_inter].line.p1x = pt1_x;
							inter_cand[num_inter].line.p1y = pt1_y;
							inter_cand[num_inter].line.p2x = pt2_x;
							inter_cand[num_inter].line.p2y = pt2_y;
							inter_cand[num_inter].line.bulge = 0;
							num_inter++;
						}
					}
					//printf("line %d\n", found);
				}
				else if (ent_type == DXF_CIRCLE){
					double center_x, center_y, center_z, radius;
					double axis = 0, rot = 0, ratio = 1;
					
					if (dxf_circle_get(drawing, current, &center_x, &center_y, &center_z, &axis, &ratio, &rot)){
						/* transform coordinates, according insert space */
						transform(&center_x, &center_y, ins_stack[ins_stack_pos]);
						axis *= fabs(ins_stack[ins_stack_pos].scale_x);
						rot += ins_stack[ins_stack_pos].rot * M_PI/180;
						ratio *= fabs(ins_stack[ins_stack_pos].scale_y / ins_stack[ins_stack_pos].scale_x);
						ellipse_transf(&center_x, &center_y, &center_z, &axis, &ratio, &rot, ins_stack[ins_stack_pos].normal);
						if (found = dxf_arc_attract(axis, 0.0, 0.0, center_x, center_y, ratio, rot, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
							ret = found;
						}
						if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
						in_ellip_bound(pos_x, pos_y, sensi, center_x, center_y, axis, ratio, rot)){
							inter_cand[num_inter].type = DXF_ARC;
							inter_cand[num_inter].arc.cx = center_x;
							inter_cand[num_inter].arc.cy = center_y;
							inter_cand[num_inter].arc.axis = axis;
							inter_cand[num_inter].arc.ratio = ratio;
							inter_cand[num_inter].arc.rot = rot;
							inter_cand[num_inter].arc.ang_start = 0.0;
							inter_cand[num_inter].arc.ang_end = 2*M_PI;
							num_inter++;
						}
					}
				}
				else if (ent_type == DXF_ARC){
					double center_x, center_y, center_z, radius, start_ang, end_ang;
					double axis = 0, rot = 0, ratio = 1;
					//if (dxf_arc_get(drawing, current, &center_x, &center_y, &center_z, &radius, &start_ang, &end_ang)){
					if (dxf_arc_get(drawing, current, &center_x, &center_y, &center_z, &axis, &ratio, &rot, &start_ang, &end_ang)){
						/* transform coordinates, according insert space */
						transform(&center_x, &center_y, ins_stack[ins_stack_pos]);
						axis *= fabs(ins_stack[ins_stack_pos].scale_x);
						rot += ins_stack[ins_stack_pos].rot * M_PI/180;
						ratio *= fabs(ins_stack[ins_stack_pos].scale_y / ins_stack[ins_stack_pos].scale_x);
						ellipse_transf(&center_x, &center_y, &center_z, &axis, &ratio, &rot, ins_stack[ins_stack_pos].normal);
						if (found = dxf_arc_attract(axis, start_ang, end_ang, center_x, center_y, ratio, rot, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
							ret = found;
						}
						if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
						in_ellip_bound(pos_x, pos_y, sensi, center_x, center_y, axis, ratio, rot)){
							inter_cand[num_inter].type = DXF_ARC;
							inter_cand[num_inter].arc.cx = center_x;
							inter_cand[num_inter].arc.cy = center_y;
							inter_cand[num_inter].arc.axis = axis;
							inter_cand[num_inter].arc.ratio = ratio;
							inter_cand[num_inter].arc.rot = rot;
							inter_cand[num_inter].arc.ang_start = start_ang;
							inter_cand[num_inter].arc.ang_end = end_ang;
							num_inter++;
						}
					}
				}
				else if (ent_type == DXF_ELLIPSE){
					double center_x, center_y, center_z, radius, start_ang, end_ang;
					double axis = 0, rot = 0, ratio = 1;
					//if (dxf_arc_get(drawing, current, &center_x, &center_y, &center_z, &radius, &start_ang, &end_ang)){
					if (dxf_ellipse_get(drawing, current, &center_x, &center_y, &center_z, &axis, &ratio, &rot, &start_ang, &end_ang)){
						/* transform coordinates, according insert space */
						transform(&center_x, &center_y, ins_stack[ins_stack_pos]);
						axis *= fabs(ins_stack[ins_stack_pos].scale_x);
						rot += ins_stack[ins_stack_pos].rot * M_PI/180;
						ratio *= fabs(ins_stack[ins_stack_pos].scale_y / ins_stack[ins_stack_pos].scale_x);
						ellipse_transf(&center_x, &center_y, &center_z, &axis, &ratio, &rot, ins_stack[ins_stack_pos].normal);
						if (found = dxf_arc_attract(axis, start_ang, end_ang, center_x, center_y, ratio, rot, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
							ret = found;
						}
						if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
						in_ellip_bound(pos_x, pos_y, sensi, center_x, center_y, axis, ratio, rot)){
							inter_cand[num_inter].type = DXF_ARC;
							inter_cand[num_inter].arc.cx = center_x;
							inter_cand[num_inter].arc.cy = center_y;
							inter_cand[num_inter].arc.axis = axis;
							inter_cand[num_inter].arc.ratio = ratio;
							inter_cand[num_inter].arc.rot = rot;
							inter_cand[num_inter].arc.ang_start = start_ang;
							inter_cand[num_inter].arc.ang_end = end_ang;
							num_inter++;
						}
					}
				}
				else if (ent_type ==  DXF_INSERT){
					insert_ent = current;
					ins_flag = 1;
					//printf("insert %d\n", current->obj.content);
					if (current->obj.content){
						/* starts the content sweep */
						current = current->obj.content->next;
						//prev = current;
						continue;
					}
					printf("Error: empty entity\n");
				}
				else if (ent_type == DXF_LWPOLYLINE){
					double pt1_x = 0, pt1_y = 0, pt1_z = 0, bulge = 0;
					double pt2_x = 0, pt2_y = 0, pt2_z = 0, prev_bulge = 0;
					dxf_node * next_vert = NULL;
					if(dxf_lwpline_get_pt(drawing, current, &next_vert, &pt2_x, &pt2_y, &pt2_z, &bulge)){
						//printf("%0.f,%0.2f  -  %d\n",pt2_x, pt2_y, next_vert);
						/* transform coordinates, according insert space */
						while (next_vert){
							transform(&pt2_x, &pt2_y, ins_stack[ins_stack_pos]);
							pt1_x = pt2_x; pt1_y = pt2_y; prev_bulge = bulge;
							
							if(!dxf_lwpline_get_pt(drawing, current, &next_vert, &pt2_x, &pt2_y, &pt2_z, &bulge)){
								break;
							}
							
							if (fabs(prev_bulge) < TOL){ /* segment is a straight line*/
								if (found = dxf_line_attract (pt1_x, pt1_y, pt2_x, pt2_y, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
									ret = found;
								}
								if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
								(IN_BOUNDS(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y) ||
								NEAR_LN(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y, sensi))){
									
									inter_cand[num_inter].type = DXF_LINE;
									inter_cand[num_inter].line.p1x = pt1_x;
									inter_cand[num_inter].line.p1y = pt1_y;
									inter_cand[num_inter].line.p2x = pt2_x;
									inter_cand[num_inter].line.p2y = pt2_y;
									inter_cand[num_inter].line.bulge = 0;
									num_inter++;
								}
							}
							else{ /* segment is an arc*/
								double radius, ang_start, ang_end, center_x, center_y, center_z = 0.0;
								double axis = 0, rot = 0, ratio = 1;
								arc_bulge(pt1_x, pt1_y, pt2_x, pt2_y, prev_bulge, &radius, &ang_start, &ang_end, &center_x, &center_y);
								/* transform coordinates, according insert space */
								transform(&center_x, &center_y, ins_stack[ins_stack_pos]);
								axis = radius * fabs(ins_stack[ins_stack_pos].scale_x);
								rot = ins_stack[ins_stack_pos].rot * M_PI/180;
								ratio = fabs(ins_stack[ins_stack_pos].scale_y / ins_stack[ins_stack_pos].scale_x);
								ellipse_transf(&center_x, &center_y, &center_z, &axis, &ratio, &rot, ins_stack[ins_stack_pos].normal);
								if (found = dxf_arc_attract(axis, ang_start, ang_end, center_x, center_y, ratio, rot, type, pos_x, pos_y, ref_x, ref_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
									ret = found;
								}
								if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
								in_ellip_bound(pos_x, pos_y, sensi, center_x, center_y, axis, ratio, rot)){
									inter_cand[num_inter].type = DXF_ARC;
									inter_cand[num_inter].arc.cx = center_x;
									inter_cand[num_inter].arc.cy = center_y;
									inter_cand[num_inter].arc.axis = axis;
									inter_cand[num_inter].arc.ratio = ratio;
									inter_cand[num_inter].arc.rot = rot;
									inter_cand[num_inter].arc.ang_start = ang_start;
									inter_cand[num_inter].arc.ang_end = ang_end;
									num_inter++;
								}
							}
						}
					}
				}
				else if (ent_type == DXF_TEXT){
					char text[DXF_MAX_CHARS];
					double ins_x, ins_y, ins_z;
					double alin_x, alin_y, alin_z; 
					double w, h, rot, above, below;
					int alin_v, alin_h, fnt_idx;
					
					if (dxf_text_get (drawing, current, 
					text, &fnt_idx, &above, &below,
					&ins_x, &ins_y, &ins_z, 
					&alin_x, &alin_y, &alin_z, 
					&w, &h, &rot, 
					&alin_v, &alin_h)){
						/* transform coordinates, according insert space */
						transform(&ins_x, &ins_y, ins_stack[ins_stack_pos]);
						transform(&alin_x, &alin_y, ins_stack[ins_stack_pos]);
						
						//printf ("text w=%0.2f, h=%0.2f\n", w, h);
					
						if (found = dxf_text_attract (ins_x, ins_y, alin_x, alin_y, w, h, rot, alin_v, alin_h, above, below, type, pos_x, pos_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
							ret = found;
						}
						/*
						if ((type & ATRC_INTER) && (num_inter < MAX_CAND) &&
						(IN_BOUNDS(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y) ||
						NEAR_LN(pos_x, pos_y, pt1_x, pt1_y, pt2_x, pt2_y, sensi))){
							
							inter_cand[num_inter].type = ent_type;
							inter_cand[num_inter].line.p1x = pt1_x;
							inter_cand[num_inter].line.p1y = pt1_y;
							inter_cand[num_inter].line.p2x = pt2_x;
							inter_cand[num_inter].line.p2y = pt2_y;
							inter_cand[num_inter].line.bulge = 0;
							num_inter++;
						}*/
					}
					//printf("line %d\n", found);
				}
				
			}
			/* ============================================================= */
			else if ((current->type == DXF_ATTR) && (ins_flag != 0)){ /* read DXF attibutes of insert block */
				//printf("%d\n", current->value.group);
				switch (current->value.group){
					case 2:
						strcpy(name1, current->value.s_data);
						break;
					case 3:
						strcpy(name2, current->value.s_data);
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
				
			current = current->next; /* go to the next in the list*/
			
			/* ============================================================= */
			/* complex entities */
			
			if (((ins_flag != 0) && (current == NULL))||
				((ins_flag != 0) && (current != NULL) && (current != insert_ent) && (current->type == DXF_ENT))){
				ins_flag = 0;
				/* look for block */
				blk = dxf_find_obj_descr2(drawing->blks, "BLOCK", name1);
				if (blk) { 
					
					//printf ("bloco %s\n", name1);
					
					/* save current entity for future process */
					ins_stack_pos++;
					ins_stack[ins_stack_pos].ins_ent = blk;
					ins_stack[ins_stack_pos].prev = prev;
					
					if (ins_stack_pos > 1){
						ins_stack[ins_stack_pos].ofs_x = pt1_x + ins_stack[ins_stack_pos - 1].ofs_x;
						ins_stack[ins_stack_pos].ofs_y = pt1_y + ins_stack[ins_stack_pos - 1].ofs_y;
						ins_stack[ins_stack_pos].ofs_z = pt1_z + ins_stack[ins_stack_pos - 1].ofs_z;
						ins_stack[ins_stack_pos].scale_x = scale_x * ins_stack[ins_stack_pos - 1].scale_x;
						ins_stack[ins_stack_pos].scale_y = scale_y * ins_stack[ins_stack_pos - 1].scale_y;
						ins_stack[ins_stack_pos].scale_z = scale_z * ins_stack[ins_stack_pos - 1].scale_z;
						ins_stack[ins_stack_pos].rot = t_rot + ins_stack[ins_stack_pos - 1].rot;
						ins_stack[ins_stack_pos].normal[0] = extru_x;
						ins_stack[ins_stack_pos].normal[1] = extru_y;
						ins_stack[ins_stack_pos].normal[2] = extru_z;
					}
					else{ 
						ins_stack[ins_stack_pos].ofs_x = pt1_x;
						ins_stack[ins_stack_pos].ofs_y = pt1_y;
						ins_stack[ins_stack_pos].ofs_z = pt1_z;
						ins_stack[ins_stack_pos].scale_x = scale_x;
						ins_stack[ins_stack_pos].scale_y = scale_y;
						ins_stack[ins_stack_pos].scale_z = scale_z;
						ins_stack[ins_stack_pos].rot = t_rot;
						ins_stack[ins_stack_pos].normal[0] = extru_x;
						ins_stack[ins_stack_pos].normal[1] = extru_y;
						ins_stack[ins_stack_pos].normal[2] = extru_z;
					}
					/*
					if (v_return->size > 0){
						ins_stack[ins_stack_pos].start_idx = v_return->size;
					}
					else{
						ins_stack[ins_stack_pos].start_idx = 0;
					}*/
					
					
					//p_space = paper;
					
					/*reinit_vars: */
					
					ent_type = DXF_NONE;
						
					pt1_x = 0; pt1_y = 0; pt1_z = 0; rot = 0;
					elev = 0; t_rot = 0;
					scale_x = 1.0; scale_y = 1.0; scale_z = 1.0;
					extru_x = 0.0; extru_y = 0.0; extru_z = 1.0;
					
					/* clear the strings */
					name1[0] = 0;
					name2[0] = 0;
					
					/*clear flags*/
					pt1 = 0;
					
					if (blk->obj.content){
						/* now, current is the block */
						/* starts the content sweep */
						current = blk->obj.content->next;
						continue;
					}
					printf("Error: empty block\n");
					continue;
				}
			}
			
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
					if (prev == ins_stack[ins_stack_pos].ins_ent){/* back on initial entity */
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
		} /* ######### END OBJ LOOP ########### */
		
		list_el = list_el->next;
		
	}/* ######### END LIST LOOP ############ */
	list_mem_pool(ZERO_LIST, 1);
	
	/* verify if exist intersections between candidates*/
	if(num_inter > 1){
		int i, j;
		/* walk in all combinations */
		for (i = 0; i < num_inter - 1; i++){
			for (j = i+1; j < num_inter; j++){
				if (found = dxf_inter_attract (inter_cand[i], inter_cand[j], pos_x, pos_y, sensi, ret_x, ret_y, &init_dist, &min_dist)){
					ret = found;
				}
			}
		}
	}
	//printf("%d\n", ret);
	return ret;
}