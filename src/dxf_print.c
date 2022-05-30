#include "dxf_print.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define TOLERANCE 1e-9

void print_img_pdf(struct pdf_doc *pdf, bmp_img *img){
	/* convert and append bitmap image data in pdf document*/
	if (!pdf) return;
	if (!img) return;
	if (!img->buf) return;
	
	unsigned char *buf, *alpha;
	int w = img->width;
	int h = img->height;
	
	buf = malloc (3 * w * h);
	alpha = malloc (w * h);
	
	if(!buf) return;
	if(!alpha) return;
	
	int i, j;
	int r = img->r_i;
	int g = img->g_i;
	int b = img->b_i;
	int a = img->a_i;
	int ofs, ofs_src, ofs_dst;
	unsigned char *img_buf = img->buf;
	
	/* fill colors and alpha buffers separatedly */
	for (i = 0; i < w; i++){
		for (j = 0; j < h; j++){
			ofs = j * w + i;
			ofs_src = 4 * ofs;
			ofs_dst = 3 * ofs;
			
			buf[ofs_dst] = img_buf[ofs_src + r];
			buf[ofs_dst + 1] = img_buf[ofs_src + g];
			buf[ofs_dst + 2] = img_buf[ofs_src + b];
			
			alpha[ofs] = img_buf[ofs_src + a];
		}
	}
	
	
	/* -------------- compress the command buffer stream (deflate algorithm)*/
	int cmp_status;
	long src_len = 3 * w * h;
	long cmp_len = compressBound(src_len);
	/* Allocate buffers to hold compressed and uncompressed data. */
	mz_uint8 *pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	if (!pCmp) {
		free(buf);
		free(alpha);
		return;
	}
	
	struct pdf_object *mask = NULL;
	src_len = w * h;
	cmp_len = compressBound(src_len);
	
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)alpha, src_len);
	if (cmp_status == Z_OK){
		mask = pdf_add_smask(pdf, (uintptr_t)img, pCmp, cmp_len, w, h);
	}
	
	src_len = 3 * w * h;
	cmp_len = compressBound(src_len);
	
	/* Compress buffer string. */
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf, src_len);
	if (cmp_status == Z_OK){
		struct pdf_object *obj = pdf_add_raw_img(pdf, (uintptr_t)img, pCmp, cmp_len, w, h, mask);
	}
	
	
	/*-------------------------*/
	
	
	
	free(pCmp);
	free(buf);
	free(alpha);
}

void print_graph_pdf(graph_obj * master, struct txt_buf *buf, struct print_param param){
	/* convert a single graph object to pdf commands*/
	
	if (master == NULL) return;
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !(master->flags & FILLED) ))//!master->fill))
		&& master->color.a > 0
	){
		int x0, y0, x1, y1, i;
		line_node *current = master->list->next;
		int tick = 0, prev_x, prev_y;
		int init = 0;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = (int) ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = (int) ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){
				/* set the pattern */
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"[");
				if(master->patt_size > 1){
					int patt_el = (int) fabs(master->pattern[0] * param.scale * param.resolution);
					if (patt_el < param.resolution) patt_el = (int) param.resolution;
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d", patt_el);
					for (i = 1; i < master->patt_size; i++){
						patt_el = (int) fabs(master->pattern[i] * param.scale * param.resolution);
						if (patt_el < param.resolution) patt_el = (int) param.resolution;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							" %d", patt_el);
					}
				}
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"] 0 d ");
				
				/* set the tickness */
				//if (master->thick_const) 
				if (master->flags & THICK_CONST) 
					tick = (int) round(master->tick * param.resolution);
				else tick = (int) round(master->tick * param.scale * param.resolution);
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d w ", tick); /*line width */
				
				/* set the color */
				if (!param.mono){
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%.4g %.4g %.4g RG ",
						(float)color.r/255,
						(float)color.g/255,
						(float)color.b/255);
					//if (master->fill) /* check if object is filled */
					if (master->flags & FILLED) /* check if object is filled */
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%.4g %.4g %.4g rg ",
							(float)color.r/255,
							(float)color.g/255,
							(float)color.b/255);
				}
				
				/* move to first point */
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d m ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y)))
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d m ", x0, y0);

			buf->pos +=snprintf(buf->data + buf->pos,
				PDF_BUF_SIZE - buf->pos,
				"%d %d l ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		//if (master->fill) /* check if object is filled */
		if (master->flags & FILLED) /* check if object is filled */
			buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"f*\r\n");
		
		else buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"S\r\n");
	}
	if((master->img) /* check if has an image */
		&& (!(pos_p0 & pos_p1))) /* and in bounds of page */
	{
		int x0, y0;
		/* insertion point is first vertice */
		line_node *current = master->list->next;
		/* apply the scale and offset */
		x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
		y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
		int w = master->img->width;
		int h = master->img->height;
		
		double u[3], v[3];
		u[0] = master->u[0] * param.scale * param.resolution * w;
		u[1] = master->u[1] * param.scale * param.resolution * w;
		u[2] = master->u[2] * param.scale * param.resolution * w;
		
		v[0] = master->v[0] * param.scale * param.resolution * h;
		v[1] = master->v[1] * param.scale * param.resolution * h;
		v[2] = master->v[2] * param.scale * param.resolution * h;
		
		/* do the image */
		buf->pos +=snprintf(buf->data + buf->pos,
			PDF_BUF_SIZE - buf->pos,
			"\r\nq %.4f %.4f %.4f %.4f %d %d cm /Img%lu Do Q\r\n", u[0], u[1], v[0], v[1], x0, y0, master->img);
	}
}

