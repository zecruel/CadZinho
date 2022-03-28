#include "dxf_hatch.h"

int dxf_parse_patt(char *buf, struct h_pattern *ret){
	static char *line, *cur_line, *next_line, line_buf[DXF_MAX_CHARS];
	int line_size;
	
	cur_line = buf;
	
	char s[2] = ",";
	char *token;
	
	double ang, org_x, org_y, delta_x, delta_y, dashes[20];
	int num_dash;
	int pos = 0, init = 0, i;
	
	struct h_pattern *curr_h = ret;
	struct hatch_line **curr_l = NULL;
	
	while(cur_line){
		next_line = strchr(cur_line, '\n');
		if (next_line) {
			line_size = next_line - cur_line;
			if (line_size > DXF_MAX_CHARS - 1) line_size = DXF_MAX_CHARS - 1;
			memcpy(line_buf, cur_line, line_size);
			line_buf[line_size] = '\0';  /*terminate the current line*/
		}
		else{
			strncpy(line_buf, cur_line, DXF_MAX_CHARS);
		}
		
		/* trim leading/trailing whitespace of line */
		line = trimwhitespace(line_buf);
		s[0] = ','; s[1] = 0; /* standard separator*/
		
		if (line[0] == '*'){
			/* get the first token */
			token = strtok(line, s);
			pos = 0;
			init = 0;
			
			/* walk through other tokens */
			while( token != NULL ) {
				token = trimwhitespace(token);
				
				if (pos == 0) {
					if (curr_h){
						curr_h->next = malloc(sizeof(struct h_pattern));
						if (curr_h->next) {
							curr_h = curr_h->next;
							curr_h->next = NULL;
							curr_h->lines = NULL;
							curr_h->name[0] = 0; /*clear strings*/
							curr_h->descr[0] = 0;
							curr_h->num_lines = 0;
							
							curr_l = &(curr_h->lines);
							
							init = 1;
						}
					}
					if (init){
						token++; /*skip '*' in name */
						strncpy(curr_h->name, token, DXF_MAX_CHARS);
						s[0] = '\n'; s[1] = 0; /* change separator*/
					}
				}
				else {
					if (init) strncpy(curr_h->descr, token, DXF_MAX_CHARS);
				}

				token = strtok(NULL, s);
				pos++;
			}
		}
		else if((init) && ((isdigit(line[0])) || (line[0] == '.') || (line[0] == '-'))){
			/* get the first token */
			token = strtok(line, s);
			pos = 0;
			num_dash = 0;
			
			/* walk through other tokens */
			while( token != NULL ) {
				token = trimwhitespace(token);
				
				if (pos == 0) {
					if (curr_l){
						*curr_l = malloc(sizeof(struct hatch_line));
						if(*curr_l) (*curr_l)->next = NULL;
					}
					
					ang = atof(token);
				}
				else if (pos == 1) {
					org_x = atof(token);
				}
				else if (pos == 2) {
					org_y = atof(token);
				}
				else if (pos == 3) {
					delta_x = atof(token);
				}
				else if (pos == 4) {
					delta_y = atof(token);
				}
				else if ((pos > 4) && (num_dash < 20)){
					dashes[num_dash] = atof(token);
					//printf( "%0.2f, ", dashes[num_dash] );
					num_dash++;
				}

				token = strtok(NULL, s);
				pos++;
			}
			if(*curr_l){
				(*curr_l)->ang = ang;
				(*curr_l)->ox = org_x;
				(*curr_l)->oy = org_y;
				(*curr_l)->dx = delta_x;
				(*curr_l)->dy = delta_y;
				(*curr_l)->num_dash = num_dash;
				for (i = 0; i < num_dash; i++){
					(*curr_l)->dash[i] = dashes[i];
				}
				
				curr_h->num_lines++;
				
				curr_l = &((*curr_l)->next);
				//printf("%s|%s|%0.2f|%0.2f|%0.2f|%0.2f|%0.2f|%d\n", name, descr, ang, org_x, org_y, delta_x, delta_y, num_dash);
			}
		}
		else init = 0;
		
		cur_line = next_line ? (next_line+1) : NULL;
	}
	return 1;
}

