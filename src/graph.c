#include "graph.h"
#define GRAPH_PAGE 10000
#define LINE_PAGE 10000
#define GRAPH_NUM_POOL 5
#define TOLERANCE 1e-9

int cmp_double(const void * a, const void * b) {
	if (*(double*)a > *(double*)b)
		return 1;
	else if (*(double*)a < *(double*)b)
		return -1;
	else
		return 0;  
}

int cmp_vert(const void * a, const void * b) {
	double pos1 = *(double *)(((struct sort_by_idx *)a)->data);
	double pos2 = *(double *)(((struct sort_by_idx *)b)->data);
	if (pos1 < pos2)
		return 1;
	else if (pos1 > pos2)
		return -1;
	else
		return 0;
}

int get_i(int pos, int len, int rev){
	if (!rev) return pos;
	else return len - 1 - pos;
}

int cmp_color(bmp_color color1, bmp_color color2){
	/* compare colors, by RGB values */
	return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b;
}

bmp_color validate_color (bmp_color color, bmp_color list[], bmp_color subst[], int len){
	/* verify if color is in substitution list and return a valid color */
	
	/* if not substitution list, return default color */
	if (subst == NULL) return color;
	/* if not indicate especific colors, every color is substituted by first in list */
	if (list == NULL) return subst[0];
	
	int i;
	for (i = 0; i < len; i++){ /* sweep the list */
		if (cmp_color(color, list[i])){
			/* if in list, get the relative substitute */
			return subst[i];
		}
	}
	return color; /* return default color */
}

void * graph_mem_pool2(enum graph_pool_action action){
	
	static graph_pool_slot graph, line;
	int i;
	
	void *ret_ptr = NULL;
	
	/* initialize the graph pool, the first allocation */
	if (graph.size < 1){
		graph.pool[0] = malloc(GRAPH_PAGE * sizeof(graph_obj));
		if (graph.pool){
			graph.size = 1;
			//printf("Init graph\n");
		}
	}
	
	/* initialize the lines pool, the first allocation */
	if (line.size < 1){
		line.pool[0] = malloc(LINE_PAGE * sizeof(line_node));
		if (line.pool){
			line.size = 1;
			//printf("Init line\n");
		}
	}
	
	/* if current page is full */
	if ((graph.pos >= GRAPH_PAGE) && (graph.size > 0)){
		/* try to change to page previuosly allocated */
		if (graph.page < graph.size - 1){
			graph.page++;
			graph.pos = 0;
			//printf("change graph page\n");
		}
		/* or then allocatte a new page */
		else if(graph.page < GRAPH_POOL_PAGES-1){
			graph.pool[graph.page + 1] = malloc(GRAPH_PAGE * sizeof(graph_obj));
			if (graph.pool[graph.page + 1]){
				graph.page++;
				graph.size ++;
				graph.pos = 0;
				//printf("Realloc graph\n");
			}
		}
	}
	
	/* if current page is full */
	if ((line.pos >= LINE_PAGE) && (line.size > 0)){
		/* try to change to page previuosly allocated */
		if (line.page < line.size - 1){
			line.page++;
			line.pos = 0;
			//printf("change line page\n");
		}
		/* or then allocatte a new page */
		else if(line.page < GRAPH_POOL_PAGES-1){
			line.pool[line.page + 1] = malloc(LINE_PAGE * sizeof(line_node));
			if (line.pool[line.page + 1]){
				line.page++;
				line.size ++;
				line.pos = 0;
				//printf("Realloc line\n");
			}
		}
	}
	
	ret_ptr = NULL;
	
	if ((graph.pool[graph.page] != NULL) &&  (line.pool[line.page] != NULL)){
		switch (action){
			case ADD_GRAPH:
				if (graph.pos < GRAPH_PAGE){
					ret_ptr = &(((graph_obj *)graph.pool[graph.page])[graph.pos]);
					graph.pos++;
				}
				break;
			case ADD_LINE:
				if (line.pos < LINE_PAGE){
					ret_ptr = &(((line_node *)line.pool[line.page])[line.pos]);
					line.pos++;
				}
				break;
			case ZERO_GRAPH:
				graph.pos = 0;
				graph.page = 0;
				break;
			case ZERO_LINE:
				line.pos = 0;
				line.page = 0;
				break;
			case FREE_ALL:
				for (i = 0; i < graph.size; i++){
					free(graph.pool[i]);
					graph.pool[i] = NULL;
				}
				graph.pos = 0;
				graph.page = 0;
				graph.size = 0;
				for (i = 0; i < line.size; i++){
					free(line.pool[i]);
					line.pool[i] = NULL;
				}
				line.pos = 0;
				line.page = 0;
				line.size = 0;
				break;
		}
	}
	return ret_ptr;
}