int print_cmplx_graph_pdf(graph_obj * master, struct txt_buf *buf, struct print_param param){
	/* for graphs with complex line type */
	if (master == NULL) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((pos_p0 & pos_p1) || master->color.a == 0.0) return 0;
	
	double x0, y0, x1, y1;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter, prev_x, prev_y;
	
	/* verify if color is substituted  */
	bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
	
	/* set the tickness */
	buf->pos +=snprintf(buf->data + buf->pos,
		PDF_BUF_SIZE - buf->pos,
		"0 w "); /*line width = 0 */
		
	/* set the color */
	if (!param.mono){
		buf->pos +=snprintf(buf->data + buf->pos,
			PDF_BUF_SIZE - buf->pos,
			"%.4g %.4g %.4g RG ",
			(float)color.r/255,
			(float)color.g/255,
			(float)color.b/255);
	}
	
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
		
		/* move to first point */
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		buf->pos +=snprintf(buf->data + buf->pos,
			PDF_BUF_SIZE - buf->pos,
			"%d %d m ", (int)p1x, (int)p1y);
		prev_x = p1x;
		prev_y = p1y;
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
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		
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
			p2x = patt_rem * param.scale * param.resolution * cosine + p1x;
			p2y = patt_rem * param.scale * param.resolution * sine + p1y;
			
			if (patt_rem > 0) {
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p2x >= 0 && p2x < param.w * param.resolution && /* inside bound parameters */
					p2y >= 0 && p2y < param.h * param.resolution)
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
								int xd0 = p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								int yd0 = p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								int xd1 = p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								int yd1 = p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								//bmp_line(img, xd0, yd0, xd1, yd1);
								if (((xd0 != prev_x)||(yd0 != prev_y)))
									buf->pos +=snprintf(buf->data + buf->pos,
										PDF_BUF_SIZE - buf->pos,
										"%d %d m ", xd0, yd0);

								buf->pos +=snprintf(buf->data + buf->pos,
									PDF_BUF_SIZE - buf->pos,
									"%d %d l ", xd1, yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
								cplx_lin = cplx_lin->next; /* go to next */
							}
						}
						cplx = cplx->next;
					}
				}
				/*------------------------------------------------------*/
				
				patt_i++;
				if (patt_i >= master->patt_size) patt_i = 0;
				
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%d %d m ", (int)p1x, (int)p1y);

					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d l ", (int)p2x, (int)p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
			}
			
			patt_rem = patt_rem_n; /* for next segment */
			p1x = p2x;
			p1y = p2y;
			
			/* draw pattern */
			iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
			for (i = 0; i < iter; i++){					
				draw = master->pattern[patt_i] >= 0.0;
				p2x = fabs(master->pattern[patt_i]) * param.scale * param.resolution * cosine + p1x;
				p2y = fabs(master->pattern[patt_i]) * param.scale * param.resolution * sine + p1y;
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%d %d m ", (int)p1x, (int)p1y);

					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d l ", (int)p2x, (int)p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
				p1x = p2x;
				p1y = p2y;
				
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p1x >= 0 && p1x < param.w * param.resolution && /* inside bound parameters */
					p1y >= 0 && p1y < param.h * param.resolution)
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
								int xd0 = p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								int yd0 = p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								int xd1 = p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								int yd1 = p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								if (((xd0 != prev_x)||(yd0 != prev_y)))
									buf->pos +=snprintf(buf->data + buf->pos,
										PDF_BUF_SIZE - buf->pos,
										"%d %d m ", xd0, yd0);

								buf->pos +=snprintf(buf->data + buf->pos,
									PDF_BUF_SIZE - buf->pos,
									"%d %d l ", xd1, yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
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
			
			p2x = last * param.scale * param.resolution * cosine + p1x;
			p2y = last * param.scale * param.resolution * sine + p1y;
			draw = master->pattern[patt_i] >= 0.0;
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d m ", (int)p1x, (int)p1y);

				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d l ", (int)p2x, (int)p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
		}
		else{ /* current segment is in same past iteration pattern */
			p2x = modulus * param.scale * param.resolution * cosine + p1x;
			p2y = modulus * param.scale * param.resolution * sine + p1y;
			
			patt_rem -= modulus;
		
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d m ", (int)p1x, (int)p1y);

				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d l ", (int)p2x, (int)p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
			p1x = p2x;
			p1y = p2y;
		}
	
		current = current->next; /* go to next */
	}
	/* stroke the graph */
	buf->pos +=snprintf(buf->data + buf->pos,
		PDF_BUF_SIZE - buf->pos,
		"S\r\n");
}