int dxf_hatch_free (struct h_pattern *hatch){
	struct h_pattern *curr_h = NULL;
	struct h_pattern *next_h = NULL;
	struct hatch_line *curr_l = NULL;
	struct hatch_line *next_l = NULL;
	
	curr_h = hatch;
	
	while (curr_h){
		//printf("Name = %s, Descr: %s\n", curr_h->name, curr_h->descr);
		curr_l = curr_h->lines;
		
		while (curr_l){
			next_l = curr_l->next;
			free(curr_l);
			curr_l = next_l;
		}
		
		
		next_h = curr_h->next;
		free(curr_h);
		curr_h = next_h;
	}
	return 1;
}

struct h_family * dxf_hatch_family(char *name, char* descr, char *buf){
	struct h_family *ret = NULL;
	struct h_pattern *list = NULL;
	
	if (buf == NULL) return NULL; /* error: file not loaded*/
	
	/* initialize structures*/
	ret = malloc(sizeof(struct h_family));
	if (ret == NULL) return NULL; /* return NULL on error*/
	list = malloc(sizeof(struct h_pattern));
	if (list == NULL){
		free (ret);
		return NULL; /* return NULL on error*/
	}
	list->next = NULL;
	list->lines = NULL;
	ret->list = list;
	ret->next = NULL;
	if (name) strncpy(ret->name, name, DXF_MAX_CHARS);
	else strncpy(ret->name, "Untitled", DXF_MAX_CHARS);
	if (descr) strncpy(ret->descr, descr, DXF_MAX_CHARS);
	else ret->descr[0] = 0;
	
	dxf_parse_patt(buf, list);
	
	return ret;
}

struct h_family * dxf_hatch_family_file(char *name, char *path){
	long fsize;
	struct h_family *ret = NULL;
	
	if (path == NULL) return NULL; /* error: no path*/
	/* load file */
	struct Mem_buffer * buf = load_file_reuse(path, &fsize);
	
	ret = dxf_hatch_family(name, path, buf->buffer);
	//free(buf);
	manage_buffer(0, BUF_RELEASE);
	
	return ret;
}

int dxf_h_fam_free (struct h_family *fam){
	struct h_family *curr = NULL;
	struct h_family *next = NULL;
	
	curr = fam;
	
	while (curr){
		dxf_hatch_free (curr->list);
		
		next = curr->next;
		free(curr);
		curr = next;
		
	}
	return 1;
}

