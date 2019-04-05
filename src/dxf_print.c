#include "dxf_print.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void print_graph_pdf(graph_obj * master, struct txt_buf *buf, struct print_param param){
	if (master != NULL){
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
			&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !master->fill))
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
						int patt_el = (int) fabs(master->pattern[0] * param.scale * param.resolution) + 1;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%d", patt_el);
						for (i = 1; i < master->patt_size; i++){
							patt_el = (int) fabs(master->pattern[i] * param.scale * param.resolution) + 1;
							buf->pos +=snprintf(buf->data + buf->pos,
								PDF_BUF_SIZE - buf->pos,
								" %d", patt_el);
						}
					}
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"] 0 d ");
					
					/* set the tickness */
					if (master->thick_const) tick = (int) round(master->tick * 10 *!(param.inch)/25.4 * param.resolution);
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
						if (master->fill) /* check if object is filled */
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
			if (master->fill) /* check if object is filled */
				buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"f*\r\n");
			
			else buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"S\r\n");
		}
	}
}

int print_list_pdf(list_node *list, struct txt_buf *buf, struct print_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				print_graph_pdf(curr_graph, buf, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_pdf(dxf_drawing *drawing, struct txt_buf *buf , struct print_param param){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					// -------------------------------------------
					if (!init){
						double res;
						if (param.resolution > 0.0) res = 1/param.resolution;
						else res = 1.0;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"0.0 0.0 0.0 RG " /*set to black */
							//"1 J " /*line cap style - round*/
							"%.4g 0.0 0.0 %.4g 0.0 0.0 cm\r\n", res, res);
						init = 1;
					}
	
					print_list_pdf(current->obj.graphics, buf, param);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

/**
 * Convert a value in inches into a number of points.
 * Always returns an integer value
 */
int inch2pt (double inch){
	return (int)((inch)*72 + 0.5);
}

/**
 * Convert a value in milli-meters into a number of points.
 * Always returns an integer value
 */
int mm2pt (double mm){
	return (int)((mm)*72 / 25.4 + 0.5);
}

int print_pdf(dxf_drawing *drawing, struct print_param param, char *dest){
	
	if (!drawing) return 0;
	
	struct txt_buf *buf = malloc(sizeof(struct txt_buf));
	
	if (!buf) return 0;
	param.resolution = 10;
	
	buf->pos = 0;
	
	double mul = 72.0 / 25.4;
	int (*conv_fnc)(double) = mm2pt;
	if (param.inch) {
		conv_fnc = inch2pt;
		mul = 72.0;
	}
	
	param.scale *= mul;
	param.w = conv_fnc(param.w);
	param.h = conv_fnc(param.h);
	
	print_ents_pdf(drawing, buf, param);
	
	int cmp_status;
	long src_len = strlen(buf->data);
	long cmp_len = compressBound(src_len);
	// Allocate buffers to hold compressed and uncompressed data.
	mz_uint8 *pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	// Compress the string.
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf->data, src_len);
	if (cmp_status == Z_OK){
		//printf("Compressed from %u to %u bytes\n", (mz_uint32)src_len, (mz_uint32)cmp_len);
	}
	
	struct pdf_info info = {
		.creator = "CadZinho",
		.producer = "CadZinho",
		.title = "Print Drawing",
		.author = "Ze Cruel",
		.subject = "Print Drawing",
		.date = ""
	};
	struct pdf_doc *pdf = pdf_create((int)param.w, (int)param.h, &info);
	
	struct pdf_object *page = pdf_append_page(pdf);
	
	//pdf_add_stream(pdf, page, buf->data);
	pdf_add_stream_zip(pdf, page, pCmp, cmp_len);
	pdf_save(pdf, dest);
	pdf_destroy(pdf);
	
	free(pCmp);
	free(buf);
	#if(0)
	#endif
	return 1;
}


void print_graph_svg(graph_obj * master, FILE *file, struct print_param param){
	if (master != NULL){
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
			&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !master->fill))
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
				
				if (init == 0){
					fprintf(file,"<path ");
					
					/* set the pattern */
					if(master->patt_size > 1){
						/* get pattern lenght */
						double patt_len = 0;
						for (i = 0; i < master->patt_size; i++){
							patt_len += fabs(master->pattern[i] * param.scale * param.resolution);
						}
						
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
					if (master->thick_const) tick = (master->tick);// * param.resolution);
					else tick = (master->tick * param.scale);// * param.resolution);
					if (tick >= 1.0) fprintf(file, "stroke-width=\"%0.1f\" ", tick);
					
					/* set the color */
					if (!param.mono){
						
						if (master->fill) /* check if object is filled */
							fprintf(file, "fill=\"rgb(%d, %d, %d)\" stroke=\"none\" ",
								color.r, color.g, color.b);
						else
							fprintf(file, "stroke=\"rgb(%d, %d, %d)\" fill=\"none\" ",
								color.r, color.g, color.b);
					}
					else {
						
						if (master->fill) /* check if object is filled */
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
				else if (((x0 != prev_x)||(y0 != prev_y)))
					fprintf(file, "M%.4g %.4g ", x0, y0);

				fprintf(file, "L%.4g %.4g ", x1, y1);
			
				prev_x = x1;
				prev_y = y1;
				
				current = current->next; /* go to next line */
			}
			/* stroke the graph */
			fprintf(file, "\"/>\n");
		}
	}
}