int print_list_pdf(list_node *list, struct txt_buf *buf, struct print_param param){
	/* convert a list of graph objects to pdf commands*/
	
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			/* proccess each element */
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if (curr_graph->flags & CMPLX_PAT)
					print_cmplx_graph_pdf(curr_graph, buf, param);
				else print_graph_pdf(curr_graph, buf, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_pdf(dxf_drawing *drawing, struct txt_buf *buf , struct print_param param){
	/* convert the entities section of a drawing to pdf commands*/
	
	dxf_node *current = NULL;
	
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ /* DXF entity */
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					if (!init){ /* init pdf object */
						double res;
						if (param.resolution > 0.0) res = 1/param.resolution;
						else res = 1.0;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"0.0 0.0 0.0 RG " /*set to black */
							//"1 J " /*line cap style - round*/
							/* transformation matrix */
							"%.4g 0.0 0.0 %.4g 0.0 0.0 cm\r\n", res, res);
						init = 1;
					}
					/* proccess each element */
					print_list_pdf(current->obj.graphics, buf, param);
				}
			}
			current = current->next;
		}
	}
}

int print_pdf(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a pdf file */
	
	if (!drawing) return 0;
	
	/* buffer to hold the pdf commands from drawing convertion */
	//struct txt_buf *buf = malloc(sizeof(struct txt_buf));
	static struct txt_buf buf;
	struct Mem_buffer *mem1 = manage_buffer(PDF_BUF_SIZE + 1, BUF_GET, 2);
	if (!mem1) return 0;
	buf.data = mem1->buffer;
	//if (!buf) return 0;
	
	buf.pos = 0; /* init buffer */
	
	/* resolution -> multiplier factor over integer units in pdf */
	param.resolution = 20;
	
	/* multiplier to fit pdf parameters in final output units */
	double mul = 1.0;
	if (param.unit == PRT_MM)
		mul = 72.0 / 25.4;
	else if (param.unit == PRT_IN)
		mul = 72.0;
	else if (param.unit == PRT_PX)
		mul = 72.0/96.0;
	
	param.w = mul * param.w + 0.5;
	param.h = mul * param.h + 0.5;
	param.scale *= mul;
	
	/* fill buffer with pdf drawing commands */
	print_ents_pdf(drawing, &buf, param);
	
	/* -------------- compress the command buffer stream (deflate algorithm)*/
	int cmp_status;
	long src_len = strlen(buf.data);
	long cmp_len = compressBound(src_len);
	/* Allocate buffers to hold compressed and uncompressed data. */
	//mz_uint8 *pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	//if (!pCmp) {
	struct Mem_buffer *mem2 = manage_buffer(cmp_len, BUF_GET, 3);
	if (!mem2) {
		//free(buf);
		manage_buffer(0, BUF_FREE, 2);
		return 0;
	}
	
	mz_uint8 *pCmp = (mz_uint8 *) mem2->buffer;
	
	/* Compress buffer string. */
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf.data, src_len);
	if (cmp_status != Z_OK){
		//free(pCmp);
		manage_buffer(0, BUF_FREE, 3);
		//free(buf);
		manage_buffer(0, BUF_FREE, 2);
		return 0;
	}
	/*-------------------------*/
	
	/*--------------- assemble the pdf structure to a file */
	/* pdf info header */
	struct pdf_info info = {
		.creator = "CadZinho",
		.producer = "CadZinho",
		.title = "Print Drawing",
		.author = "Ze Cruel",
		.subject = "Print Drawing",
		.date = ""
	};
	/* pdf main struct */
	struct pdf_doc *pdf = pdf_create((int)param.w, (int)param.h, &info);
	
	
	/*---------------------*/
	/* add  the drawing images in pdf */
	if (drawing->img_list != NULL){
		
		list_node *list = drawing->img_list;
		
		struct dxf_img_def * img_def;
		bmp_img * img;
		
		list_node *current = list->next;
		while (current != NULL){ /* sweep the image list */
			if (current->data){
				img_def = (struct dxf_img_def *)current->data;
				img = img_def->img;
				if (img){
					/* convert and append image data*/
					print_img_pdf(pdf, img);
				}
			}
			current = current->next;
		}
	}
	/*---------------------*/
	
	/* add a page to pdf file */
	struct pdf_object *page = pdf_append_page(pdf);
	
	/* add the print objet to pdf page */
	//pdf_add_stream(pdf, page, buf->data); /* non compressed stream */
	pdf_add_stream_zip(pdf, page, pCmp, cmp_len); /* compressed stream */
	
	/* save pdf file */ 
	int e = pdf_save(pdf, dest);
	/*-------------------------*/
	
	/* clear and safe quit */
	pdf_destroy(pdf);
	//free(pCmp);
	manage_buffer(0, BUF_FREE, 3);
	//free(buf);
	manage_buffer(0, BUF_FREE, 2);
	if (e) return 0; /* error in file creation */
	
	return 1;
}