int dxf_hatch_bound (dxf_node *hatch, list_node *list){
	if (!hatch) return 0;
	if (!list) return 0;
	
	dxf_node * obj = NULL;
	int loops = 0, ok = 1, pool = 0;
	
	pool = hatch->obj.pool;
	/*============== bondaries =============*/
	list_node *current = list->next;
	while (current != NULL){
		if (current->data){
			obj = (dxf_node *)current->data;
			int type = dxf_ident_ent_type(obj);
			if (type == DXF_LWPOLYLINE){
				int closed = 0, num_vert = 0;
				
				dxf_node *closed_o = dxf_find_attr_i(obj, 70, 0);
				if (closed_o) closed = closed_o->value.i_data & 1;
				
				if (closed){
					
					/* boundary type */
					ok &= dxf_attr_append(hatch, 92, (void *) (int[]){2}, pool); /* polyline*/
					ok &= dxf_attr_append(hatch, 72, (void *) (int[]){1}, pool); /* has bulge*/
					
					
					
					dxf_node *num_vert_o = dxf_find_attr_i(obj, 90, 0);
					if (num_vert_o) num_vert = num_vert_o->value.i_data;
					
					ok &= dxf_attr_append(hatch, 73, (void *) &closed, pool);
					ok &= dxf_attr_append(hatch, 93, (void *) &num_vert, pool);
					
					dxf_node *curr_attr = obj->obj.content->next;
					double x = 0.0, y = 0.0, bulge = 0.0, prev_x = 0.0;
					int vert = 0, first = 0;
					while (curr_attr){
						switch (curr_attr->value.group){
							case 10:
								x = curr_attr->value.d_data;
								vert = 1; /* set flag */
								break;
							case 20:
								y = curr_attr->value.d_data;
								break;
							case 42:
								bulge = curr_attr->value.d_data;
								break;
						}
						
						if (vert){
							if (!first) first = 1;
							else{
								ok &= dxf_attr_append(hatch, 10, (void *) &prev_x, pool);
								ok &= dxf_attr_append(hatch, 20, (void *) &y, pool);
								ok &= dxf_attr_append(hatch, 42, (void *) &bulge, pool);
							}
							prev_x = x;
							bulge = 0.0; vert = 0;
						}
						
						//x = 0; y = 0; bulge = 0; vert = 0;
						curr_attr = curr_attr->next;
					}
					/* last vertex */
					ok &= dxf_attr_append(hatch, 10, (void *) &prev_x, pool);
					ok &= dxf_attr_append(hatch, 20, (void *) &y, pool);
					ok &= dxf_attr_append(hatch, 42, (void *) &bulge, pool);
					
					/* number of source boundary objects - ?? */
					ok &= dxf_attr_append(hatch, 97, (void *) (int[]){0}, pool);
					
					if(ok) loops++;
				}
				
				list_remove(list, current); /* remove from list - processed entity */
			}
			else if (type == DXF_CIRCLE){
				/* boundary type */
				ok &= dxf_attr_append(hatch, 92, (void *) (int[]){1}, pool); /* not polyline*/
				ok &= dxf_attr_append(hatch, 93, (void *) (int[]){1}, pool); /* number of edges = 1*/
				ok &= dxf_attr_append(hatch, 72, (void *) (int[]){2}, pool); /* circular arc*/
				
				dxf_node *curr_attr = obj->obj.content->next;
				double x = 0.0, y = 0.0, radius = 0.0;
				while (curr_attr){
					switch (curr_attr->value.group){
						case 10:
							x = curr_attr->value.d_data;
							break;
						case 20:
							y = curr_attr->value.d_data;
							break;
						case 40:
							radius = curr_attr->value.d_data;
							break;
					}
					curr_attr = curr_attr->next;
				}
				/* center */
				ok &= dxf_attr_append(hatch, 10, (void *) &x, pool);
				ok &= dxf_attr_append(hatch, 20, (void *) &y, pool);
				/* radius */
				ok &= dxf_attr_append(hatch, 40, (void *) &radius, pool);
				/* start angle = 0 */
				ok &= dxf_attr_append(hatch, 50, (void *) (double[]){0.0}, pool);
				/* end angle = 0 */
				ok &= dxf_attr_append(hatch, 51, (void *) (double[]){360.0}, pool);
				/* counterclockwise flag */
				ok &= dxf_attr_append(hatch, 73, (void *) (int[]){0}, pool);
				
				/* number of source boundary objects - ?? */
				ok &= dxf_attr_append(hatch, 97, (void *) (int[]){0}, pool);
				
				if(ok) loops++;
				
				list_remove(list, current);  /* remove from list - processed entity */
			}
			else if (type == DXF_ELLIPSE){
				dxf_node *curr_attr = obj->obj.content->next;
				double x0 = 0.0, y0 = 0.0, ratio = 1.0;
				double x1 = 0.0, y1 = 0.0;
				double start = 0.0, end = 2 * M_PI;
				while (curr_attr){
					switch (curr_attr->value.group){
						case 10:
							x0 = curr_attr->value.d_data;
							break;
						case 20:
							y0 = curr_attr->value.d_data;
							break;
						case 11:
							x1 = curr_attr->value.d_data;
							break;
						case 21:
							y1 = curr_attr->value.d_data;
							break;
						case 40:
							ratio = curr_attr->value.d_data;
							break;
						case 41:
							start = curr_attr->value.d_data;
							break;
						case 42:
							end = curr_attr->value.d_data;
							break;
					}
					curr_attr = curr_attr->next;
				}
				
				if (fabs(start) < 1e-9 && fabs(end - 2 * M_PI) < 1e-9) { /* full ellipse */
					/* boundary type */
					ok &= dxf_attr_append(hatch, 92, (void *) (int[]){1}, pool); /* not polyline*/
					ok &= dxf_attr_append(hatch, 93, (void *) (int[]){1}, pool); /* number of edges = 1*/
					ok &= dxf_attr_append(hatch, 72, (void *) (int[]){3}, pool); /* elliptic arc*/
					
					/* center */
					ok &= dxf_attr_append(hatch, 10, (void *) &x0, pool);
					ok &= dxf_attr_append(hatch, 20, (void *) &y0, pool);
					/* major axis point */
					ok &= dxf_attr_append(hatch, 11, (void *) &x1, pool);
					ok &= dxf_attr_append(hatch, 21, (void *) &y1, pool);
					/* minor axis ratio */
					ok &= dxf_attr_append(hatch, 40, (void *) &ratio, pool);
					/* start angle = 0 */
					ok &= dxf_attr_append(hatch, 50, (void *) (double[]){0.0}, pool);
					/* end angle = 0 */
					ok &= dxf_attr_append(hatch, 51, (void *) (double[]){360.0}, pool);
					/* counterclockwise flag */
					ok &= dxf_attr_append(hatch, 73, (void *) (int[]){0}, pool);
					
					/* number of source boundary objects - ?? */
					ok &= dxf_attr_append(hatch, 97, (void *) (int[]){0}, pool);
					
					if(ok) loops++;
				}
				list_remove(list, current);  /* remove from list - processed entity */
			}
			else if (type == DXF_SPLINE){
				int flags = 0;
				
				dxf_node *tmp = dxf_find_attr_i(obj, 70, 0);
				if (tmp) flags = tmp->value.i_data;
				
				if (flags  & 1){ /* closed spline */
					int degree = 3, rational = 0, periodic = 0, knots = 0, ctrl = 0;
					periodic = (flags & 2) ? 1 : 0;
					rational = (flags & 4) ? 1 : 0;
					
					tmp = dxf_find_attr_i(obj, 71, 0);
					if (tmp) degree = tmp->value.i_data;
					tmp = dxf_find_attr_i(obj, 72, 0);
					if (tmp) knots = tmp->value.i_data;
					tmp = dxf_find_attr_i(obj, 73, 0);
					if (tmp) ctrl = tmp->value.i_data;
					
					/* boundary type */
					ok &= dxf_attr_append(hatch, 92, (void *) (int[]){1}, pool); /* not polyline*/
					ok &= dxf_attr_append(hatch, 93, (void *) (int[]){1}, pool); /* number of edges = 1*/
					ok &= dxf_attr_append(hatch, 72, (void *) (int[]){4}, pool); /* spline */
						
					ok &= dxf_attr_append(hatch, 94, (void *) &degree, pool);
					ok &= dxf_attr_append(hatch, 73, (void *) &rational, pool);
					ok &= dxf_attr_append(hatch, 74, (void *) &periodic, pool);
					ok &= dxf_attr_append(hatch, 95, (void *) &knots, pool);
					ok &= dxf_attr_append(hatch, 96, (void *) &ctrl, pool);
					
					dxf_node *curr_attr = obj->obj.content->next;
					double x = 0.0, y = 0.0, knot = 0.0, weight = 0.0;
					while (curr_attr){
						switch (curr_attr->value.group){
							case 40:
								knot = curr_attr->value.d_data;
								ok &= dxf_attr_append(hatch, 40, (void *) &knot, pool);
								break;
							case 10:
								x = curr_attr->value.d_data;
								ok &= dxf_attr_append(hatch, 10, (void *) &x, pool);
								break;
							case 20:
								y = curr_attr->value.d_data;
								ok &= dxf_attr_append(hatch, 20, (void *) &y, pool);
								break;
							case 42:
								weight = curr_attr->value.d_data;
								ok &= dxf_attr_append(hatch, 42, (void *) &weight, pool);
								break;
						}
						
						curr_attr = curr_attr->next;
					}
					
					/* number of source boundary objects - ?? */
					ok &= dxf_attr_append(hatch, 97, (void *) (int[]){0}, pool);
					
					if(ok) loops++;
				}
				
				list_remove(list, current); /* remove from list - processed entity */
			}
			else{
				list_remove(list, current);  /* remove from list - not supported entity */
			}
		}
		else{
			list_remove(list, current);  /* remove from list - invalid data */
		}
		current = list->next; /*reinit list sweep */
	}
	
	/*===================================*/
	
	return loops;
}