void * graph_mem_pool(enum graph_pool_action action, int idx){
	
	static graph_pool_slot graph[GRAPH_NUM_POOL], line[GRAPH_NUM_POOL];
	int i;
	
	void *ret_ptr = NULL;
	
	if ((idx >= 0) && (idx < GRAPH_NUM_POOL)){ /* check if index is valid */
		
		/* initialize the graph pool, the first allocation */
		if (graph[idx].size < 1){
			graph[idx].pool[0] = malloc(GRAPH_PAGE * sizeof(graph_obj));
			if (graph[idx].pool){
				graph[idx].size = 1;
				//printf("Init graph\n");
			}
		}
		
		/* initialize the lines pool, the first allocation */
		if (line[idx].size < 1){
			line[idx].pool[0] = malloc(LINE_PAGE * sizeof(line_node));
			if (line[idx].pool){
				line[idx].size = 1;
				//printf("Init line\n");
			}
		}
		
		/* if current page is full */
		if ((graph[idx].pos >= GRAPH_PAGE) && (graph[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (graph[idx].page < graph[idx].size - 1){
				graph[idx].page++;
				graph[idx].pos = 0;
				//printf("change graph page\n");
			}
			/* or then allocatte a new page */
			else if(graph[idx].page < GRAPH_POOL_PAGES-1){
				graph[idx].pool[graph[idx].page + 1] = malloc(GRAPH_PAGE * sizeof(graph_obj));
				if (graph[idx].pool[graph[idx].page + 1]){
					graph[idx].page++;
					graph[idx].size ++;
					graph[idx].pos = 0;
					//printf("Realloc graph\n");
				}
			}
		}
		
		/* if current page is full */
		if ((line[idx].pos >= LINE_PAGE) && (line[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (line[idx].page < line[idx].size - 1){
				line[idx].page++;
				line[idx].pos = 0;
				//printf("change line page\n");
			}
			/* or then allocatte a new page */
			else if(line[idx].page < GRAPH_POOL_PAGES-1){
				line[idx].pool[line[idx].page + 1] = malloc(LINE_PAGE * sizeof(line_node));
				if (line[idx].pool[line[idx].page + 1]){
					line[idx].page++;
					line[idx].size ++;
					line[idx].pos = 0;
					//printf("Realloc line\n");
				}
			}
		}
		
		ret_ptr = NULL;
		
		if ((graph[idx].pool[graph[idx].page] != NULL) &&  (line[idx].pool[line[idx].page] != NULL)){
			switch (action){
				case ADD_GRAPH:
					if (graph[idx].pos < GRAPH_PAGE){
						ret_ptr = &(((graph_obj *)graph[idx].pool[graph[idx].page])[graph[idx].pos]);
						graph[idx].pos++;
					}
					break;
				case ADD_LINE:
					if (line[idx].pos < LINE_PAGE){
						ret_ptr = &(((line_node *)line[idx].pool[line[idx].page])[line[idx].pos]);
						line[idx].pos++;
					}
					break;
				case ZERO_GRAPH:
					graph[idx].pos = 0;
					graph[idx].page = 0;
					break;
				case ZERO_LINE:
					line[idx].pos = 0;
					line[idx].page = 0;
					break;
				case FREE_ALL:
					for (i = 0; i < graph[idx].size; i++){
						free(graph[idx].pool[i]);
						graph[idx].pool[i] = NULL;
					}
					graph[idx].pos = 0;
					graph[idx].page = 0;
					graph[idx].size = 0;
					for (i = 0; i < line[idx].size; i++){
						free(line[idx].pool[i]);
						line[idx].pool[i] = NULL;
					}
					line[idx].pos = 0;
					line[idx].page = 0;
					line[idx].size = 0;
					break;
			}
		}
	}
	return ret_ptr;
}

graph_obj * graph_new(int pool_idx){
	/* create new graphics object */
	
	/* allocate the main struct */
	//graph_obj * new_obj = malloc(sizeof(graph_obj));
	graph_obj * new_obj = graph_mem_pool(ADD_GRAPH, pool_idx);
	
	if (new_obj){
		
		/* initialize */
		new_obj->owner = NULL;
		new_obj->pool_idx = pool_idx;
		/* initialize with a black color */
		new_obj->color.r = 0;
		new_obj->color.g = 0;
		new_obj->color.b =0;
		new_obj->color.a = 255;
		
		/*new_obj->rot = 0;
		new_obj->scale = 1;
		new_obj->ofs_x = 0;
		new_obj->ofs_y = 0;*/
		new_obj->tick = 0.0;
		//new_obj->thick_const = 0;
		//new_obj->fill = 0;
		new_obj->flags = 0;
		
		/* initialize with a solid line pattern */
		new_obj->patt_size = 1;
		new_obj->pattern[0] = 1.0;
		
		int i;
		for (i = 0; i < 20; i++){
			new_obj->cmplx_pat[i] = NULL;
		}
		
		/* extent init */
		//new_obj->ext_ini = 0;
		new_obj->ext_min_x = 0.0;
		new_obj->ext_min_y = 0.0;
		new_obj->ext_max_x = 0.0;
		new_obj->ext_max_y = 0.0;
		
		new_obj->img = NULL;
		
		new_obj->u[0] = 1.0;
		new_obj->u[1] = 0.0;
		new_obj->u[2] = 0.0;
		
		new_obj->v[0] = 0.0;
		new_obj->v[1] = 1.0;
		new_obj->v[2] = 0.0;
		
		/* allocate the line list */
		//new_obj->list = malloc(sizeof(line_node));
		new_obj->list = graph_mem_pool(ADD_LINE, pool_idx);
		
		if(new_obj->list){
			/* the list is empty */
			new_obj->list->next = NULL;
		}
		else{ /* if allocation fails */
			//free(new_obj); /* clear the main struct */
			new_obj = NULL;
		}
		
	}
	return new_obj;
}

void line_add(graph_obj * master, double x0, double y0, double z0, double x1, double y1, double z1){
	/* create and add a line object to the graph master´s list */
	
	if (master){
		//line_node *new_line = malloc(sizeof(line_node));
		line_node *new_line = graph_mem_pool(ADD_LINE, master->pool_idx);
		
		if (new_line){
			new_line->x0 = x0;
			new_line->y0 = y0;
			new_line->z0 = z0;
			new_line->x1 = x1;
			new_line->y1 = y1;
			new_line->z1 = z1;
			new_line->next = NULL;
			
			if(master->list->next == NULL){ /* check if list is empty */
				/* then, the new line is the first element */
				master->list->next = new_line;
			}
			else{ /* look for the end of list */
				line_node *tmp = master->list->next;
				while(tmp->next != NULL){
					tmp = tmp->next;
				}
				/* then, the new line is put in end of list */
				tmp->next = new_line;
			}
			/*update the extent of graph */
			/* sort the coordinates of entire line*/
			double min_x = (x0 <= x1) ? x0 : x1;
			double min_y = (y0 <= y1) ? y0 : y1;
			double max_x = (x0 > x1) ? x0 : x1;
			double max_y = (y0 > y1) ? y0 : y1;
			if (!(master->flags & EXT_INI)){
				master->flags |= EXT_INI;
				master->ext_min_x = min_x;
				master->ext_min_y = min_y;
				master->ext_max_x = max_x;
				master->ext_max_y = max_y;
			}
			else{
				master->ext_min_x = (master->ext_min_x <= min_x) ? master->ext_min_x : min_x;
				master->ext_min_y = (master->ext_min_y <= min_y) ? master->ext_min_y : min_y;
				master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
				master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
			}
		}
	}
}

void graph_merge(graph_obj * master, graph_obj *tail){
	
	if ((master) && (tail)){
		
		line_node *new_line = tail->list->next;
		if (new_line){
			
			if(master->list->next == NULL){ /* check if list is empty */
				/* then, the new line is the first element */
				master->list->next = new_line;
			}
			else{ /* look for the end of list */
				line_node *tmp = master->list->next;
				while(tmp->next != NULL){
					tmp = tmp->next;
				}
				/* then, the new line is put in end of list */
				tmp->next = new_line;
			}
			/*update the extent of graph */
			/* sort the coordinates of entire line*/
			if (!(master->flags & EXT_INI)){
				master->flags |= EXT_INI;
				master->ext_min_x = tail->ext_min_x;
				master->ext_min_y = tail->ext_min_y;
				master->ext_max_x = tail->ext_max_x;
				master->ext_max_y = tail->ext_max_y;
			}
			else{
				master->ext_min_x = (master->ext_min_x < tail->ext_min_x) ? master->ext_min_x : tail->ext_min_x;
				master->ext_min_y = (master->ext_min_y < tail->ext_min_y) ? master->ext_min_y : tail->ext_min_y;
				master->ext_max_x = (master->ext_max_x > tail->ext_max_x) ? master->ext_max_x : tail->ext_max_x;
				master->ext_max_y = (master->ext_max_y > tail->ext_max_y) ? master->ext_max_y : tail->ext_max_y;
			}
		}
	}
}

void graph_draw(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, ok = 1;
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int corners = 0, prev_x, prev_y; /* for fill */
			int corner_x[10000], corner_y[10000], stroke[10000];
			
			/* set the pattern */
			patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			img->frg = master->color;
			
			/* set the tickness */
			if (master->flags & THICK_CONST) img->tick = (int) round(master->tick);
			else img->tick = (int) round(master->tick * scale);
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = round((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = round((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = round((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = round((current->y1 - ofs_y) * scale)));
					
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				
				if (ok){
					x0 = (int) xd0;
					y0 = (int) yd0;
					x1 = (int) xd1;
					y1 = (int) yd1;
					
					bmp_line(img, xd0, yd0, xd1, yd1);
					//printf("%f %d %d %d %d\n", scale, x0, y0, x1, y1);
					
					if ((master->flags & FILLED) && (corners < 10000)){ /* check if object is filled */
						/*build the lists of corners */
						if (((x0 != prev_x)||(y0 != prev_y))||(corners == 0)){
							corner_x[corners] = x0;
							corner_y[corners] = y0;
							stroke[corners] = 0;
							corners++;
						}
						corner_x[corners] = x1;
						corner_y[corners] = y1;
						stroke[corners] = 1;
						corners++;
						
						prev_x = x1;
						prev_y = y1;
					}
				}
				current = current->next; /* go to next */
			}
			
			if ((master->flags & FILLED) && corners < 10000){ /* check if object is filled */
				/* draw a filled polygon */
				bmp_poly_fill(img, corners, corner_x, corner_y, stroke);
			}
		}
	}
}

int graph_draw2(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale){
	if ((master == NULL) || (img == NULL)) return 0;
	 /* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	double x0, y0, x1, y1;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter;
	
	/* set the pattern */
	patt_change(img, (double[]){1.0,}, 1);
	/* set the color */
	img->frg = master->color;
	
	/* set the tickness */
	if (master->flags & THICK_CONST)  img->tick = (int) round(master->tick);
	else img->tick = (int) round(master->tick * scale);
	
	if (master->patt_size > 1) { /* if graph is dashed lines */
		int patt_i = 0, patt_a_i = 0, patt_p_i = 0, draw;
		double patt_len = 0.0, patt_int, patt_part, patt_rem = 0.0, patt_acc, patt_rem_n;
		double patt_start_x = 0, patt_start_y = 0;
		double patt_start = 0;
		
		double p1x, p1y, p2x, p2y;
		double last;
		
		/* get the pattern length */
		for (i = 0; i < master->patt_size && i < 20; i++){
			patt_len += fabs(master->pattern[i]);
		}
		//patt_len *= scale;
		
		/*first vertice*/
		if (current){
			x0 = current->x0;
			y0 = current->y0;
			x1 = current->x1;
			y1 = current->y1;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
			
			/* find the start point of pattern, based in line - axis interceptions*/
			patt_start_x = x0;
			patt_start_y = y0;
			patt_start = 0;
			
			if (fabs(dy) > TOLERANCE){
				patt_start_x = (dy * x0 - dx * y0) / dy;
				patt_start_x /= 2;
			}else patt_start_x = 0;
			
			if (fabs(dx) > TOLERANCE){
				patt_start_y = (dx * y0 - dy * x0) / dx;
				patt_start_y /= 2;
			}else patt_start_y = 0;
			
			/*find distance between pattern start and segment's first point */
			if (fabs(cosine) > TOLERANCE){
				patt_start = (patt_start_x - x0)/ cosine;
			}
			else if (fabs(sine) > TOLERANCE){
				patt_start = (patt_start_y - y0)/ sine;
			}
			
			/* find the pattern initial conditions for the first point*/
			if (patt_start <= 0){ /* start of pattern outside segment */
				patt_start = fabs(fmod(patt_start, patt_len));
				patt_acc = fabs(master->pattern[0]);
				for (i = 1; i < master->patt_size && i < 20; i++){
					patt_i = i - 1;
					if (patt_start <= patt_acc){
						patt_rem = (patt_acc - patt_start);
						break;
					}
					patt_acc += fabs(master->pattern[i]);
				}
			}
			else { /* start of pattern on segment -> reverse the search */
				patt_start = fabs(fmod(patt_start, patt_len));
				patt_acc = fabs(master->pattern[master->patt_size - 1]);
				for (i = 1; i < master->patt_size && i < 20; i++){
					patt_i = master->patt_size - i;
					if (patt_start <= patt_acc){
						patt_rem = fabs(master->pattern[patt_i]) - (patt_acc - patt_start);
						break;
					}
					patt_acc += fabs(master->pattern[patt_i - 1]);
				}
			}
		}
		
		/* draw the lines */
		while(current){ /*sweep the list content */
			x0 = current->x0;
			y0 = current->y0;
			x1 = current->x1;
			y1 = current->y1;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
			
			/* initial point */
			draw = master->pattern[patt_i] >= 0.0;
			p1x = ((x0 - ofs_x) * scale);
			p1y = ((y0 - ofs_y) * scale);
			
			if (patt_rem <= modulus){ /* current segment needs some iterations over pattern */
			
				/* find how many interations over whole pattern */ 
				patt_part = modf((modulus - patt_rem)/patt_len, &patt_int);
				patt_part *= patt_len; /* remainder for the next step*/
				
				/* find how many interations over partial pattern */
				patt_a_i = 0;
				patt_p_i = patt_i;
				if (patt_rem > 0) patt_p_i++;
				if (patt_p_i >= master->patt_size) patt_p_i = 0;
				patt_acc = fabs(master->pattern[patt_p_i]);
				
				patt_rem_n = patt_part; /* remainder pattern for next segment continues */
				if (patt_part < patt_acc) patt_rem_n = patt_acc - patt_part;
				
				last = modulus - patt_int*patt_len - patt_rem; /* the last stroke (pattern fractional part) of current segment*/
				for (i = 0; i < master->patt_size && i < 20; i++){
					patt_a_i = i;
					if (patt_part < patt_acc) break;
					
					last -= fabs(master->pattern[patt_p_i]);
					
					patt_p_i++;
					if (patt_p_i >= master->patt_size) patt_p_i = 0;
					
					patt_acc += fabs(master->pattern[patt_p_i]);
					
					patt_rem_n = patt_acc - patt_part;
				}
				
				/* first stroke - remainder of past pattern*/
				p2x = patt_rem * scale * cosine + p1x;
				p2y = patt_rem * scale * sine + p1y;
				
				if (patt_rem > 0) {
					patt_i++;
					if (patt_i >= master->patt_size) patt_i = 0;
					
					if (draw){
						bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
					}
				}
				
				patt_rem = patt_rem_n; /* for next segment */
				p1x = p2x;
				p1y = p2y;
				
				/* draw pattern */
				iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
				for (i = 0; i < iter; i++){
					draw = master->pattern[patt_i] >= 0.0;
					p2x = fabs(master->pattern[patt_i]) * scale * cosine + p1x;
					p2y = fabs(master->pattern[patt_i]) * scale * sine + p1y;
					if (draw){
						bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
					}
					p1x = p2x;
					p1y = p2y;
					
					patt_i++;
					if (patt_i >= master->patt_size) patt_i = 0;
				}
				
				p2x = last * scale * cosine + p1x;
				p2y = last * scale * sine + p1y;
				draw = master->pattern[patt_i] >= 0.0;
				if (draw) bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
			}
			else{ /* current segment is in same past iteration pattern */
				p2x = modulus * scale * cosine + p1x;
				p2y = modulus * scale * sine + p1y;
				
				patt_rem -= modulus;
			
				if (draw){
					bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
				}
				p1x = p2x;
				p1y = p2y;
			}
		
			current = current->next; /* go to next */
		}
	}
	else{ /* for continuous lines*/
		while(current){ /*sweep the list content */
			/* apply the scale and offset */
			x0 = (current->x0 - ofs_x) * scale;
			y0 =(current->y0 - ofs_y) * scale;
			x1 = (current->x1 - ofs_x) * scale;
			y1 = (current->y1 - ofs_y) * scale;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
			
			if (master->pattern[0] >= 0.0)
				bmp_line_norm(img, x0, y0, x1, y1, -sine, cosine);
			
			current = current->next; /* go to next */
		}
	}
	if (master->flags & FILLED){
		graph_fill(master, img, ofs_x, ofs_y, scale);
	}
}

int graph_draw3(graph_obj * master, bmp_img * img, struct draw_param param){
	if ((master == NULL) || (img == NULL)) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	double x0, y0, x1, y1;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter;
	
	/* if has a bitmap image associated */
	if (master->img){
		/* apply  offset an scale */
		/* insertion point is first vertice */
		current = master->list->next;
		x0 = (current->x0 - param.ofs_x) * param.scale;
		y0 =(current->y0 - param.ofs_y) * param.scale;
		
		double u[3], v[3];
		u[0] = master->u[0] * param.scale;
		u[1] = master->u[1] * param.scale;
		u[2] = master->u[2] * param.scale;
		
		v[0] = master->v[0] * param.scale;
		v[1] = master->v[1] * param.scale;
		v[2] = master->v[2] * param.scale;
		
		/* draw bitmap image */
		bmp_put(master->img, img, x0, y0, u, v);
	}
	
	/* set the pattern */
	patt_change(img, (double[]){1.0,}, 1);
	/* set the color */
	img->frg = validate_color(master->color, param.list, param.subst, param.len_subst);
	
	/* set the tickness */
	if (master->flags & THICK_CONST) img->tick = (int) round(master->tick) + param.inc_thick;
	else img->tick = (int) round(master->tick * param.scale) + param.inc_thick;
	
	if (master->patt_size > 1) { /* if graph is dashed lines */
		int patt_i = 0, patt_a_i = 0, patt_p_i = 0, draw;
		double patt_len = 0.0, patt_int, patt_part, patt_rem = 0.0, patt_acc, patt_rem_n;
		
		double p1x, p1y, p2x, p2y;
		double last;
		
		/* get the pattern length */
		for (i = 0; i < master->patt_size && i < 20; i++){
			patt_len += fabs(master->pattern[i]);
		}
		//patt_len *= param.scale;
		
		/*first vertice*/
		if (current){
			x0 = current->x0;
			y0 = current->y0;
			x1 = current->x1;
			y1 = current->y1;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
		}
		
		/* draw the lines */
		while(current){ /*sweep the list content */
			
			x0 = current->x0;
			y0 = current->y0;
			x1 = current->x1;
			y1 = current->y1;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
			
			/* initial point */
			draw = master->pattern[patt_i] >= 0.0;
			p1x = ((x0 - param.ofs_x) * param.scale);
			p1y = ((y0 - param.ofs_y) * param.scale);
			
			if (patt_rem <= modulus){ /* current segment needs some iterations over pattern */
			
				/* find how many interations over whole pattern */ 
				patt_part = modf((modulus - patt_rem)/patt_len, &patt_int);
				patt_part *= patt_len; /* remainder for the next step*/
				
				/* find how many interations over partial pattern */
				patt_a_i = 0;
				patt_p_i = patt_i;
				if (patt_rem > 0) patt_p_i++;
				if (patt_p_i >= master->patt_size) patt_p_i = 0;
				patt_acc = fabs(master->pattern[patt_p_i]);
				
				patt_rem_n = patt_part; /* remainder pattern for next segment continues */
				if (patt_part < patt_acc) patt_rem_n = patt_acc - patt_part;
				
				last = modulus - patt_int*patt_len - patt_rem; /* the last stroke (pattern fractional part) of current segment*/
				for (i = 0; i < master->patt_size && i < 20; i++){
					patt_a_i = i;
					if (patt_part < patt_acc) break;
					
					last -= fabs(master->pattern[patt_p_i]);
					
					patt_p_i++;
					if (patt_p_i >= master->patt_size) patt_p_i = 0;
					
					patt_acc += fabs(master->pattern[patt_p_i]);
					
					patt_rem_n = patt_acc - patt_part;
				}
				
				/* first stroke - remainder of past pattern*/
				p2x = patt_rem * param.scale * cosine + p1x;
				p2y = patt_rem * param.scale * sine + p1y;
				
				if (patt_rem > 0) {
					/*------------- complex line type ----------------*/
					if (master->cmplx_pat[patt_i] != NULL &&  /* complex element */
						p2x > img->clip_x && p2x < (img->clip_x + img->clip_w) && /* inside bound parameters */
						p2y > img->clip_y && p2y < (img->clip_y + img->clip_h) )
					{
						list_node *cplx = master->cmplx_pat[patt_i]->next;
						graph_obj *cplx_gr = NULL;
						line_node *cplx_lin = NULL;
						
						/* sweep the main list */
						while (cplx != NULL){
							if (cplx->data){
								cplx_gr = (graph_obj *)cplx->data;
								cplx_lin = cplx_gr->list->next;
								/* draw the lines */
								while(cplx_lin){ /*sweep the list content */
									double xd0 = p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
									double yd0 = p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
									double xd1 = p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
									double yd1 = p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
									
									bmp_line(img, xd0, yd0, xd1, yd1);
									
									cplx_lin = cplx_lin->next; /* go to next */
								}
							}
							cplx = cplx->next;
						}
					}
					/*------------------------------------------------------*/
					
					patt_i++;
					if (patt_i >= master->patt_size) patt_i = 0;
					
					if (draw){
						bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
					}
				}
				
				patt_rem = patt_rem_n; /* for next segment */
				p1x = p2x;
				p1y = p2y;
				
				/* draw pattern */
				iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
				for (i = 0; i < iter; i++){					
					draw = master->pattern[patt_i] >= 0.0;
					p2x = fabs(master->pattern[patt_i]) * param.scale * cosine + p1x;
					p2y = fabs(master->pattern[patt_i]) * param.scale * sine + p1y;
					if (draw){
						bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
					}
					p1x = p2x;
					p1y = p2y;
					
					/*------------- complex line type ----------------*/
					if (master->cmplx_pat[patt_i] != NULL && /* complex element */
						p1x > img->clip_x && p1x < (img->clip_x + img->clip_w) && /* inside bound parameters */
						p1y > img->clip_y && p1y < (img->clip_y + img->clip_h) )
					{
						list_node *cplx = master->cmplx_pat[patt_i]->next;
						graph_obj *cplx_gr = NULL;
						line_node *cplx_lin = NULL;
						
						/* sweep the main list */
						while (cplx != NULL){
							if (cplx->data){
								cplx_gr = (graph_obj *)cplx->data;
								cplx_lin = cplx_gr->list->next;
								/* draw the lines */
								while(cplx_lin){ /*sweep the list content */
									
									double xd0 = p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
									double yd0 = p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
									double xd1 = p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
									double yd1 = p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
									
									bmp_line(img, xd0, yd0, xd1, yd1);
									
									cplx_lin = cplx_lin->next; /* go to next */
								}
							}
							cplx = cplx->next;
						}
					}
					/*------------------------------------------------------*/
					
					patt_i++;
					if (patt_i >= master->patt_size) patt_i = 0;
				}
				
				p2x = last * param.scale * cosine + p1x;
				p2y = last * param.scale * sine + p1y;
				draw = master->pattern[patt_i] >= 0.0;
				if (draw) bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
			}
			else{ /* current segment is in same past iteration pattern */
				p2x = modulus * param.scale * cosine + p1x;
				p2y = modulus * param.scale * sine + p1y;
				
				patt_rem -= modulus;
			
				if (draw){
					bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
				}
				p1x = p2x;
				p1y = p2y;
			}
		
			current = current->next; /* go to next */
		}
	}
	else{ /* for continuous lines*/
		while(current){ /*sweep the list content */
			/* apply the scale and offset */
			x0 = (current->x0 - param.ofs_x) * param.scale;
			y0 =(current->y0 - param.ofs_y) * param.scale;
			x1 = (current->x1 - param.ofs_x) * param.scale;
			y1 = (current->y1 - param.ofs_y) * param.scale;
			
			/* get polar parameters of line */
			dx = x1 - x0;
			dy = y1 - y0;
			modulus = sqrt(pow(dx, 2) + pow(dy, 2));
			cosine = 1.0;
			sine = 0.0;
			
			if (modulus > TOLERANCE){
				cosine = dx/modulus;
				sine = dy/modulus;
			}
			
			if (master->pattern[0] >= 0.0)
				bmp_line_norm(img, x0, y0, x1, y1, -sine, cosine);
			
			current = current->next; /* go to next */
		}
	}
	if (master->flags & FILLED){
		graph_fill(master, img, param.ofs_x, param.ofs_y, param.scale);
	}
}

void graph_line_t(bmp_img *img, double x0, double y0, double x1, double y1, double norm_x, double norm_y, double thick){
	if (thick > 2){
		int x_0 = (int) round(x0), y_0 = (int) round(y0);
		int x_1 = (int) round(x1), y_1 = (int) round(y1);
		if ((x_0 == x_1) && (y_0 == y_1)){ /* a dot*/
			bmp_circle_fill(img, x_0, y_0, thick/2);
		}
		else {
			int vert_x[4], vert_y[4];
			vert_x[0] = (int) round(x0 - norm_x);
			vert_x[1] = (int) round(x0 + norm_x);
			vert_x[3] = (int) round(x1 - norm_x);
			vert_x[2] = (int) round(x1 + norm_x);
			
			vert_y[0] = (int) round(y0 - norm_y);
			vert_y[1] = (int) round(y0 + norm_y);
			vert_y[3] = (int) round(y1 - norm_y);
			vert_y[2] = (int) round(y1 + norm_y);

			bmp_rect_fill(img, vert_x, vert_y);
			
		}
	}
	else{
		if (line_clip(img, &x0, &y0, &x1, &y1)) {
			int x_0 = (int) round(x0), y_0 = (int) round(y0);
			int x_1 = (int) round(x1), y_1 = (int) round(y1);
			if ((x_0 == x_1) && (y_0 == y_1)){ /* a dot*/
				
				bmp_point_raw (img, x_0, y_0);
			}
			else bmp_thin_line(img, x_0, y_0, x_1, y_1);
		}
	}
}

void graph_draw_fix(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, ok = 1;
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int corners = 0, prev_x, prev_y; /* for fill */
			int corner_x[1000], corner_y[1000], stroke[1000];
			
			/* set the pattern */
			patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			img->frg = color;
			/* set the tickness */
			if (master->flags & THICK_CONST) img->tick = (int) round(master->tick) + 3;
			else img->tick = (int) round(master->tick * scale) + 3;
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = round((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = round((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = round((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = round((current->y1 - ofs_y) * scale)));
				
				//x0 = (int) round((current->x0 - ofs_x) * scale);
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				if (ok){
					x0 = (int) xd0;
					y0 = (int) yd0;
					x1 = (int) xd1;
					y1 = (int) yd1;
					
					bmp_line(img, xd0, yd0, xd1, yd1);
					//bmp_line(img, x0, y0, x1, y1);
					//printf("%f %d %d %d %d\n", scale, x0, y0, x1, y1);
					
					if ((master->flags & FILLED) && (corners < 1000)){ /* check if object is filled */
						/*build the lists of corners */
						if (((x0 != prev_x)||(y0 != prev_y))||(corners == 0)){
							corner_x[corners] = x0;
							corner_y[corners] = y0;
							stroke[corners] = 0;
							corners++;
						}
						corner_x[corners] = x1;
						corner_y[corners] = y1;
						stroke[corners] = 1;
						corners++;
						
						prev_x = x1;
						prev_y = y1;
					}
				}
				current = current->next; /* go to next */
			}
			
			if ((master->flags & FILLED) && corners){ /* check if object is filled */
				/* draw a filled polygon */
				bmp_poly_fill(img, corners, corner_x, corner_y, stroke);
			}
		}
	}
}

void graph_arc(graph_obj * master, double c_x, double c_y, double c_z, double radius, double ang_start, double ang_end, int sig){
	if (master){
		int n = 32; //numero de vertices do polígono regular que aproxima o circulo ->bom numero 
		double ang;
		int steps, i;
		double x0, y0, x1, y1, step;
		
		ang_start *= M_PI/180;
		ang_end *= M_PI/180;
		
		ang = (ang_end - ang_start) * sig; //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o laço a seguir
		//steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		step = ang/(double) n;
		
		x0 = c_x + radius * cos(ang_start);
		y0 = c_y + radius * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//já começa do segundo vértice
		for (i = 1; i < n; i++){
			x1 = c_x + radius * cos(step * i * sig + ang_start);
			y1 = c_y + radius * sin(step * i * sig + ang_start);
			
			
			line_add(master, x0, y0, c_z, x1, y1, c_z);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no laço
		x1 = c_x + radius * cos(ang_end);
		y1 = c_y + radius * sin(ang_end);
		line_add(master, x0, y0, c_z, x1, y1, c_z);
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_arc_bulge(graph_obj * master, 
			double pt1_x, double pt1_y , double pt1_z,
			double pt2_x, double pt2_y, double pt2_z, 
			double bulge){
	
	if (fabs(bulge) > TOLERANCE){ /*bulge is non zero*/
		double theta, alfa, d, radius, ang_c, ang_start, ang_end, center_x, center_y;
		int sig;
		
		
		theta = 2 * atan(bulge);
		alfa = atan2(pt2_y-pt1_y, pt2_x-pt1_x);
		d = sqrt((pt2_y-pt1_y)*(pt2_y-pt1_y) + (pt2_x-pt1_x)*(pt2_x-pt1_x)) / 2;
		radius = d*(bulge*bulge + 1)/(2*bulge);
		
		ang_c = M_PI+(alfa - M_PI/2 - theta);
		center_x = radius*cos(ang_c) + pt1_x;
		center_y = radius*sin(ang_c) + pt1_y;
		
		//angulo inicial e final obtidos das coordenadas iniciais
		ang_start = atan2(pt1_y-center_y,pt1_x-center_x);
		ang_end = atan2(pt2_y-center_y,pt2_x-center_x);
		
		sig = 1;
		if (bulge < 0){
			ang_start += M_PI;
			ang_end += M_PI;
			sig = -1;
		}
		//converte para garus
		ang_start *= 180/M_PI;
		ang_end *= 180/M_PI;
		
		//arco(entidade, camada, center, radius, ang_start, ang_end, cor, esp, sig);
		graph_arc(master, center_x, center_y, pt1_z, radius, ang_start, ang_end, sig);
	}
	else{
		line_add(master, pt1_x, pt1_y, pt1_z, pt2_x, pt2_y, pt2_z);
	}
}

void graph_ellipse(graph_obj * master,
		double p1_x, double p1_y, double p1_z,
		double p2_x, double p2_y, double p2_z,
		double minor_ax, double ang_start, double ang_end){
	if (master){
		int n = 32; //numero de vertices do polígono regular que aproxima o circulo ->bom numero 
		double ang, major_ax, cosine, sine, step;
		int steps, i;
		double x0, y0, x1, y1;
		double xx0, yy0, xx1, yy1;
		
		//ang_start *= M_PI/180;
		//ang_end *= M_PI/180;
		
		major_ax = sqrt(pow(p2_x, 2) + pow(p2_y, 2)) ;
		minor_ax *= major_ax;
		
		/* rotation constants */
		cosine = cos(atan2(p2_y, p2_x));
		sine = sin(atan2(p2_y, p2_x));
		
		//printf("major = %0.2f, minor = %0.2f\n", major_ax, minor_ax);
		//printf("ang_start = %0.2f, ang_end = %0.2f\n", ang_start, ang_end);
		
		ang = (ang_end - ang_start); //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o laço a seguir
		//steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		step = ang/(double) n;
		
		x0 = p1_x + major_ax * cos(ang_start);
		y0 = p1_y + minor_ax * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//já começa do segundo vértice
		for (i = 1; i < n; i++){
			x1 = p1_x + major_ax * cos(step * i  + ang_start);
			y1 = p1_y + minor_ax * sin(step * i  + ang_start);
			
			xx0 = cosine*(x0-p1_x) - sine*(y0-p1_y) + p1_x;
			yy0 = sine*(x0-p1_x) + cosine*(y0-p1_y) + p1_y;
			xx1 = cosine*(x1-p1_x) - sine*(y1-p1_y) + p1_x;
			yy1 = sine*(x1-p1_x) + cosine*(y1-p1_y) + p1_y;
			line_add(master, xx0, yy0, p1_z, xx1, yy1, p1_z);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no laço
		x1 = p1_x + major_ax * cos(ang_end);
		y1 = p1_y + minor_ax * sin(ang_end);
		
		xx0 = cosine*(x0-p1_x) - sine*(y0-p1_y) + p1_x;
		yy0 = sine*(x0-p1_x) + cosine*(y0-p1_y) + p1_y;
		xx1 = cosine*(x1-p1_x) - sine*(y1-p1_y) + p1_x;
		yy1 = sine*(x1-p1_x) + cosine*(y1-p1_y) + p1_y;
		line_add(master, xx0, yy0, p1_z, xx1, yy1, p1_z);
		
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_ellipse2(graph_obj * master,
		double major_ax, double minor_ax, 
		double ang_start, double ang_end){
	if (master){
		int n = 32; //numero de vertices do polígono regular que aproxima o circulo ->bom numero 
		double ang, step;
		int steps, i;
		double x0, y0, x1, y1;
		
		//ang_start *= M_PI/180;
		//ang_end *= M_PI/180;
		
		//major_ax = sqrt(pow(p2_x, 2) + pow(p2_y, 2)) ;
		minor_ax *= major_ax;
		
		//printf("major = %0.2f, minor = %0.2f\n", major_ax, minor_ax);
		//printf("ang_start = %0.2f, ang_end = %0.2f\n", ang_start, ang_end);
		
		ang = (ang_end - ang_start); //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o laço a seguir
		//steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		step = ang/(double) n;
		
		x0 = major_ax * cos(ang_start);
		y0 = minor_ax * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//já começa do segundo vértice
		for (i = 1; i < n; i++){
			x1 = major_ax * cos(step * i + ang_start);
			y1 = minor_ax * sin(step * i + ang_start);
			
			line_add(master, x0, y0, 0.0, x1, y1, 0.0);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no laço
		x1 = major_ax * cos(ang_end);
		y1 = minor_ax * sin(ang_end);
		line_add(master, x0, y0, 0.0, x1, y1, 0.0);
		
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_modify(graph_obj * master, double ofs_x, double ofs_y, double scale_x, double scale_y, double rot){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x0, y0, x1, y1;
			double sine = 0, cosine = 1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			master->flags &= ~(EXT_INI);
			
			/* rotation constants */
			cosine = cos(rot*M_PI/180);
			sine = sin(rot*M_PI/180);
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				current->x0 = current->x0 * scale_x + ofs_x;
				current->y0 = current->y0 * scale_y + ofs_y;
				current->x1 = current->x1 * scale_x + ofs_x;
				current->y1 = current->y1 * scale_y + ofs_y;
				
				x0 = cosine*(current->x0-ofs_x) - sine*(current->y0-ofs_y) + ofs_x;
				y0 = sine*(current->x0-ofs_x) + cosine*(current->y0-ofs_y) + ofs_y;
				x1 = cosine*(current->x1-ofs_x) - sine*(current->y1-ofs_y) + ofs_x;
				y1 = sine*(current->x1-ofs_x) + cosine*(current->y1-ofs_y) + ofs_y;
				
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
				if (!(master->flags & EXT_INI)){
					master->flags |= EXT_INI;
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

void graph_rot(graph_obj * master, double base_x, double base_y, double rot){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x0, y0, x1, y1;
			double sine = 0, cosine = 1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			master->flags &= ~(EXT_INI);
			
			/* rotation constants */
			cosine = cos(rot*M_PI/180);
			sine = sin(rot*M_PI/180);
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				
				x0 = cosine*(current->x0-base_x) - sine*(current->y0-base_y) + base_x;
				y0 = sine*(current->x0-base_x) + cosine*(current->y0-base_y) + base_y;
				x1 = cosine*(current->x1-base_x) - sine*(current->y1-base_y) + base_x;
				y1 = sine*(current->x1-base_x) + cosine*(current->y1-base_y) + base_y;
				
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
				if (!(master->flags & EXT_INI)){
					master->flags |= EXT_INI;
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

/*
int main (void){
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	
	bmp_img * img = bmp_new(200, 200, white, black);
	graph_obj * test = graph_new();
	line_add(test, 0,0,100,100);
	line_add(test, 210,210,100,2);
	test->tick = 5;
	test->color = red;
	
	graph_draw(test, img);
	
	bmp_save("teste2.ppm", img);
	bmp_free(img);
	graph_free(test);
	return 0;
}
*/



void knot(int n, int c, int x[]){
	/*
	Subroutine to generate a B-spline open knot vector with multiplicity
	equal to the order at the ends.

	c            = order of the basis function
	n            = the number of defining polygon vertices
	nplus2       = index of x() for the first occurence of the maximum knot vector value
	nplusc       = maximum value of the knot vector -- $n + c$
	x()          = array containing the knot vector
	*/
	
	int nplusc,nplus2,i;

	nplusc = n + c;
	nplus2 = n + 2;
	
	if (nplusc < MAX_SPLINE_PTS){
		x[1] = 0;
		for (i = 2; i <= nplusc; i++){
			if ( (i > c) && (i < nplus2) )
				x[i] = x[i-1] + 1;
			else
				x[i] = x[i-1];
		}
	}
}	

void rbasis(int c, double t, int npts, int x[], double h[], double r[]){
	/*  Subroutine to generate rational B-spline basis functions--open knot vector

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: rbasis
	Language: C
	Subroutines called: none
	Book reference: Chapter 4, Sec. 4. , p 296

	c        = order of the B-spline basis function
	d        = first term of the basis function recursion relation
	e        = second term of the basis function recursion relation
	h[]	     = array containing the homogeneous weights
	npts     = number of defining polygon vertices
	nplusc   = constant -- npts + c -- maximum number of knot values
	r[]      = array containing the rationalbasis functions
	       r[1] contains the basis function associated with B1 etc.
	t        = parameter value
	temp[]   = temporary array
	x[]      = knot vector
	*/
	
	int nplusc;
	int i,j,k;
	double d,e;
	double sum;
	double temp[MAX_SPLINE_PTS];

	nplusc = npts + c;
	
	if (nplusc < MAX_SPLINE_PTS){

		/*		
		printf("knot vector is \n");
		for (i = 1; i <= nplusc; i++){
			printf(" %d %d \n", i,x[i]);
		}
		printf("t is %f \n", t);
		*/

		/* calculate the first order nonrational basis functions n[i]	*/

		for (i = 1; i<= nplusc-1; i++){
			if (( t >= x[i]) && (t < x[i+1]))
				temp[i] = 1;
			else
				temp[i] = 0;
		}

		/* calculate the higher order nonrational basis functions */

		for (k = 2; k <= c; k++){
			for (i = 1; i <= nplusc-k; i++){
				if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
					d = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
				else
					d = 0;

				if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
					e = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
				else
					e = 0;

				temp[i] = d + e;
			}
		}

		if (t == (double)x[nplusc]){		/*    pick up last point	*/
			temp[npts] = 1;
		}
		/*
		printf("Nonrational basis functions are \n");
		for (i=1; i<= npts; i++){
			printf("%f ", temp[i]);
		}
		printf("\n");
		*/
		/* calculate sum for denominator of rational basis functions */

		sum = 0.;
		for (i = 1; i <= npts; i++){
			sum = sum + temp[i]*h[i];
		}

		/* form rational basis functions and put in r vector */

		for (i = 1; i <= npts; i++){
			if (sum != 0){
				r[i] = (temp[i]*h[i])/(sum);}
			else
				r[i] = 0;
		}
	}
}

void rbspline(int npts, int k, int p1, double b[], double h[], double p[]){
	/*  Subroutine to generate a rational B-spline curve using an uniform open knot vector

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: rbspline.c
	Language: C
	Subroutines called: knot.c, rbasis.c, fmtmul.c
	Book reference: Chapter 4, Alg. p. 297

	b[]         = array containing the defining polygon vertices
		  b[1] contains the x-component of the vertex
		  b[2] contains the y-component of the vertex
		  b[3] contains the z-component of the vertex
	h[]			= array containing the homogeneous weighting factors 
	k           = order of the B-spline basis function
	nbasis      = array containing the basis functions for a single value of t
	nplusc      = number of knot values
	npts        = number of defining polygon vertices
	p[,]        = array containing the curve points
		  p[1] contains the x-component of the point
		  p[2] contains the y-component of the point
		  p[3] contains the z-component of the point
	p1          = number of points to be calculated on the curve
	t           = parameter value 0 <= t <= npts - k + 1
	x[]         = array containing the knot vector
	*/
	
	int i,j,icount,jcount;
	int i1;
	int x[MAX_SPLINE_PTS];		/* allows for 20 data points with basis function of order 5 */
	int nplusc;

	double step;
	double t;
	double nbasis[MAX_SPLINE_PTS];
	double temp;


	nplusc = npts + k;
	
	if (nplusc < MAX_SPLINE_PTS){

		/*  zero and redimension the knot vector and the basis array */

		for(i = 0; i <= npts; i++){
			nbasis[i] = 0.;
		}

		for(i = 0; i <= nplusc; i++){
			x[i] = 0.;
		}

		/* generate the uniform open knot vector */

		knot(npts,k,x);

		/*
		printf("The knot vector is ");
		for (i = 1; i <= nplusc; i++){
			printf(" %d ", x[i]);
		}
		printf("\n");
		*/

		
		icount = 0;

		/*    calculate the points on the rational B-spline curve */

		t = 0;
		step = ((double)x[nplusc])/((double)(p1-1));

		for (i1 = 1; i1<= p1; i1++){
			if ((double)x[nplusc] - t < 5e-6){
				t = (double)x[nplusc];
			}
			
			rbasis(k,t,npts,x,h,nbasis);      /* generate the basis function for this value of t */
			//printf("cpts=%d, order=%d, t=%0.2f \n",npts,k,t);
			/*
			printf("t = %f \n",t);
			printf("nbasis = ");
			for (i = 1; i <= npts; i++){
				printf("%f  ",nbasis[i]);
			}
			printf("\n");
			*/
			for (j = 1; j <= 3; j++){      /* generate a point on the curve */
				jcount = j;
				p[icount+j] = 0.;

				for (i = 1; i <= npts; i++){ /* Do local matrix multiplication */
					temp = nbasis[i]*b[jcount];
					p[icount + j] = p[icount + j] + temp;
					/*
					printf("jcount,nbasis,b,nbasis*b,p = %d %f %f %f %f\n",jcount,nbasis[i],b[jcount],temp,p[icount+j]);
					*/
					jcount = jcount + 3;
				}
			}
			/*
			printf("icount, p %d %f %f %f \n",icount,p[icount+1],p[icount+2],p[icount+3]);
			*/
			icount = icount + 3;
			t = t + step;
		}
	}
}

void graph_mod_axis(graph_obj * master, double normal[3] , double elev){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x_axis[3], y_axis[3], point[3], x_col[3], y_col[3];
			double wy_axis[3] = {0.0, 1.0, 0.0};
			double wz_axis[3] = {0.0, 0.0, 1.0};
			
			
			double x0, y0, x1, y1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			
			master->flags &= ~(EXT_INI);
			
			
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
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				point[0] = current->x0;
				point[1] = current->y0;
				point[2] = current->z0;
				if (fabs(point[2]) < TOLERANCE) point[2] = elev;
				x0 = dot_product(point, x_col);
				y0 = dot_product(point, y_col);
				
				point[0] = current->x1;
				point[1] = current->y1;
				point[2] = current->z1;
				if (fabs(point[2]) < TOLERANCE) point[2] = elev;
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
				if (!(master->flags & EXT_INI)){
					master->flags |= EXT_INI;
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

int l_r_isect(double lin_pt1[2], double lin_pt2[2], double rect_pt1[2], double rect_pt2[2]){
	/* check if a line instersect a rectangle */
	
	double r_bl_x, r_bl_y, r_tr_x,r_tr_y;
	double a, b, c, pol1, pol2, pol3, pol4;
	
	/* sort the rectangle corners */
	r_bl_x = (rect_pt1[0] < rect_pt2[0]) ? rect_pt1[0] : rect_pt2[0];
	r_bl_y = (rect_pt1[1] < rect_pt2[1]) ? rect_pt1[1] : rect_pt2[1];
	r_tr_x = (rect_pt1[0] > rect_pt2[0]) ? rect_pt1[0] : rect_pt2[0];
	r_tr_y = (rect_pt1[1] > rect_pt2[1]) ? rect_pt1[1] : rect_pt2[1];
	
	/* check if line is out of bounds of rectangle */
	if (((lin_pt1[0] > r_tr_x) && (lin_pt2[0] > r_tr_x)) || ( /* line in right side */
	(lin_pt1[1] > r_tr_y) && (lin_pt2[1] > r_tr_y)) || (  /* line in up side */
	(lin_pt1[0] < r_bl_x) && (lin_pt2[0] < r_bl_x)) || ( /* line in left side */
	(lin_pt1[1] < r_bl_y) && (lin_pt2[1] < r_bl_y))){ /* line in down side */
		return 0;}
	else{ /* compute the triangle polarizations in each corner of rectangle,
		relative to line*/
		a = lin_pt2[1]-lin_pt1[1];
		b = lin_pt1[0]-lin_pt2[0];
		c = lin_pt2[0]*lin_pt1[1] - lin_pt1[0]*lin_pt2[1];
		
		pol1 = a*rect_pt1[0] + b*rect_pt1[1] + c;
		pol2 = a*rect_pt1[0] + b*rect_pt2[1] + c;
		pol3 = a*rect_pt2[0] + b*rect_pt1[1] + c;
		pol4 = a*rect_pt2[0] + b*rect_pt2[1] + c;
		if (((pol1>=0) && (pol2>=0) && (pol3>=0) && (pol4>=0)) ||(
		(pol1<0) && (pol2<0) && (pol3<0) && (pol4<0))){
			return 0;}
		return 1;
	}
}

int graph_isect(graph_obj * master, double rect_pt1[2], double rect_pt2[2]){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double lin_pt1[2], lin_pt2[2];
			line_node *current = master->list->next;
			
			while(current){ /*sweep the list content */
				/* update the graph */
				lin_pt1[0] = current->x0;
				lin_pt1[1] = current->y0;
				lin_pt2[0] = current->x1;
				lin_pt2[1] = current->y1;
				if (((lin_pt1[0] > rect_pt1[0] && lin_pt1[0] < rect_pt2[0]) &&
					(lin_pt1[1] > rect_pt1[1] && lin_pt1[1] < rect_pt2[1])) ||
					((lin_pt2[0] > rect_pt1[0] && lin_pt2[0] < rect_pt2[0]) &&
					(lin_pt2[1] > rect_pt1[1] && lin_pt2[1] < rect_pt2[1])))
					return 1;
				if(l_r_isect(lin_pt1, lin_pt2, rect_pt1, rect_pt2)){
					return 1;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
	return 0;
}

int graph_in_rect(graph_obj * master, double rect_pt1[2], double rect_pt2[2]){
	if (!master) return 0;
	if(!master->list->next) return 0; /* check if list is not empty */
	
	double lin_pt1[2], lin_pt2[2];
	line_node *current = master->list->next;
	
	while(current){ /*sweep the list content */
		/* update the graph */
		lin_pt1[0] = current->x0;
		lin_pt1[1] = current->y0;
		lin_pt2[0] = current->x1;
		lin_pt2[1] = current->y1;
		if (!(((lin_pt1[0] > rect_pt1[0] && lin_pt1[0] < rect_pt2[0]) &&
			(lin_pt1[1] > rect_pt1[1] && lin_pt1[1] < rect_pt2[1])) &&
			((lin_pt2[0] > rect_pt1[0] && lin_pt2[0] < rect_pt2[0]) &&
			(lin_pt2[1] > rect_pt1[1] && lin_pt2[1] < rect_pt2[1]))))
			return -1;
		
		current = current->next; /* go to next */
	}
	return 1;
}

#if(0)
int graph_list_draw(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_draw2(curr_graph, img, ofs_x, ofs_y, scale);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}
#endif

int graph_list_draw(list_node *list, bmp_img * img, struct draw_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				//print_graph_pdf(curr_graph, buf, param);
				graph_draw3(curr_graph, img, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}


#if(0)
int graph_list_draw_fix(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_draw_fix(curr_graph, img, ofs_x, ofs_y, scale, color);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}
#endif

int graph_list_ext(list_node *list, int *init, double * min_x, double * min_y, double * max_x, double * max_y){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if (curr_graph->list->next != NULL){ /*check if graph is not empty */
					if (*init == 0){
						*init = 1;
						*min_x = curr_graph->ext_min_x;
						*min_y = curr_graph->ext_min_y;
						*max_x = curr_graph->ext_max_x;
						*max_y = curr_graph->ext_max_y;
					}
					else{
						*min_x = (*min_x < curr_graph->ext_min_x) ? *min_x : curr_graph->ext_min_x;
						*min_y = (*min_y < curr_graph->ext_min_y) ? *min_y : curr_graph->ext_min_y;
						*max_x = (*max_x > curr_graph->ext_max_x) ? *max_x : curr_graph->ext_max_x;
						*max_y = (*max_y > curr_graph->ext_max_y) ? *max_y : curr_graph->ext_max_y;
					}
				}
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_modify(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_modify(curr_graph, ofs_x, ofs_y, scale_x, scale_y, rot);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_modify_idx(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_modify(curr_graph, ofs_x, ofs_y, scale_x, scale_y, rot);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_rot(list_node *list, double base_x, double base_y , double rot){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_rot(curr_graph, base_x, base_y, rot);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_rot_idx(list_node *list, double base_x, double base_y , double rot, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_rot(curr_graph, base_x, base_y, rot);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_mod_ax(list_node *list, double normal[3], double elev, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_mod_axis(curr_graph, normal, elev);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

graph_obj * graph_list_isect(list_node *list, double rect_pt1[2], double rect_pt2[2]){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(graph_isect(curr_graph, rect_pt1, rect_pt2)){
					return curr_graph;
				}
			}
			current = current->next;
		}
	}
	return NULL;
}

int graph_list_in_rect(list_node *list, double rect_pt1[2], double rect_pt2[2]){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int num = 0;
		
	if (!list) return 0;
	current = list->next;
	
	/* sweep the main list */
	while (current != NULL){
		if (current->data){
			curr_graph = (graph_obj *)current->data;
			int ret = graph_in_rect(curr_graph, rect_pt1, rect_pt2);
			if(ret < 0)
				return 0;
			num += ret;
		}
		current = current->next;
	}
	
	if (num) return 1;
	return 0;
}

int graph_list_color(list_node *list, bmp_color color){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				curr_graph->color = color;
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

double pt_dist(double x0, double y0, double x1, double y1){
	return sqrt(pow(x1-x0, 2) + pow(y1-y0, 2));
}

int pt_lies_seg (double ln_x0, double ln_y0, double ln_x1 , double ln_y1, double pt_x, double pt_y){
	double ln = pt_dist(ln_x0, ln_y0, ln_x1, ln_y1);
	double seg1 = pt_dist(ln_x0, ln_y0, pt_x, pt_y);
	double seg2 = pt_dist(pt_x, pt_y, ln_x1, ln_y1);
	return fabs(ln - seg1 - seg2) < TOLERANCE;
}

int graph_is_closed(graph_obj * master){
	if (master == NULL) return 0;
	if (master->list->next == NULL) return 0; /* check if list is not empty */
	
	double curr_x, curr_y, prev_x, prev_y, first_x, first_y;
	int first = 0;
	line_node *current = master->list->next;
	
	while(current){ /*sweep the list content */
		curr_x = current->x0;
		curr_y = current->y0;
		
		if (!first){
			first = 1;
			first_x = current->x0;
			first_y = current->y0;
		}
		else if ((fabs(curr_x - prev_x) > TOLERANCE) || 
			(fabs(curr_y - prev_y) > TOLERANCE)) return 0;
		
		prev_x = current->x1;
		prev_y = current->y1;
		
		current = current->next; /* go to next */
	}
	
	if ((fabs(first_x - prev_x) > TOLERANCE) || 
		(fabs(first_y - prev_y) > TOLERANCE)) return 0;
	
	return 1;
}

int graph_dash(graph_obj * master, double x0, double y0, 
double x1, double y1,
double skew, double dash[], int num_dash){
	if (master == NULL) return 0; /* check if list is not empty */
	
	double dx, dy, modulus, sine, cosine;
	int i, iter, reverse = 0, idx = 0;
	
	if (num_dash > 1) { /* if graph is dashed lines */
		int patt_i = 0, patt_a_i = 0, patt_p_i = 0, draw;
		double patt_len = 0.0, patt_int, patt_part, patt_rem = 0.0, patt_acc, patt_rem_n;
		double patt_start_x = 0, patt_start_y = 0;
		double patt_start = 0;
		
		double p1x, p1y, p2x, p2y;
		double last;
		
		/* get the pattern length */
		for (i = 0; i < num_dash && i < 20; i++){
			patt_len += fabs(dash[i]);
		}
		
		/* get polar parameters of line */
		dx = x1 - x0 + TOLERANCE;
		dy = y1 - y0 + TOLERANCE;
		modulus = sqrt(pow(dx, 2) + pow(dy, 2));
		cosine = 1.0;
		sine = 0.0;
		
		if (modulus > TOLERANCE){
			cosine = dx/modulus;
			sine = dy/modulus;
		}
		
		patt_start = cosine*x0 + sine*y0 - skew;
		reverse = patt_start < 0;
		
		/* find the pattern initial conditions for the first point*/
		patt_start = fabs(fmod(patt_start, patt_len));
		iter = get_i(0, num_dash, reverse);
		patt_acc = fabs(dash[iter]) + TOLERANCE;
		patt_rem_n = patt_start;
		for (i = 1; i < num_dash && i < 20; i++){
			
			if (patt_start < patt_acc){
				
				break;
			}
			patt_rem_n -= fabs(dash[get_i(i - 1, num_dash, reverse)]);
			patt_acc += fabs(dash[get_i(i, num_dash, reverse)]);
		}
		patt_i = i - 1;
		
		if (reverse) patt_rem = patt_rem_n;
		else patt_rem = (patt_acc - patt_start);
		
		/* initial point */
		draw = dash[get_i(patt_i, num_dash, reverse)] >= 0.0;
		p1x = x0;
		p1y = y0;
		patt_i = get_i(patt_i, num_dash, reverse);
		reverse = 0;
		
		if (patt_rem < modulus){ /* current segment needs some iterations over pattern */
			
			/* find how many interations over whole pattern */ 
			patt_part = modf((modulus - patt_rem)/patt_len, &patt_int);
			patt_part *= patt_len; /* remainder for the next step*/
			
			/* find how many interations over partial pattern */
			patt_a_i = 0;
			patt_p_i = patt_i;
			if (patt_rem > 0) patt_p_i++;
			if (patt_p_i >= num_dash) patt_p_i = 0;
			
			/* calcule the last stroke (pattern fractional part) of current segment*/
			patt_acc = fabs(dash[get_i(patt_p_i, num_dash, reverse)]) + TOLERANCE;
			last = modulus - patt_int*patt_len - patt_rem;
			for (i = 0; i < num_dash && i < 20; i++){
				patt_a_i = i;
				if (patt_part < patt_acc) break;
				
				last -= fabs(dash[get_i(patt_p_i, num_dash, reverse)]);
				
				patt_p_i++;
				if (patt_p_i >= num_dash) patt_p_i = 0;
				
				patt_acc += fabs(dash[get_i(patt_p_i, num_dash, reverse)]);
			}
			
			/* do the first stroke - partial dash*/
			p2x = patt_rem * cosine + p1x;
			p2y = patt_rem * sine + p1y;
			
			if (patt_rem > 0) {
				patt_i++;
				if (patt_i >= num_dash) patt_i = 0;
				
				if (draw){
					line_add(master, p1x, p1y, 0.0, p2x, p2y, 0.0);
				}
			}
			
			/* do the dashes */
			p1x = p2x;
			p1y = p2y;
			
			/* draw pattern */
			iter = (int) (patt_int * (num_dash)) + patt_a_i;
			for (i = 0; i < iter; i++){
				idx = get_i(patt_i, num_dash, reverse);
				draw = dash[idx] >= 0.0;
				p2x = fabs(dash[idx]) * cosine + p1x;
				p2y = fabs(dash[idx]) * sine + p1y;
				if (draw){
					line_add(master, p1x, p1y, 0.0, p2x, p2y, 0.0);
				}
				p1x = p2x;
				p1y = p2y;
				patt_i++;
				if (patt_i >= num_dash) patt_i = 0;
			}
			
			/* do the last stroke - partial dash*/
			idx = get_i(patt_i, num_dash, reverse);
			p2x = last * cosine + p1x;
			p2y = last * sine + p1y;
			draw = dash[idx] >= 0.0;
			if (draw) line_add(master, p1x, p1y, 0.0, p2x, p2y, 0.0);
		}
		else{ /* current segment is in same past iteration pattern */
			p2x = modulus * cosine + p1x;
			p2y = modulus * sine + p1y;
			
			patt_rem -= modulus;
		
			if (draw){
				line_add(master, p1x, p1y, 0.0, p2x, p2y, 0.0);
			}
			p1x = p2x;
			p1y = p2y;
		}
	}
	else{ /* for continuous lines*/
		
		/* get polar parameters of line */
		dx = x1 - x0;
		dy = y1 - y0;
		modulus = sqrt(pow(dx, 2) + pow(dy, 2));
		cosine = 1.0;
		sine = 0.0;
		
		if (modulus > TOLERANCE){
			cosine = dx/modulus;
			sine = dy/modulus;
		}
		
		if (dash[0] >= 0.0){
			line_add(master, x0, y0, 0.0, x1, y1, 0.0);
		}
	}

}


graph_obj * graph_hatch(graph_obj * ref, 
double angle,
double orig_x, double orig_y,
double delta_x, double delta_y,
double dash[], int num_dash,
int pool_idx){
	//if (!graph_is_closed(ref)) return NULL; /* verify if reference is a closed path */
	
	graph_obj *ret_graph = NULL;//graph_new(pool_idx);
	
	int i, j, idx0, idx1;
	double min_x, max_x, min_y, max_y;
	
	int nodes = 0, steps = 0;
	
	double  start, end, swap;
	double node_x[1000], node_y[1000];
	double pos[1000];
	struct sort_by_idx sort_vert[1000];
	
	double a = sin(angle);
	double b = -cos(angle);
	double c = -(a*orig_x+b*orig_y);
	
	double delta = -(a * delta_x + b * delta_y); /* distance between hatch lines */
	double skew = -b * delta_x + a * delta_y; /* dash skew in hatch line direction*/
	double curr_skew = (-b * orig_x + a * orig_y);
	
	/* find min and max by reference graph rectangle*/
	min_x = ref->ext_min_x;
	max_x = ref->ext_max_x;
	min_y = ref->ext_min_y;
	max_y = ref->ext_max_y;
	
	/* calcule distances between cornes of graph rectangle and line at orign */
	double dist[4], tmp;
	dist[0] = (a*min_x + b*min_y + c);
	dist[1] = (a*min_x + b*max_y + c);
	dist[2] = (a*max_x + b*min_y + c);
	dist[3] = (a*max_x + b*max_y + c);
	
	/* Sort the distances*/
	qsort(dist, 4, sizeof(double), cmp_double);
	
	start = floor(dist[0]/delta) * (delta);
	end = ceil(dist[3]/delta) * (delta);
	if (delta < 0){ /* invert corners */
		start = floor(dist[3]/delta) * (delta);
		end = ceil(dist[0]/delta) * (delta);
	}
	
	/* how many lines are sampled */
	steps = (int) fabs(round((end - start)/delta));
	
	if (steps > 0) ret_graph = graph_new(pool_idx);
	
	c -= start; /* perpenticular displacement of line */
	
	curr_skew -= start/delta * skew; /* linear displacement of dahes */
	
	for (i = 0; i < steps; i++){
		if((ref->list->next) && (ret_graph != NULL)) { /* check if list is not empty */
			line_node *current = ref->list->next;
			
			while(current){ /*sweep the bondary segments list */
				
				/* get line parameters of current boundary segment*/
				double a2 = current->y1 - current->y0;
				double b2 = -(current->x1 - current->x0);
				double c2 = current->x1 * current->y0 - current->y1 * current->x0;
				
				/* calcule intersections between hatch line and boundary segment*/
				double den =b*a2-b2*a;
				if ( (fabs(den) > TOLERANCE) && (nodes < 1000)){
					double x = (b*c2-b2*c)/-den;
					double y = (a*c2-a2*c)/den;
					
					if (pt_lies_seg(current->x0, current->y0, current->x1, current->y1, x, y)){
						/* if intersection is in segment, add point to hatch vertices list*/
						node_x[nodes] = x;
						node_y[nodes] = y;
						
						/* parameters for sorting vertices */
						if (fabs(b) > TOLERANCE)
							pos[nodes] = (x + c * a) / b;
						else if (fabs(a) > TOLERANCE)
							pos[nodes] = -(y + c * b) / a;
						//else pos[nodes] = 0.0;
						sort_vert[nodes].idx = nodes;
						sort_vert[nodes].data = &pos[nodes];
						
						
						nodes++;
					}
				}
				current = current->next; /* go to next */
			}
		}
		
		if (nodes > 1){
			/* Sort the hatch vertices */
			qsort(sort_vert, nodes, sizeof(struct sort_by_idx), cmp_vert);
			
			/* create the hatch segments between vertices pairs */
			j = 0;
			while (j < nodes - 1){
				idx0 = sort_vert[j].idx;
				idx1 = sort_vert[j+1].idx;
				if  ((fabs(node_x[idx0] - node_x[idx1]) < TOLERANCE) &&
					(fabs(node_y[idx0] - node_y[idx1]) < TOLERANCE)){
					/* ignore duplicated nodes */
					j++;
				}
				else {
					graph_dash(ret_graph, node_x[idx0], node_y[idx0],
					node_x[idx1], node_y[idx1],
					curr_skew, dash, num_dash);
					
					j += 2;
				}
			}
		}
		/* update the next line parameters */
		curr_skew -= skew;
		c -= delta;
		nodes = 0;
	}
	
	return ret_graph;
	
}

#define plot_(X,Y,D) do{ unsigned char alfa = img->frg.a;	\
  img->frg.a = (D) * alfa;	\
  bmp_point_raw(img, (X), (Y));	\
  img->frg.a = alfa; }while(0)

/* Xiaolin Wu antialising line algorithm
font: http://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C */
#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
 
#define swap_(a, b) do{ __typeof__(a) tmp;  tmp = a; a = b; b = tmp; }while(0)
void draw_line_antialias( bmp_img * img, double x1, double y1, double x2, double y2){
	double dx = x2 - x1;
	double dy = y2 - y1;
	
	if ( fabs(dx) > fabs(dy) ) {
		if ( x2 < x1 ) {
			swap_(x1, x2);
			swap_(y1, y2);
		}
		double gradient = dy / dx;
		double xend = round_(x1);
		double yend = y1 + gradient*(xend - x1);
		double xgap = rfpart_(x1 + 0.5);
		int xpxl1 = xend;
		int ypxl1 = ipart_(yend);
		plot_(xpxl1, ypxl1, rfpart_(yend)*xgap);
		plot_(xpxl1, ypxl1+1, fpart_(yend)*xgap);
		double intery = yend + gradient;

		xend = round_(x2);
		yend = y2 + gradient*(xend - x2);
		xgap = fpart_(x2+0.5);
		int xpxl2 = xend;
		int ypxl2 = ipart_(yend);
		plot_(xpxl2, ypxl2, rfpart_(yend) * xgap);
		plot_(xpxl2, ypxl2 + 1, fpart_(yend) * xgap);

		int x;
		for(x=xpxl1+1; x < xpxl2; x++) {
			plot_(x, ipart_(intery), rfpart_(intery));
			plot_(x, ipart_(intery) + 1, fpart_(intery));
			intery += gradient;
		}
	} else {
		if ( y2 < y1 ) {
			swap_(x1, x2);
			swap_(y1, y2);
		}
		double gradient = dx / dy;
		double yend = round_(y1);
		double xend = x1 + gradient*(yend - y1);
		double ygap = rfpart_(y1 + 0.5);
		int ypxl1 = yend;
		int xpxl1 = ipart_(xend);
		plot_(xpxl1, ypxl1, rfpart_(xend)*ygap);
		plot_(xpxl1 + 1, ypxl1, fpart_(xend)*ygap);
		double interx = xend + gradient;

		yend = round_(y2);
		xend = x2 + gradient*(yend - y2);
		ygap = fpart_(y2+0.5);
		int ypxl2 = yend;
		int xpxl2 = ipart_(xend);
		plot_(xpxl2, ypxl2, rfpart_(xend) * ygap);
		plot_(xpxl2 + 1, ypxl2, fpart_(xend) * ygap);

		int y;
		for(y=ypxl1+1; y < ypxl2; y++) {
			plot_(ipart_(interx), y, rfpart_(interx));
			plot_(ipart_(interx) + 1, y, fpart_(interx));
			interx += gradient;
		}
	}
}
#undef swap_
//#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_

/*int bmp_units (double value, double ofs, double scale){
	return (int)round((value - ofs) * scale);
}*/

#define BMP_U(value, ofs, scale) (int)round((value - ofs) * scale)

int graph_fill(graph_obj * ref, bmp_img * img,
double ofs_x, double ofs_y, double scale){
	
	//if (!graph_is_closed(ref)) return 0; /* verify if reference is a closed path */
	
	int i, j;
	int min_x, max_x, min_y, max_y;
	
	int nodes = 0, steps = 0;
	
	//double  start, end, swap;
	double x0, y0, x1, y1, swap;
	double node_x[1000];//, node_y[1000];
	int pix_x, pix_y;
	
	
	/* first, draw contour with antialised lines */
	line_node *current = ref->list->next;
	
	while(current){ /*sweep the list content */
		x0 = (current->x0 - ofs_x) * scale;
		y0 = (current->y0 - ofs_y) * scale;
		x1 = (current->x1 - ofs_x) * scale;
		y1 = (current->y1 - ofs_y) * scale;
		
		draw_line_antialias(img, x0, y0, x1, y1);
		
		current = current->next; /* go to next */
	}
	
	
	/* find min and max by reference graph rectangle*/
	/*
	min_x = (int)ceil((ref->ext_min_x - ofs_x) * scale);
	max_x = (int)floor((ref->ext_max_x - ofs_x) * scale);
	min_y = (int)ceil((ref->ext_min_y - ofs_y) * scale);
	max_y = (int)floor((ref->ext_max_y - ofs_y) * scale);*/
	min_x = BMP_U(ref->ext_min_x, ofs_x, scale);
	max_x = BMP_U(ref->ext_max_x, ofs_x, scale);
	min_y = BMP_U(ref->ext_min_y, ofs_y, scale);
	max_y = BMP_U(ref->ext_max_y, ofs_y, scale);
	
	int w = img->width, h = img->height;
	if((min_x < w) && (max_x > 0) && (min_y < h) && (max_y > 0 )){
		
		min_x = (min_x > 0) ? min_x : 0;
		max_x = (max_x < w) ? max_x : w;
		min_y = (min_y > 0) ? min_y : 0;
		max_y = (max_y < h) ? max_y : h;
		
		
		
		for (pix_y = min_y; pix_y < max_y; pix_y++){ /* sweep line in y coordinate*/
			if(ref->list->next) { /* check if list is not empty */
				current = ref->list->next;
				
				while(current){ /*sweep the list content */
					x0 = BMP_U(current->x0, ofs_x, scale);
					y0 = BMP_U(current->y0, ofs_y, scale);
					x1 = BMP_U(current->x1, ofs_x, scale);
					y1 = BMP_U(current->y1, ofs_y, scale);
					
					if(((y0 < pix_y) && (y1 >= pix_y)) || 
						((y1 < pix_y) && (y0 >= pix_y))){
						/* find x coord of intersection and add to list */
						node_x[nodes] = (x1 + (double) (pix_y - y1)/(y0 - y1)*(x0 - x1));
						node_x[nodes] = (node_x[nodes] >= 0) ? node_x[nodes] : -1;
						node_x[nodes] = (node_x[nodes] <= max_x) ? node_x[nodes] : max_x + 1;
						nodes++;
					}
					
					current = current->next; /* go to next */
				}
			}
			
			if (nodes > 1){
				/* Sort the nodes */
				qsort(node_x, nodes, sizeof(double), cmp_double);
				
				/*fill the pixels between node pairs*/
				for (i = 0; i < nodes ; i += 2){
					if (i+1 < nodes){
						for(pix_x = node_x[i]+1; pix_x <= node_x[i + 1]; pix_x++){
							bmp_point_raw(img, pix_x, pix_y);
						}
					}
				}
				
				
			}
			nodes = 0;
		}
	}
	
	return 1;
	
}

void graph_draw_aa(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int ok = 1;
			
			/* set the pattern */
			patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			img->frg = master->color;
			
			/* set the tickness */
			//if (master->thick_const)
			if (master->flags & THICK_CONST)
				img->tick = (int) round(master->tick);
			else img->tick = (int) round(master->tick * scale);
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = ((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = ((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = ((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = ((current->y1 - ofs_y) * scale)));
					
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				
				if (ok){
					
					
					//bmp_line(img, xd0, yd0, xd1, yd1);
					draw_line_antialias(img, xd0, yd0, xd1, yd1);
					
					
				}
				current = current->next; /* go to next */
			}
			
			
		}
	}
}

int graph_list_draw_aa(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_draw_aa(curr_graph, img, ofs_x, ofs_y, scale);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

/*
https://stackoverflow.com/questions/11907947/how-to-check-if-a-point-lies-on-a-line-between-2-other-points
by https://stackoverflow.com/users/688624/imallett

This is independent of Javascript. Try the following algorithm, with points p1=point1 and p2=point2, and your third point being p3=currPoint:

v1 = p2 - p1
v2 = p3 - p1
v3 = p3 - p2
if (dot(v2,v1)>0 and dot(v3,v1)<0) return between
else return not between
If you want to be sure it's on the line segment between p1 and p2 as well:

v1 = normalize(p2 - p1)
v2 = normalize(p3 - p1)
v3 = p3 - p2
if (fabs(dot(v2,v1)-1.0)<EPS and dot(v3,v1)<0) return between
else return not between


Define function PlotAntiAliasedPoint ( number x, number y )
    For roundedx = floor ( x ) to ceil ( x ) do
         For roundedy = floor ( y ) to ceil ( y ) do
              percent_x = 1 - abs ( x - roundedx )
              percent_y = 1 - abs ( y - roundedy )
              percent = percent_x * percent_y
              DrawPixel ( coordinates roundedx, roundedy, color percent (range 0-1) )
*/

/* https://gamedev.stackexchange.com/questions/45298/convert-orientation-vec3-to-a-rotation-matrix
answered Dec 10 '12 at 12:08 by sam hocevar

Your problem is under-constrained, so there are a lot of possible solutions.

My suggestion is to see your vec3 as the result of rotating vector [1 0 0] by Ï† around axis Y then by Î¸ around axis Z. This is the latitude/longitude notation. The corresponding transformation matrix is:

|cosÎ¸ cosÏ†   -sinÎ¸   -cosÎ¸ sinÏ†|
|sinÎ¸ cosÏ†    cosÎ¸   -sinÎ¸ sinÏ†|
|   sinÏ†       0         cosÏ†  |
See the first column? Thatâ€™s your vec3, since itâ€™s the image of [1 0 0]. So the good thing is that we already know a lot of the matrix values.

The following code computes the remaining values:

mat3 rotation_matrix(vec3 v)
{
    // Find cosÏ† and sinÏ† 
    float c1 = sqrt(v.x * v.x + v.y * v.y);
    float s1 = v.z;
    // Find cosÎ¸ and sinÎ¸; if gimbal lock, choose (1,0) arbitrarily 
    float c2 = c1 ? v1.x / c1 : 1.0;
    float s2 = c1 ? v1.y / c1 : 0.0;

    return mat3(v.x, -s2, -s1*c2,
                v.y,  c2, -s1*s2,
                v.z,   0,     c1);
}
*/

void vec_2_rot_matrix(double matrix[3][3], double x, double y, double z){
	/* Find cos(fi) and sin(fi) */ 
	double c1 = sqrt(x * x + y * y);
	double s1 = z;
	/* Find cos(omega) and sin(omega); if gimbal lock, choose (1,0) arbitrarily */
	double c2 = c1 ? x / c1 : 1.0;
	double s2 = c1 ? y / c1 : 0.0;
	
	matrix[0][0] = x;
	matrix[1][0] = y;
	matrix[2][0] = z;
	
	matrix[0][1] = -s2;
	matrix[1][1] = c2;
	matrix[2][1] = 0;
	
	matrix[0][2] = -s1*c2;
	matrix[1][2] = -s1*s2;
	matrix[2][2] = c1;
}

void graph_matrix(graph_obj * master, double matrix[3][3]){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x0, y0, z0, x1, y1, z1;
			double sine = 0, cosine = 1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			master->flags &= ~(EXT_INI);
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				
				x0 = current->x0 * matrix[0][0] + current->y0 * matrix[0][1] +current->z0 * matrix[0][2];
				y0 = current->x0 * matrix[1][0] + current->y0 * matrix[1][1] +current->z0 * matrix[1][2];
				z0 = current->x0 * matrix[2][0] + current->y0 * matrix[2][1] +current->z0 * matrix[2][2];
				
				x1 = current->x1 * matrix[0][0] + current->y1 * matrix[0][1] +current->z1 * matrix[0][2];
				y1 = current->x1 * matrix[1][0] + current->y1 * matrix[1][1] +current->z1 * matrix[1][2];
				z1 = current->x1 * matrix[2][0] + current->y1 * matrix[2][1] +current->z1 * matrix[2][2];
				
				/* update the graph */
				current->x0 = x0;
				current->y0 = y0;
				current->z0 = z0;
				current->x1 = x1;
				current->y1 = y1;
				current->z1 = z1;
				
				/*update the extent of graph */
				/* sort the coordinates of entire line*/
				min_x = (x0 < x1) ? x0 : x1;
				min_y = (y0 < y1) ? y0 : y1;
				max_x = (x0 > x1) ? x0 : x1;
				max_y = (y0 > y1) ? y0 : y1;
				if (!(master->flags & EXT_INI)){
					master->flags |= EXT_INI;
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

int graph_list_matrix(list_node *list, double matrix[3][3]){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_matrix(curr_graph, matrix);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}
#if(0)
int graph_change_patt (graph_obj * graph, double pattern[20], int size){
	/* change the graph line pattern */
	if(graph){
		int i;
		graph->patt_size = size;
		for (i = 0; i < size; i++){
			graph->pattern[i] = pattern[i];
			
			/* TODO */
			graph->cmplx_pat[i] = NULL;
		}
		return 1;
	}
	return 0;
}

int graph_list_change_patt(list_node *list, double pattern[20], int size){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_change_patt(curr_graph, pattern, size);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}
#endif