void svg_img_base64(void *context, void *data, int size){
	/* callback function from stbi_write
	Convert to base64 and write to file */
	
	static char base64_table[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};
	
	unsigned long triple = 0;
	static unsigned long octets[3];
	static int count = 0;
	int i, j;
	
	FILE *file = (FILE *)context; /*file to write*/
	
	if (!file) return;
	
	
	unsigned char *stream = (unsigned char *)data;
	if (stream){
		/* sweep the stream data */
		for (i = 0; i < size; i++){
			octets[count] = stream[i];
			count++;
			/* for each 3 inputs bytes, generate 4 output bytes in base64 notation*/
			if (count >= 3){
				/* combine input data in unique variable */
				triple = (octets[0] << 0x10) + (octets[1] << 0x08) + octets[2];
				for (j = 0; j < 4; j++){
					/* break the result variable  in correspondent base64 codes*/
					char c = base64_table[(triple >> (3 - j) * 6) & 0x3F];
					fputc(c, file); /* write to file */
				}
				count = 0; /* reset status */
			}
		}
	}
	else if (count > 0){ /* if has pendent bytes to write */
		/* complete remainder input bytes whith zeros */
		i = count;
		while (i < 3){
			octets[i] = 0;
			i++;
		}
		/* combine input data in unique variable */
		triple = (octets[0] << 0x10) + (octets[1] << 0x08) + octets[2];
		for (j = 0; j < 4; j++){
			char c;
			/* break the result variable  in correspondent base64 codes*/
			if (j <= count) c = base64_table[(triple >> (3 - j) * 6) & 0x3F];
			else c = '='; /* complete remainder output bytes whith '=' */
			fputc(c, file); /* write to file */
		}
		
		count = 0; /* reset status */
	}
	else count = 0; /* reset status */
}

void print_graph_svg(graph_obj * master, FILE *file, struct print_param param){
	/* convert a single graph object to svg commands*/
	
	if (master == NULL) return;
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !(master->flags & FILLED) )) //!master->fill))
		&& master->color.a > 0
	){
		double x0, y0, x1, y1;
		line_node *current = master->list->next;
		double tick = 0, prev_x, prev_y;
		int init = 0, i;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = param.h * param.resolution - ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = param.h * param.resolution - ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){ /* init the svg path */
				fprintf(file,"<path ");
				
				/* set the pattern */
				if(master->patt_size > 1){
					/* get pattern lenght */
					double patt_len = 0;
					for (i = 0; i < master->patt_size; i++){
						patt_len += fabs(master->pattern[i] * param.scale * param.resolution);
					}
					
					/* assemble pattern array */
					if ((patt_len / master->patt_size) >= 1.0){
					
						fprintf(file,"stroke-dasharray=\"");
						
						double patt_el = fabs(master->pattern[0] * param.scale * param.resolution);
						if (patt_el < 1.0) patt_el = 1.0;
						fprintf(file, "%.4g", patt_el);
						for (i = 1; i < master->patt_size; i++){
							patt_el = fabs(master->pattern[i] * param.scale * param.resolution);
							if (patt_el < 1.0) patt_el = 1.0;
							fprintf(file, ",%.4g", patt_el);
						}
						
						fprintf(file,"\" ");
					}
				}
				
				/* set the tickness */
				//if (master->thick_const) 
				if (master->flags & THICK_CONST) tick = (master->tick);// * param.resolution);
				else tick = (master->tick * param.scale);// * param.resolution);
				if (tick >= 1.0) fprintf(file, "stroke-width=\"%0.1f\" ", tick);
				
				/* set the color */
				if (!param.mono){
					
					//if (master->fill) /* check if object is filled */
					if (master->flags & FILLED) /* check if object is filled */
						fprintf(file, "fill=\"rgb(%d, %d, %d)\" stroke=\"none\" ",
							color.r, color.g, color.b);
					else
						fprintf(file, "stroke=\"rgb(%d, %d, %d)\" fill=\"none\" ",
							color.r, color.g, color.b);
				}
				else {
					
					//if (master->fill) /* check if object is filled */
					if (master->flags & FILLED) /* check if object is filled */
						fprintf(file, "fill=\"black\" stroke=\"none\" ");
					else
						fprintf(file, "stroke=\"black\" fill=\"none\" ");
				}
				
				/* move to first point */
				fprintf(file, "d=\"M%.4g %.4g ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if ((fabs(x0 - prev_x) > 1e-4) || (fabs(y0 - prev_y) > 1e-4))
				fprintf(file, "M%.4g %.4g ", x0, y0);

			fprintf(file, "L%.4g %.4g ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		fprintf(file, "\"/>\n");
	}
	if((master->img) /* check if has an image */
		&& (!(pos_p0 & pos_p1))) /* and in bounds of page */
	{
		double x0, y0;
		int w = master->img->width;
		int h = master->img->height;
		
		double u[3], v[3];
		u[0] = master->u[0] * param.scale * param.resolution;
		u[1] = master->u[1] * param.scale * param.resolution;
		u[2] = master->u[2] * param.scale * param.resolution;
		
		v[0] = master->v[0] * param.scale * param.resolution;
		v[1] = master->v[1] * param.scale * param.resolution;
		v[2] = master->v[2] * param.scale * param.resolution;
		
		/* insertion point is first vertice */
		line_node *current = master->list->next;
		
		/* apply the scale and offset */
		x0 = ((current->x0 - param.ofs_x) * param.scale * param.resolution);
		y0 = param.h * param.resolution - ((current->y0 - param.ofs_y) * param.scale * param.resolution);
		
		/* top left image correction */
		x0 += v[0] * h;
		y0 -= v[1] * h;
		
		/* do the image */
		fprintf(file,"<image height=\"%dpx\" width=\"%dpx\" "
		"transform=\"matrix(%f %f %f %f %f %f)\" "
		"xlink:href=\"data:image/png;base64,",
		h, w, u[0], -u[1], -v[0], v[1], x0, y0);
		/* convert bitmap image buffer to PNG and write to base64 enconding */
		stbi_write_png_to_func(&svg_img_base64, file, w, h, 4, master->img->buf, w * 4);
		svg_img_base64(file, NULL, 0); /* terminate base64 convetion, if has pendent bytes */
		fprintf(file, "\"/>\n");
	}
}