int print_list_svg(list_node *list, FILE *file, struct print_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				print_graph_svg(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_svg(dxf_drawing *drawing, FILE *file , struct print_param param){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					// -------------------------------------------
					if (!init){
						fprintf(file, "<svg width=\"%0.4f\" height=\"%0.4f\" "
							"version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
							"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
							param.w * param.resolution,
							param.h * param.resolution);
						init = 1;
					}
	
					print_list_svg(current->obj.graphics, file, param);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
		if (init) fprintf(file,"</svg>");
	}
}

int print_svg(dxf_drawing *drawing, struct print_param param, char *dest){
	
	if (!drawing) return 0;
	
	FILE *file = fopen(dest, "w"); /* open the file */
	
	if (!file) return 0;
	param.resolution = 4.0;
	
	
	
	
	
	print_ents_svg(drawing, file, param);
	
	
	
	fclose(file);
	
	return 1;
}

int print_list_png(list_node *list, bmp_img * img, struct draw_param param){
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

int print_ents_png(dxf_drawing *drawing, bmp_img * img, struct draw_param param){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					// -------------------------------------------
					if (!init){
						
						init = 1;
					}
	
					print_list_png(current->obj.graphics, img, param);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

int print_png(dxf_drawing *drawing, struct print_param param, char *dest){
	
	if (!drawing) return 0;
	
	
	param.resolution = 4.0;
	
	
	bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 0 };
	bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
	
	int w = (int) param.w * param.resolution;
	int h = (int) param.h * param.resolution;
	
	bmp_img * img = bmp_new(w, h, white, black);
	
	if (img == NULL) return 0;
	
	/*order of color components in buffer.*/
	img->r_i = 0;
	img->g_i = 1;
	img->b_i = 2;
	img->a_i = 3;
	
	struct draw_param d_param;
	
	d_param.ofs_x = param.ofs_x;
	d_param.ofs_y = param.ofs_y;
	d_param.scale = param.scale * param.resolution;
	d_param.list = param.list;
	d_param.subst = param.subst;
	d_param.len_subst = param.len;
	
	bmp_fill_clip(img, img->bkg); /* clear bitmap */
	
	print_ents_png(drawing, img, d_param);
	
	stbi_write_png((char const *)dest, w, h, 4, img->buf, w * 4);
	
	
	bmp_free(img);
	
	return 1;
}