int print_cmplx_graph_svg (graph_obj * master, FILE *file, struct print_param param){
	/* convert a single graph object to svg commands - for graphs with complex line type */
	
	if (master == NULL) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((pos_p0 & pos_p1) || master->color.a == 0.0) return 0;
	
	double x0, y0, x1, y1, prev_x, prev_y;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter;
	
	/* verify if color is substituted  */
	bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
	
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
		
		/* init the svg path */
		fprintf(file,"<path ");
		
		/* set the color */
		if (!param.mono){
			fprintf(file, "stroke=\"rgb(%d, %d, %d)\" fill=\"none\" ",
				color.r, color.g, color.b);
		}
		else {
			fprintf(file, "stroke=\"black\" fill=\"none\" ");
		}
		
		/* move to first point */
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		
		fprintf(file, "d=\"M%.4g %.4g ", p1x, param.h * param.resolution - p1y);
		prev_x = p1x;
		prev_y = p1y;
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
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		
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
			p2x = patt_rem * param.scale * param.resolution * cosine + p1x;
			p2y = patt_rem * param.scale * param.resolution * sine + p1y;
			
			if (patt_rem > 0) {
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p2x >= 0 && p2x < param.w * param.resolution && /* inside bound parameters */
					p2y >= 0 && p2y < param.h * param.resolution)
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
								double xd0 = p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								double yd0 = p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								double xd1 = p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								double yd1 = p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								if ((fabs(xd0 - prev_x) > 1e-4) || (fabs(yd0 - prev_y) > 1e-4))
									fprintf(file, "M%.4g %.4g ", xd0, param.h * param.resolution - yd0);

								fprintf(file, "L%.4g %.4g ", xd1, param.h * param.resolution - yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
								cplx_lin = cplx_lin->next; /* go to next */
							}
						}
						cplx = cplx->next;
					}
				}
				/*------------------------------------------------------*/
				
				patt_i++;
				if (patt_i >= master->patt_size) patt_i = 0;
				
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((fabs(p1x - prev_x) > 1e-4) || (fabs(p1y - prev_y) > 1e-4))
						fprintf(file, "M%.4g %.4g ", p1x, param.h * param.resolution - p1y);

					fprintf(file, "L%.4g %.4g ", p2x, param.h * param.resolution - p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
			}
			
			patt_rem = patt_rem_n; /* for next segment */
			p1x = p2x;
			p1y = p2y;
			
			/* draw pattern */
			iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
			for (i = 0; i < iter; i++){					
				draw = master->pattern[patt_i] >= 0.0;
				p2x = fabs(master->pattern[patt_i]) * param.scale * param.resolution * cosine + p1x;
				p2y = fabs(master->pattern[patt_i]) * param.scale * param.resolution * sine + p1y;
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((fabs(p1x - prev_x) > 1e-4) || (fabs(p1y - prev_y) > 1e-4))
						fprintf(file, "M%.4g %.4g ", p1x, param.h * param.resolution - p1y);

					fprintf(file, "L%.4g %.4g ", p2x, param.h * param.resolution - p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
				p1x = p2x;
				p1y = p2y;
				
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p1x >= 0 && p1x < param.w * param.resolution && /* inside bound parameters */
					p1y >= 0 && p1y < param.h * param.resolution)
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
								double xd0 = p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								double yd0 = p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								double xd1 = p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								double yd1 = p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								if ((fabs(xd0 - prev_x) > 1e-4) || (fabs(yd0 - prev_y) > 1e-4))
									fprintf(file, "M%.4g %.4g ", xd0, param.h * param.resolution - yd0);

								fprintf(file, "L%.4g %.4g ", xd1, param.h * param.resolution - yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
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
			
			p2x = last * param.scale * param.resolution * cosine + p1x;
			p2y = last * param.scale * param.resolution * sine + p1y;
			draw = master->pattern[patt_i] >= 0.0;
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((fabs(p1x - prev_x) > 1e-4) || (fabs(p1y - prev_y) > 1e-4))
					fprintf(file, "M%.4g %.4g ", p1x, param.h * param.resolution - p1y);

				fprintf(file, "L%.4g %.4g ", p2x, param.h * param.resolution - p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
		}
		else{ /* current segment is in same past iteration pattern */
			p2x = modulus * param.scale * param.resolution * cosine + p1x;
			p2y = modulus * param.scale * param.resolution * sine + p1y;
			
			patt_rem -= modulus;
		
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((fabs(p1x - prev_x) > 1e-4) || (fabs(p1y - prev_y) > 1e-4))
					fprintf(file, "M%.4g %.4g ", p1x, param.h * param.resolution - p1y);

				fprintf(file, "L%.4g %.4g ", p2x, param.h * param.resolution - p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
			p1x = p2x;
			p1y = p2y;
		}
	
		current = current->next; /* go to next */
	}
	/* stroke the graph */
	fprintf(file, "\"/>\n");
}

int print_list_svg(list_node *list, FILE *file, struct print_param param){
	/* convert a list of graph objects to svg commands*/
	
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			/* proccess each element */
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if (curr_graph->flags & CMPLX_PAT)
					print_cmplx_graph_svg(curr_graph, file, param);
				else print_graph_svg(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_svg(dxf_drawing *drawing, FILE *file , struct print_param param){
	/* convert the entities section of a drawing to svg commands*/
	
	dxf_node *current = NULL;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
					/* init the main svg object */
					if (!init){
						/* write the svg header */
						fprintf(file, "<svg width=\"%0.4f\" height=\"%0.4f\" "
							"version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
							"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
							param.w * param.resolution,
							param.h * param.resolution);
						init = 1;
					}
					/* proccess each entity */
					print_list_svg(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the main svg object */
		if (init) fprintf(file,"</svg>");
	}
}

int print_svg(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a svg file */
	
	if (!drawing) return 0;
	
	/* open the file for writing */
	FILE *file = fopen(dest, "w");
	
	if (!file) return 0; /* error on creating/opening file */
	param.resolution = 4.0;
	
	/* fill buffer with svg drawing commands */
	print_ents_svg(drawing, file, param);
	
	fclose(file);
	
	return 1;
}

void print_graph_ps(graph_obj * master, FILE *file, struct print_param param){
	/* convert a single graph object to postscript commands*/
	
	if (master == NULL) return;
	
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !(master->flags & FILLED) ))//!master->fill))
	){
		int x0, y0, x1, y1, i;
		line_node *current = master->list->next;
		int tick = 0, prev_x, prev_y;
		int init = 0;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = (int) ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = (int) ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){
				/* set the pattern */
				fprintf(file, "n [");
				if(master->patt_size > 1){
					int patt_el = (int) fabs(master->pattern[0] * param.scale * param.resolution);
					if(patt_el < param.resolution) patt_el = param.resolution;
					fprintf(file, "%d", patt_el);
					for (i = 1; i < master->patt_size; i++){
						patt_el = (int) fabs(master->pattern[i] * param.scale * param.resolution) + 1;
						if(patt_el < param.resolution) patt_el = param.resolution;
						fprintf(file, " %d", patt_el);
					}
				}
				fprintf(file, "] 0 d ");
				
				/* set the tickness */
				//if (master->thick_const) 
				if (master->flags & THICK_CONST)
					tick = (int) round(master->tick * param.resolution);
				else tick = (int) round(master->tick * param.scale * param.resolution);
				fprintf(file, "%d lw ", tick); /*line width */
				
				/* set the color */
				if (!param.mono){
					fprintf(file, "%.4g %.4g %.4g rg ",
						(float)color.r/255,
						(float)color.g/255,
						(float)color.b/255);
				}
				
				/* move to first point */
				fprintf(file, "%d %d m ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y)))
				fprintf(file, "%d %d m ", x0, y0);

			fprintf(file, "%d %d l ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		//if (master->fill) /* check if object is filled */
		if (master->flags & FILLED) /* check if object is filled */
			fprintf(file, "f\n");
		
		else fprintf(file, "s\n");
	}
}

int print_cmplx_graph_ps(graph_obj * master, FILE *file, struct print_param param){
	/* for graphs with complex line type */
	if (master == NULL) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((pos_p0 & pos_p1) || master->color.a == 0.0) return 0;
	
	double x0, y0, x1, y1;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter, prev_x, prev_y;
	
	/* verify if color is substituted  */
	bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
	
	/* set the tickness */
	fprintf(file, "0 lw "); /*line width */
				
	/* set the color */
	if (!param.mono){
		fprintf(file, "%.4g %.4g %.4g rg ",
			(float)color.r/255,
			(float)color.g/255,
			(float)color.b/255);
	}
	
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
		
		/* move to first point */
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		fprintf(file, "%d %d m ", (int)p1x, (int)p1y);
		prev_x = p1x;
		prev_y = p1y;
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
		p1x = ((x0 - param.ofs_x) * param.scale * param.resolution);
		p1y = ((y0 - param.ofs_y) * param.scale * param.resolution);
		
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
			p2x = patt_rem * param.scale * param.resolution * cosine + p1x;
			p2y = patt_rem * param.scale * param.resolution * sine + p1y;
			
			if (patt_rem > 0) {
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p2x >= 0 && p2x < param.w * param.resolution && /* inside bound parameters */
					p2y >= 0 && p2y < param.h * param.resolution)
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
								int xd0 = p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								int yd0 = p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								int xd1 = p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								int yd1 = p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								if (((xd0 != prev_x)||(yd0 != prev_y)))
									fprintf(file, "%d %d m ", xd0, yd0);

								fprintf(file, "%d %d l ", xd1, yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
								cplx_lin = cplx_lin->next; /* go to next */
							}
						}
						cplx = cplx->next;
					}
				}
				/*------------------------------------------------------*/
				
				patt_i++;
				if (patt_i >= master->patt_size) patt_i = 0;
				
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
						fprintf(file, "%d %d m ", (int)p1x, (int)p1y);

					fprintf(file, "%d %d l ", (int)p2x, (int)p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
			}
			
			patt_rem = patt_rem_n; /* for next segment */
			p1x = p2x;
			p1y = p2y;
			
			/* draw pattern */
			iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
			for (i = 0; i < iter; i++){					
				draw = master->pattern[patt_i] >= 0.0;
				p2x = fabs(master->pattern[patt_i]) * param.scale * param.resolution * cosine + p1x;
				p2y = fabs(master->pattern[patt_i]) * param.scale * param.resolution * sine + p1y;
				if (draw && ( /* if draw and inside bound parameters */
					(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
					(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
				) ){
					if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
						fprintf(file, "%d %d m ", (int)p1x, (int)p1y);

					fprintf(file, "%d %d l ", (int)p2x, (int)p2y);
				
					prev_x = p2x;
					prev_y = p2y;
				}
				p1x = p2x;
				p1y = p2y;
				
				/*------------- complex line type ----------------*/
				if (master->cmplx_pat[patt_i] != NULL && /* complex element */
					p1x >= 0 && p1x < param.w * param.resolution && /* inside bound parameters */
					p1y >= 0 && p1y < param.h * param.resolution)
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
								int xd0 = p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale * param.resolution);
								int yd0 = p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale* param.resolution);
								int xd1 = p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale * param.resolution);
								int yd1 = p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale * param.resolution);
								
								if (((xd0 != prev_x)||(yd0 != prev_y)))
									fprintf(file, "%d %d m ", xd0, yd0);

								fprintf(file, "%d %d l ", xd1, yd1);
							
								prev_x = xd1;
								prev_y = yd1;
								
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
			
			p2x = last * param.scale * param.resolution * cosine + p1x;
			p2y = last * param.scale * param.resolution * sine + p1y;
			draw = master->pattern[patt_i] >= 0.0;
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
					fprintf(file, "%d %d m ", (int)p1x, (int)p1y);

				fprintf(file, "%d %d l ", (int)p2x, (int)p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
		}
		else{ /* current segment is in same past iteration pattern */
			p2x = modulus * param.scale * param.resolution * cosine + p1x;
			p2y = modulus * param.scale * param.resolution * sine + p1y;
			
			patt_rem -= modulus;
		
			if (draw && ( /* if draw and inside bound parameters */
				(p1x >= 0.0 && p1x < param.w * param.resolution && p1y >= 0.0 && p1y < param.h * param.resolution) ||
				(p2x >= 0.0 && p2x < param.w * param.resolution && p2y >= 0.0 && p2y < param.h * param.resolution)
			) ){
				if ((((int)p1x != prev_x)||((int)p1y != prev_y)))
					fprintf(file, "%d %d m ", (int)p1x, (int)p1y);

				fprintf(file, "%d %d l ", (int)p2x, (int)p2y);
			
				prev_x = p2x;
				prev_y = p2y;
			}
			p1x = p2x;
			p1y = p2y;
		}
	
		current = current->next; /* go to next */
	}
	/* stroke the graph */
	fprintf(file, "s\n");
}

int print_list_ps(list_node *list, FILE *file, struct print_param param){
	/* convert a list of graph objects to postscript commands */
	
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			/* proccess each element */
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if (curr_graph->flags & CMPLX_PAT)
					print_cmplx_graph_ps(curr_graph, file, param);
				else print_graph_ps(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_ps(dxf_drawing *drawing, FILE *file , struct print_param param){
	/* convert the entities section of a drawing to postscript commands*/
	
	dxf_node *current = NULL;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					/* init the postscript file */
					if (!init){
						/* postscript header */
						fprintf(file, "%%!PS\n\n"
							"/W %.9g def\n"
							"/H %.9g def\n"
							/* set some alias to minimize text size */
							"/n {newpath} bind def\n"
							"/m {moveto} bind def\n"
							"/l {lineto} bind def\n"
							"/cp {closepath} bind def\n"
							"/s {stroke} bind def\n"
							"/f {fill} bind def\n"
							"/d {setdash} bind def\n"
							"/lw {setlinewidth} bind def\n"
							"/rg {setrgbcolor} bind def\n"
							/* set page size (for convertion programs) */
							"<< /PageSize [W H] >> setpagedevice\n"
							/* set clip area */
							"n 0 0 m W 0 l W H l 0 H l cp clip\n\n"
							/* scale to improve resolution on integer units */
							"%.9g dup scale\n\n",
							param.w , param.h,
							1/param.resolution
						);
						init = 1;
					}
					/* fill file with postscript drawing commands */
					print_list_ps(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the postscript file */
		if (init) fprintf(file,"\nshowpage\n");
	}
}

int print_ps(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a postscript file */
	
	if (!drawing) return 0;
	
	FILE *file = fopen(dest, "w"); /* open the file */
	
	if (!file) return 0; /* error on creating/opening file */
	
	/* scale to improve resolution on integer units */
	param.resolution = 20;
	
	/* multiplier to fit postscript parameters in final output units */
	double mul = 1.0;
	if (param.unit == PRT_MM)
		mul = 72.0 / 25.4;
	else if (param.unit == PRT_IN)
		mul = 72.0;
	else if (param.unit == PRT_PX)
		mul = 72.0/96.0;
	
	param.w = (int) (mul * param.w + 0.5);
	param.h = (int) (mul * param.h + 0.5);
	param.scale *= mul;
	
	/* write drawing postscript commands to file */
	print_ents_ps(drawing, file, param);
	
	/* end */
	fclose(file);
	
	return 1;
}

int print_img(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a image file */
	/* use the same main engine to draw on the screen */
	/* write to especific data format is provided by stb_image_write library */
	
	if (!drawing) return 0;
	
	/* multiplier to fit image parameters in final output units */
	param.resolution = 1.0;
	if (param.unit == PRT_MM){
		param.resolution = 96.0 / 25.4;
	}
	else if (param.unit == PRT_IN){
		param.resolution = 96.0;
	}
	
	int w = (int) param.w * param.resolution;
	int h = (int) param.h * param.resolution;
	int ret = 0;
	
	/* main colors */
	bmp_color transp = { .r = 255, .g = 255, .b = 255, .a = 0 };
	bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
	bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
	
	/* buffer to write the raw image */
	bmp_img * img = bmp_new(w, h, transp, black);
	
	if (img == NULL) return 0;
	
	/*order of color components in buffer.*/
	img->r_i = 0;
	img->g_i = 1;
	img->b_i = 2;
	img->a_i = 3;
	
	/* init draw parameters */
	struct draw_param d_param;
	d_param.ofs_x = param.ofs_x;
	d_param.ofs_y = param.ofs_y;
	d_param.ofs_z = 0;
	d_param.scale = param.scale * param.resolution;
	d_param.list = param.list;
	d_param.subst = param.subst;
	d_param.len_subst = param.len;
	d_param.inc_thick = 0;
	
	/* clear image with background color */
	if (param.out_fmt == PRT_PNG)
		bmp_fill(img, transp); /* in PNG format, background is transparent */
	else
		bmp_fill(img, white); /* white background */
	
	/* draw image */
	dxf_ents_draw(drawing, img, d_param);
	
	/* save file in specified format */
	if (param.out_fmt == PRT_PNG)
		ret = stbi_write_png((char const *)dest, w, h, 4, img->buf, w * 4);
	else if (param.out_fmt == PRT_BMP)
		ret = stbi_write_bmp((char const *)dest, w, h, 4, img->buf);
	else if (param.out_fmt == PRT_JPG)
		ret = stbi_write_jpg((char const *)dest, w, h, 4, img->buf, 80);
	
	/* end */
	bmp_free(img);
	
	return ret;
}